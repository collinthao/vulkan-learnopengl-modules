#version 450

layout(binding = 0) uniform UniformBufferObjectModel
{
    mat4 model; 
	mat4 view;
	mat4 proj;
} ubom;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 FragPos;

void main()
{
    gl_Position = ubom.proj * ubom.view * ubom.model * vec4(inPosition, 1.0);
    //fragColor = inColor;
    fragTexCoord = inTexCoord;
    Normal = inNormal;
    FragPos = vec3(ubom.model * vec4(inPosition, 1.0));
}
