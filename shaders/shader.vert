#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model; 
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 normal;
layout(location = 1) out vec3 fragPos;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(vec2(inPosition), inPosition.z, 1.0);
    normal = inNormal;
    fragPos = vec3(ubo.model * vec4(inPosition, 1.));
}
