-- Vertex
layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// OUT
out vec2 vTexCoord;

// UNIFORM
uniform mat4 uModelViewProjMatrix;

void main()
{
	gl_Position = uModelViewProjMatrix * vec4( inPosition.xyz, 1.0f);
	vTexCoord = inTexCoord;
}

-- Fragment

// IN
in vec2 vTexCoord;

// out
layout(location = 0) out vec4 outColor;

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
	vec4 color = texture(tex, vTexCoord);
	color.xyz *= 10;
	color.xyz = ApplySRGBCurve(ToneMapACES(color.xyz));
	outColor = color;
}
