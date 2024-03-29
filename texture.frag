#version 150 

// Fragment shader for texture mapping 

#define PI_INV 0.31830988618379067153776752674 

uniform int gouraudMode;
uniform int seamFix;
uniform int gi;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 objColor;
uniform sampler2D sampler0;
uniform sampler2D sampler1;
uniform sampler2D sampler2;
uniform sampler2D sampler3;
uniform sampler2D sampler4;
uniform sampler2D sampler5;
uniform sampler2D sampler6;
uniform sampler2D sampler7;
uniform sampler2D sampler8;
uniform sampler2D sampler9;
uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shexp;

in vec4 fragColor;
in vec3 texCoord;
in vec3 vnrm;

out vec4 color;

void main() {

  vec4 c;
  vec2 tc;
  if (seamFix!=0) { // without seam
    // recover theta from the vertex shader
    // atan(sin(theta)/cos(theta))
    tc.x = -0.5 * PI_INV * atan(texCoord.z, texCoord.x) ;
    tc.y = texCoord.y;
  }
  else {
    tc = texCoord.xy;
  }

  switch (gi)
  {
    case 0:
      c = texture(sampler0, tc);
      break;
    case 1:
      c = texture(sampler1, tc);
      break;
    default:
      c.rgb = objColor;
  }

  if (gouraudMode != 0) { // in Gouraud mode
    color = fragColor;
  }
  else { // in Phong mode
    vec3 diff = Kd * max(0.0, dot(vnrm, lightDir)) * c.rgb;
    vec3 amb = Ka * c.rgb;

    vec3 r = normalize(reflect(-normalize(lightDir), normalize(vnrm)));
    float vnrmdotr = max(0.0, dot(normalize(vnrm), r));
    vec3 spec = Ks * pow(vnrmdotr, shexp) * lightColor;

    color.rgb = diff + amb + spec;
    color.a = 1.0;

    color.rgb = c.rgb;
  }
}
