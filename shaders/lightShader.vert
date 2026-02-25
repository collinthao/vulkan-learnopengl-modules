#version 450 

layout(binding = 0) uniform UniformBufferObjectModel
{
    	mat4 model; 
	mat4 view;
	mat4 proj;
	mat4 lightModel;
	vec3 lightColor;
} ubom;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = ubom.proj * ubom.view * ubom.lightModel * vec4(inPosition, 1.0);	
}
