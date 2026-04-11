#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 position;
layout (binding = 0) uniform samplerCube cubemapTexture;

void main()
{ 
	fragColor = texture(cubemapTexture, position);
}
