#version 450

layout(binding = 1) uniform UniformBufferObjectModel
{
mat4 model;
mat4 view;
mat4 proj;
vec3 fragColor;
vec3 cameraPos;
} ubom;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 position;

void main()
{
	gl_Position = ubom.proj * ubom.view * ubom.model * vec4(inPosition, 1.);
	position = inPosition;
}
