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
out vec3 vTexCoord;

// UNIFORM
uniform mat4 uModelViewProjMatrix;

void main()
{
  gl_Position = uModelViewProjMatrix * inPosition;
  vTexCoord = inNormal;
}


--

//------------------------------------------------------------------------------


-- Fragment

// IN
in vec3 vTexCoord;

// OUT
layout(location = 0) out vec4 fragColor;

// UNIFORM
uniform sampler2D tex;

vec3 ToneMapACES( vec3 hdr )
{
    const float A = 2.51, B = 0.03, C = 2.43, D = 0.59, E = 0.14;
    return clamp((hdr * (A * hdr + B)) / (hdr * (C * hdr + D) + E), 0, 1);
}

vec3 ApplySRGBCurve( vec3 x )
{
	float c = 1.0/2.2;
	vec3 f = vec3(c, c, c); 
	return pow(x, f);
}

void main()
{  
  vec3 D = normalize(vTexCoord);
  float pi = 3.141529;
  float r = 1/pi * acos(D.z) / sqrt(D.x*D.x + D.y*D.y);
  vec2 coord = D.xy * r * 0.5 + 0.5;

  vec3 color = texture(tex, coord).xyz;
  color *= 10;
  color = ApplySRGBCurve(ToneMapACES(color));
  fragColor = vec4(color, 1);
}

