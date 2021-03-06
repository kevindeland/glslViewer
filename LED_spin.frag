#ifdef GL_ES
precision mediump float;
#endif

#define PI 3.14159265359

uniform vec2 u_mouse;
uniform vec2 u_resolution;
uniform float u_time;

float myDist (float x0, float y0, float x1, float y1) {
  return sqrt((x0 - x1) * (x0 - x1) + (y0 - y1) * (y0 - y1));
}

vec3 fadeLED (float distFromCenter , float edge) {
  float color = distFromCenter < edge / 2.0 ? 1.0 : 1.5 - distFromCenter / edge;
  return vec3(color, color, color);
}

void main(void) {
  vec4 BLK_4 = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 GRN_4 = vec4(0.0, 1.0, 0.0, 1.0);
  vec4 BLU_4 = vec4(0.0, 0.0, 1.0, 1.0);
  vec4 RED_4 = vec4(1.0, 0.0, 0.0, 1.0);
  vec4 YLW_4 = vec4(1.0, 1.0, 0.0, 1.0);
  vec4 MAG_4 = vec4(1.0, 0.0, 1.0, 1.0);
  vec4 CYN_4 = vec4(0.0, 1.0, 1.0, 1.0);
  
  vec2 thisDotsXY  = gl_FragCoord.xy / u_resolution; // st is a scaled xy

  // about the canvas
  float midX = u_resolution.x / 2.0;
  float midY = u_resolution.y / 2.0;
  // background color
  gl_FragColor = BLK_4;

  // about the circle
  // big  circcle
  float r1 = 300.0;
  // little cir
  float r2 = 60.0;

  // option for alternating spin w/ mouse location
  float direction = u_mouse.x > midX ? 1.0 : -1.0;

  // 
  float speed = ceil(4.0 * u_mouse.y / u_resolution.y);
  float velocity = direction * speed;
  
  // locating center of LED
  float theta = 0.5 * PI * u_time;
  float x1 = u_mouse.x + r1 * cos(velocity * theta);
  float y1 = u_mouse.y + r1 * sin(velocity * theta);
  float x2 = u_mouse.x - r1 * cos(velocity * theta);
  float y2 = u_mouse.y - r1 * sin(velocity * theta);

  float x3 = u_mouse.x + r1 * cos(velocity * theta + PI / 2.0);
  float y3 = u_mouse.y + r1 * sin(velocity * theta + PI / 2.0);
  float x4 = u_mouse.x - r1 * cos(velocity * theta + PI / 2.0);
  float y4 = u_mouse.y - r1 * sin(velocity * theta + PI / 2.0);

  if( myDist(x1, y1,  gl_FragCoord.x, gl_FragCoord.y) < r2) {
    gl_FragColor = GRN_4 * vec4(fadeLED(myDist(x1, y1,  gl_FragCoord.x, gl_FragCoord.y), r2), 1.0);
  }

  if( myDist(x2, y2,  gl_FragCoord.x, gl_FragCoord.y) < r2) {
    gl_FragColor = GRN_4 * vec4(fadeLED(myDist(x2, y2,  gl_FragCoord.x, gl_FragCoord.y), r2), 1.0);
  }

  if( myDist(x3, y3,  gl_FragCoord.x, gl_FragCoord.y) < r2) {
       gl_FragColor = BLU_4 * vec4(fadeLED(myDist(x3, y3,  gl_FragCoord.x, gl_FragCoord.y), r2), 1.0); 
  }

  if( myDist(x4, y4,  gl_FragCoord.x, gl_FragCoord.y) < r2) {
    gl_FragColor = BLU_4 * vec4(fadeLED(myDist(x4, y4,  gl_FragCoord.x, gl_FragCoord.y), r2), 1.0);
  }

  if(myDist(gl_FragCoord.x, gl_FragCoord.y, u_mouse.x, u_mouse.y) < r2) {
    gl_FragColor = RED_4 * vec4(fadeLED(myDist(u_mouse.x, u_mouse.y, gl_FragCoord.x, gl_FragCoord.y), r2), 1.0);
  }
  
}
