#version 150

// Vertex shader for texture mapping 

#define PI 3.14159265358979323846264338327

uniform int gouraudMode;
uniform int seamFix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform vec3 lightDir;  // assumed to be unit-length (already normalized)
uniform vec3 lightColor;
uniform vec3 objColor;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shexp;

in vec4 vertPos;
in vec3 vertRgb;
in vec3 vertNorm;
in vec2 vertTex2;

out vec4 fragColor;
out vec3 texCoord;
out vec3 vnrm;

void main() {

  // transform vertices 
  gl_Position = projMatrix * viewMatrix * modelMatrix * vertPos;

  // set up texture coordinate
    // with seam fix
    // set texCoord.x and z to sin(theta) and cos(theta) respectively
    texCoord.x = sin(2 * PI * vertTex2.x + 0.5 * PI); 
    texCoord.z = cos(2 * PI * vertTex2.x + 0.5 * PI);
    texCoord.y = vertTex2.y;

  // in Phong mode
    // surface normal in view coords
    vnrm = normalMatrix * vertNorm;

    fragColor.rgb = objColor;
    fragColor.a = 1.0;
}
