#version 450 

layout(binding = 0) uniform PointLight
{	
	mat4 model;
	mat4 view;
	mat4 projection;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 position;
	vec3 color;
	float constant;
	float linear;
	float quadratic;
} u_PointLight;

layout(location = 0) in vec3 inPosition;
layout(location = 0) out vec3 lightColor;

void main()
{
	gl_Position = u_PointLight.projection * u_PointLight.view * u_PointLight.model * vec4(inPosition, 1.0);	
	lightColor = u_PointLight.color;
}
