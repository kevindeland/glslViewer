#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.14159265359

uniform vec2 u_mouse;
uniform vec2 u_resolution;
uniform float u_time;

uniform sampler2D u_tex0;
uniform vec2 u_tex0Resolution;

varying vec4 v_position;
varying vec4 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;

// my first function
vec4 red(){
  return vec4(1.0,0.0,0.0,1.0);
}

vec3 greenVec3 = vec3(0.0, 1.0, 0.0);
vec3 blueVec3 = vec3(0.0, 1.0, 1.0);

vec4 noFilter = vec4(1.0, 1.0, 1.0, 1.0);
vec4 redFilter = vec4(1.0, 0.0, 0.0, 1.0);
vec4 greenFilter = vec4(0.0, 1.0, 0.0, 1.0);
vec4 blueFilter = vec4(0.0, 0.0, 1.0, 1.0);
vec4 magentaFilter = vec4(1.0, 0.0, 1.0, 1.0);
vec4 yellowFilter = vec4(1.0, 1.0, 0.0, 1.0);
vec4 cyanFilter = vec4(0.0, 1.0, 1.0, 1.0);

float myLength(in vec2 xy0, in vec2 xy1) {
  return sqrt((xy0.x - xy1.x)*(xy0.x - xy1.x) + (xy0.y - xy1.y)*(xy0.y-xy1.y));
}


// plot a line on Y using a value between 0.0-1.0
float plot(vec2 st, float pct) {
  float thick = 0.01 * sin(10.0 * u_time) + 0.1;
  return smoothstep(pct - thick, pct, st.y) - smoothstep(pct, pct + thick, st.y);
}

void main(void) {
  vec2 st = gl_FragCoord.xy / u_resolution;
  vec2 mouseRelative = u_mouse.xy / u_resolution;

  // mapping function???
  float x, y;
  y = log(4.0 * st.x);//, 2.0); // y = x
  y = (st.x - st.y);
   y = (st.x / 2.0);

   float xxx = 1.0 * PI;
   //    y =( 0.5 * abs(cos(xxx * u_time)) * sin(4.0 * PI * st.x +  xxx * u_time)  + 0.5 ) ;
  // y = step(0.5 * sin(3.0 * u_time) + 0.5, st.x);
  //   y = smoothstep(0.1,0.9,st.x);
   
   x = st.x;
   y = mod(x,0.3333); // return x modulo of 0.5
    y = fract(x * 3.0); // return only the fraction part of a number
    y = ceil(x * 4.0) / 4.0;  // nearest integer that is greater than or equal to x
    y = floor(x * 4.0) / 4.0; // nearest integer less than or equal to x

    // beat builder
    y = sign(sin(4.0 * PI * x +  PI * ceil(ceil(pow(2.0, mouseRelative.x * 4.0)) * u_time))) * 0.5 + 0.5;  // extract the sign of x
    //y = abs(x);   // return the absolute value of x
    //y = clamp(x,0.0,1.0); // constrain x to lie between 0.0 and 1.0
    //y = min(0.0,x);   // return the lesser of x and 0.0
    //y = max(0.0,x);   // return the greater of x and 0.0
    y = abs(sin(1.0 * PI * u_time)) * (smoothstep(0.0, 0.3, st.x)- smoothstep(0.5, 0.8, st.x));
   vec3 colorM = vec3(0.0, 1.0, 1.0);
   vec3 color = colorM * vec3(y);
   vec3 colorMinv = vec3(1.0 - colorM[0], 1.0 - colorM[1], 1.0 - colorM[2]);
   //   vec3 color = vec3(y, y, 0.0); // (color multiplier)
  
  // Plot a line
  float pct = plot(st, y); // plot ((x, y), x)
  color = (1.0 - pct) * color + pct * colorMinv;
  gl_FragColor = vec4(color, 1.0);

}

void oldmain(void)
{

  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
  // dynamic shape size
  float mouse_dims = 20.0 * abs(sin(3.0 * u_time));
  float mouse_radius = 600.0 * abs(sin(3.0 * u_time));

  gl_FragColor = red();
  vec2 uv = gl_FragCoord.xy / u_resolution.xy;
  gl_FragColor = vec4(uv,0.5+0.5*sin(u_time),1.0);

  // find middle line
  float midX = u_resolution.x / 2.0;
  float midY = u_resolution.y / 2.0;

  gl_FragColor = vec4(1.0 - abs(sin(1.0 * u_time)), abs(cos(3.0 * u_time)), 1.0, 1.0);

  float b = 0.0;
  float r = gl_FragCoord.x > midX ? 1.0 : 0.0;
  float g = gl_FragCoord.y > midY ? 1.0 : 0.0;

  if( myLength(vec2(gl_FragCoord.x, gl_FragCoord.y), vec2(u_mouse.x, u_mouse.y))  < mouse_radius) {
    //if (abs(u_mouse.x - gl_FragCoord.x) < mouse_dims && abs(u_mouse.y - gl_FragCoord.y) < mouse_dims) {
    b = 1.0;
  }

  vec4 myv =  vec4(r, g, b, 1.0);
  gl_FragColor = noFilter * myv;
  //fragCol
  return;
  return;
  if(gl_FragCoord.x > midX) {
    r = 0.0;
  }
  else {
    r = 1.0;
  }
}

