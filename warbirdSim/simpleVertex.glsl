/*
diffuseVertex.glsl

Fragment shader for light effects.
Adapted from OpenGL Programming Guide 8th edition sample code 
ch08_lightmodels.cpp function render_vs().

Mike Barnes
9/24/2015
*/

# version 330 core
        
layout (location = 0) in vec4 vPosition;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec3 vNormal;

uniform mat3 NormalMatrix;
uniform mat4 ModelViewProjection;
uniform mat4 ViewMatrix;
  
out vec4 vsColor;      
out vec3 vs_worldpos;
out vec3 vs_normal;
out vec3 vs_lightpos;

uniform vec4 lightpos = vec4(0.0f,0.0f,0.0f,1.0f);
        
void main(void) {
  vec4 position = ModelViewProjection * vPosition;
  vec4 view_lightpos = (ViewMatrix * lightpos); //Transform Point Light Position to Camera Space
  vs_lightpos = view_lightpos.xyz; //Vec4 to Vec3
  gl_Position = position;
  vs_worldpos = position.xyz;
  vs_normal = NormalMatrix * vNormal;
  vsColor = vColor;
  }
 