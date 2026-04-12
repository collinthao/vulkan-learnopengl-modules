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
	vec4 pos = ubom.proj * ubom.view * vec4(inPosition, 1.);
	gl_Position = pos.xyww;
	position = inPosition;
}
