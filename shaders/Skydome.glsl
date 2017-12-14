/*
 *          SkyBox.glsl
 *
 */

//------------------------------------------------------------------------------


-- Vertex

// IN
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// OUT
out vec2 vTexCoord;

// UNIFORM
uniform mat4 uModelViewProjMatrix;

void main()
{
  gl_Position = uModelViewProjMatrix * inPosition;
  vec3 D = normalize(inNormal);
  float pi = 3.141529;
  float r = 1/pi * acos(D.z) / sqrt(D.x*D.x + D.y*D.y);
  vTexCoord = D.xy * r * 0.5 + 0.5;
  // float u = inTexCoord.x * 2 - 1;
  // float v = inTexCoord.y * 2 - 1;
  // vTexCoord = vec2(atan(v,u), pi*sqrt(u*u + v*v));
}


--

//------------------------------------------------------------------------------


-- Fragment

// IN
in vec2 vTexCoord;

// OUT
layout(location = 0) out vec4 fragColor;

// UNIFORM
uniform sampler2D tex;

void main()
{  
  fragColor = texture(tex, vTexCoord) * 10;
}

