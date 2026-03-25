#version 450

layout(binding = 0) uniform UniformBufferObjectModel
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

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 Normal;
layout(location = 3) out vec3 CameraPos;
layout(location = 4) out vec3 FragPos;

void main()
{
    gl_Position = ubom.proj * ubom.view * ubom.model * vec4(inPosition, 1.0);
    fragColor = ubom.fragColor;
    fragTexCoord = vec2(inTexCoord.x, -inTexCoord.y);
    Normal = mat3(transpose(inverse(ubom.model)))* inNormal;
    CameraPos = ubom.cameraPos;
    FragPos = vec3(ubom.model * vec4(inPosition, 1.));
}
