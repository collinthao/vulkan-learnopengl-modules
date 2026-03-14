#version 450 

layout(binding = 0) uniform UniformBufferObjectModel
{
mat4 model;
mat4 view;
mat4 proj;
mat4 lightModels[4];
vec3 lightPositions[4];
vec3 lightColor;
vec3 fragColor;
vec3 cameraPos;
} ubom;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 lightColor;

void main()
{
	gl_Position = ubom.proj * ubom.view * ubom.lightModel * vec4(inPosition, 1.0);	
	lightColor = ubom.lightColor;
}
