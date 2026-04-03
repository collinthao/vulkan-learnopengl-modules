#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D texture_sampler;

void main()
{
	fragColor = texture(texture_sampler, texCoords);
}
