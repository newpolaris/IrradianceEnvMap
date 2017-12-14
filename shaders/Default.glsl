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
layout(location = 0) out vec4 color;

uniform sampler2D tex;

void main()
{
	color = texture(tex, vTexCoord);
}
