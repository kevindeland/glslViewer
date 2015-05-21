#include "app.h"

#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <fcntl.h>
#include <iostream>
#include <termios.h>

#include "glm/gtc/matrix_transform.hpp"

#include "utils.h"

#ifdef PLATFORM_RPI
#define MOUSEFILE "/dev/input/event0"
#define check() assert(glGetError() == 0)
EGLDisplay display;
EGLSurface surface;
EGLContext context;

#include <linux/input.h>
static int mouse_fd = -1;

#else

static GLFWwindow* window;
static float dpiScale = 1.0;

#endif

static glm::ivec4 viewport;
static glm::mat4 orthoMatrix;

typedef struct {
    float   x,y;
    float   velX,velY;
    int     button;
} Mouse;
static Mouse mouse;
static unsigned char keyPressed;

// TIME
//----------------------------------------------------
struct timeval tv;
unsigned long long timeStart;
float timeSec = 0.0f;

void initTime() {
    gettimeofday(&tv, NULL);
    timeStart = (unsigned long long)(tv.tv_sec) * 1000 +
                (unsigned long long)(tv.tv_usec) / 1000; 
}

void updateTime() {
    gettimeofday(&tv, NULL);
    unsigned long long timeNow =    (unsigned long long)(tv.tv_sec) * 1000 +
                                    (unsigned long long)(tv.tv_usec) / 1000;

    timeSec = (timeNow - timeStart)*0.001;
}

float getTime() {
    return timeSec;
}

// OPENGL through BREADCOM GPU on RASPBERRYPI
//----------------------------------------------------
#ifdef PLATFORM_RPI

void closeMouse(){
    if (mouse_fd > 0)
        close(mouse_fd);
    mouse_fd = -1;
}

int initMouse(){
    closeMouse();

    mouse.x = viewport.z*0.5;
    mouse.y = viewport.w*0.5;
    mouse_fd = open(MOUSEFILE, O_RDONLY | O_NONBLOCK);

    return mouse_fd;
}

void initGL(int argc, char **argv){

    // Start OpenGL ES
    bcm_host_init();

    // Clear application state
    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativeviewport;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    uint32_t screen_width;
    uint32_t screen_height;

    static const EGLint attribute_list[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_DEPTH_SIZE, 16,
        EGL_NONE
    };

    static const EGLint context_attributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLConfig config;

    // get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(display!=EGL_NO_DISPLAY);
    check();

    // initialize the EGL display connection
    result = eglInitialize(display, NULL, NULL);
    assert(EGL_FALSE != result);
    check();

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);
    check();

    // get an appropriate EGL frame buffer configuration
    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);
    check();

    // create an EGL rendering context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attributes);
    assert(context!=EGL_NO_CONTEXT);
    check();

    // create an EGL viewport surface
    success = graphics_get_display_size(0 /* LCD */, &screen_width, &screen_height);
    assert( success >= 0 );

    //  Initially the viewport is for all the screen
    viewport.x = 0;
    viewport.y = 0;
    viewport.z = screen_width;
    viewport.w = screen_height;

    //  Adjust the viewport acording to the passed argument
    for (int i = 1; i < argc ; i++){
        if ( std::string(argv[i]) == "-x" ) {
            i++;
            viewport.x = getInt(std::string(argv[i]));
        } else if ( std::string(argv[i]) == "-y" ) {
            i++;
            viewport.y = getInt(std::string(argv[i]));
        } else if ( std::string(argv[i]) == "-w" || 
                    std::string(argv[i]) == "--width" ) {
            i++;
            viewport.z = getInt(std::string(argv[i]));
        } else if ( std::string(argv[i]) == "-h" || 
                    std::string(argv[i]) == "--height") {
            i++;
            viewport.w = getInt(std::string(argv[i]));
        } else if ( std::string(argv[i]) == "--square") {
            if (screen_width > screen_height) {
                viewport.x = screen_width/2-screen_height/2;
            } else {
                viewport.y = screen_height/2-screen_width/2;
            }
            viewport.z = viewport.w = MIN(screen_width,screen_height);
        } else if ( std::string(argv[i]) == "-l" || 
                    std::string(argv[i]) == "--life-coding" ){
            viewport.x = viewport.z-500;
            viewport.z = viewport.w = 500;
        }
    }

    dst_rect.x = viewport.x;
    dst_rect.y = viewport.y;
    dst_rect.width = viewport.z;
    dst_rect.height = viewport.w;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = viewport.z << 16;
    src_rect.height = viewport.w << 16;

    dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
    dispman_update = vc_dispmanx_update_start( 0 );

    dispman_element = vc_dispmanx_element_add( dispman_update, dispman_display,
                                       0/*layer*/, &dst_rect, 0/*src*/,
                                       &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, (DISPMANX_TRANSFORM_T)0/*transform*/);

    nativeviewport.element = dispman_element;
    nativeviewport.width = viewport.z;
    nativeviewport.height = viewport.w;
    vc_dispmanx_update_submit_sync( dispman_update );

    check();

    surface = eglCreateWindowSurface( display, config, &nativeviewport, NULL );
    assert(surface != EGL_NO_SURFACE);
    check();

    // connect the context to the surface
    result = eglMakeCurrent(display, surface, surface, context);
    assert(EGL_FALSE != result);
    check();

    // Set background color and clear buffers
    // glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
    // glClear( GL_COLOR_BUFFER_BIT );

    setWindowSize(viewport.z,viewport.w);
    check();

    ///printf("OpenGL Initialize at %i,%i,%i,%i\n",viewport.x,viewport.y,viewport.z,viewport.w);
    initTime();
    initMouse();
}

bool isGL(){
    return true;
}

bool readMouseEvent(struct input_event *mousee){
    int bytes;
    if (mouse_fd > 0) {
        bytes = read(mouse_fd, mousee, sizeof(struct input_event));
        if (bytes == -1)
            return false;
        else 
            return true;
    }
    return false;
}

bool updateMouse() {
    if (mouse_fd < 0) {
        return false;
    }

    struct input_event mousee;
    while ( readMouseEvent(&mousee) ) {
        
        mouse.velX=0;
        mouse.velY=0;

        // Temporal holders
        int dx = 0;
        int dy = 0;
        int button = 0;
        switch(mousee.type) {
            // Update Mouse Event
            case EV_KEY:
                switch (mousee.code) {
                    case BTN_LEFT:
                        if (mousee.value == 1){
                            button = 1;
                        }
                        break;
                    case BTN_RIGHT:
                        if(mousee.value == 1){
                            button = 2;
                        }
                        break;
                    case BTN_MIDDLE:
                        if(mousee.value == 1){
                            button = 3;
                        }
                        break;
                    default:
                        button = 0;
                        break;
                }
                break;
            case EV_REL:
                switch (mousee.code) {
                    case REL_X:
                        dx = mousee.value;
                        break;
                    case REL_Y:
                        mousee.value = mousee.value * -1;
                        dy = mousee.value;
                        break;
                    // case REL_WHEEL:
                    //     if (mousee.value > 0)
                    //         std::cout << "Mouse wheel Forward" << std::endl;
                    //     else if(mousee.value < 0)
                    //         std::cout << "Mouse wheel Backward" << std::endl;
                    //     break;
                    default:
                        break;
                }
                break;
            case EV_ABS:
                switch (mousee.code) {
                    case ABS_X:
                        mouse.x = ((float)mousee.value/4095.0f)*viewport.z;
                        std::cout << "Mouse ABS_X " << mousee.value << std::endl;
                        break;
                    case ABS_Y:
                        mouse.y = (1.0-((float)mousee.value/4095.0f))*viewport.w;
                        std::cout << "Mouse ABS_Y " << mousee.value << std::endl;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }

        // Update position
        mouse.velX=dx;
        mouse.velY=dy;
        mouse.x+=mouse.velX;
        mouse.y+=mouse.velY;

        // Clamp values
        if (mouse.x < 0) mouse.x=0;
        if (mouse.y < 0) mouse.y=0;
        if (mouse.x > viewport.z) mouse.x = viewport.z;
        if (mouse.y > viewport.w) mouse.y = viewport.w;
        
        // Lunch events
        if(button != mouse.button){
            mouse.button = button;
            if(mouse.button == 0){
                onMouseRelease(mouse.x,mouse.y);
            } else {
                onMouseClick(mouse.x,mouse.y,mouse.button);
            }
        }

        if(mouse.velX != 0.0 || mouse.velY != 0.0 ){
            if (button != 0) {
                onMouseDrag(mouse.x,mouse.y,mouse.button);
            } else {
                onMouseMove(mouse.x,mouse.y);
            }
        }
    }
}

int getKey() {
    int character;
    struct termios orig_term_attr;
    struct termios new_term_attr;

    /* set the terminal to raw mode */
    tcgetattr(fileno(stdin), &orig_term_attr);
    memcpy(&new_term_attr, &orig_term_attr, sizeof(struct termios));
    new_term_attr.c_lflag &= ~(ECHO|ICANON);
    new_term_attr.c_cc[VTIME] = 0;
    new_term_attr.c_cc[VMIN] = 0;
    tcsetattr(fileno(stdin), TCSANOW, &new_term_attr);
    
    /* read a character from the stdin stream without blocking */
    /*   returns EOF (-1) if no character is available */
    character = fgetc(stdin);

    /* restore the original terminal attributes */
    tcsetattr(fileno(stdin), TCSANOW, &orig_term_attr);
    
    return character;
}

void updateGL(){
    updateTime();
    updateMouse();

    int key = getKey();
    if ( key != 0 && key != keyPressed ){
        keyPressed = key;
        onKeyPress(key);
    }  
}

void renderGL(){
    eglSwapBuffers(display, surface);
}

void closeGL(){
    closeMouse();
    eglSwapBuffers(display, surface);

    // Release OpenGL resources
    eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
    eglDestroySurface( display, surface );
    eglDestroyContext( display, context );
    eglTerminate( display );

    // printf("\nOpenGL Closed\n");
}

#else  // PLATFORM_LINUX || PLATFORM_OSX

// OPENGL through GLFW on LINUX and OSX
//----------------------------------------------------
void handleError(const std::string& _message, int _exitStatus) {
    std::cerr << "ABORT: "<< _message << std::endl;
    exit(_exitStatus);
}

void handleKeypress(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods) {
    onKeyPress(_key);
}

void fixDpiScale(){
    int window_width, window_height, framebuffer_width, framebuffer_height;
    glfwGetWindowSize(window, &window_width, &window_height);
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    dpiScale = framebuffer_width/window_width;
}

void handleResize(GLFWwindow* _window, int _w, int _h) {
    fixDpiScale();
    setWindowSize(_w*dpiScale,_h*dpiScale);
}

void handleCursor(GLFWwindow* _window, double x, double y) {

    // Update stuff
    x *= dpiScale;
    y *= dpiScale;

    mouse.velX = x - mouse.x;
    mouse.velY = (viewport.w - y) - mouse.y;
    mouse.x = x;
    mouse.y = viewport.w - y;

    if (mouse.x < 0) mouse.x=0;
    if (mouse.y < 0) mouse.y=0;
    if (mouse.x > viewport.z) mouse.x = viewport.z;
    if (mouse.y > viewport.w) mouse.y = viewport.w;

    int action1 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1);
    int action2 = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2);
    int button = 0;

    if (action1 == GLFW_PRESS) {
        button = 1;
    } else if (action2 == GLFW_PRESS) {
        button = 2;
    }

    // Lunch events
    if(mouse.button == 0 && button != mouse.button){
        mouse.button = button;
        onMouseClick(mouse.x,mouse.y,mouse.button);
    } else {
        mouse.button = button;
    }

    if(mouse.velX != 0.0 || mouse.velY != 0.0){
        if (button != 0) {
            onMouseDrag(mouse.x,mouse.y,mouse.button);
        } else {
            onMouseMove(mouse.x,mouse.y);
        }
    }    
}

void initGL(int argc, char **argv){
    viewport.x = 0;
    viewport.y = 0;
    viewport.z = 500;
    viewport.w = 500;

    for (int i = 1; i < argc ; i++){
        if ( std::string(argv[i]) == "-w" || 
             std::string(argv[i]) == "--width" ) {
            i++;
            viewport.z = getInt(std::string(argv[i]));
        } else if ( std::string(argv[i]) == "-h" || 
                    std::string(argv[i]) == "--height") {
            i++;
            viewport.w = getInt(std::string(argv[i]));
        }
    }

    if(!glfwInit()) {
        handleError("GLFW init failed", -1);
    }

    window = glfwCreateWindow(viewport.z, viewport.w, "glslViewer", NULL, NULL);

    if(!window) {
        glfwTerminate();
        handleError("GLFW create window failed", -1);
    }

    fixDpiScale();
    viewport.z *= dpiScale;
    viewport.w *= dpiScale;
    setWindowSize(viewport.z,viewport.w);

    glfwMakeContextCurrent(window);

    glfwSetWindowSizeCallback(window, handleResize);
    glfwSetKeyCallback(window, handleKeypress);
    glfwSetCursorPosCallback(window, handleCursor);

    initTime();
}

bool isGL(){
    return !glfwWindowShouldClose(window);
}

void updateGL(){
    updateTime();
    keyPressed = -1;
    glfwPollEvents();
}

void renderGL(){
    glfwSwapBuffers(window);
}

void closeGL(){
    glfwSetWindowShouldClose(window, GL_TRUE);
    glfwTerminate();
}
#endif

void setWindowSize(int _width, int _height) {
    viewport.z = _width;
    viewport.w = _height;
    glViewport((float)viewport.x, (float)viewport.y, (float)viewport.z, (float)viewport.w);
    orthoMatrix = glm::ortho((float)viewport.x, (float)viewport.z, (float)viewport.y, (float)viewport.w);

    onViewportResize(viewport.z, viewport.w);
}

int getWindowWidth(){
    return viewport.z;
}

int getWindowHeight(){
    return viewport.w;
}

glm::mat4 getOrthoMatrix(){
    return orthoMatrix;
}

float getMouseX(){
    return mouse.x;
}

float getMouseY(){
    return mouse.y;
}

glm::vec2 getMousePosition() {
    return glm::vec2(mouse.x,mouse.y);
}

float getMouseVelX(){
    return mouse.velX;
}

float getMouseVelY(){
    return mouse.velY;
}

glm::vec2 getMouseVelocity() {
    return glm::vec2(mouse.velX,mouse.velY);
}

int getMouseButton(){
    return mouse.button;
}

unsigned char getKeyPressed(){
    return keyPressed;
}