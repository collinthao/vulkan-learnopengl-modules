#version 450 core

layout (binding = 1) uniform Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
} u_material;

layout (binding = 3) uniform Light
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 position; 
} u_light;

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 FragColor;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 FragPos;
layout (location = 4) in vec3 LightPos;
layout (location = 5) in vec3 LightColor;
layout (location = 6) in vec3 CameraPos;

void main()
{
	vec3 norm = normalize(Normal);

	vec3 lightColor = LightColor;

	vec3 ambient = u_material.ambient * u_light.ambient;

	vec3 lightPos = u_light.position;
	
	vec3 lightDir = normalize(lightPos - FragPos);

	float diff = max(dot(norm, lightDir), 0.0f);
	
	vec3 diffuse = (diff * u_material.diffuse) * u_light.diffuse;

	vec3 cameraDir = normalize(CameraPos - FragPos);

	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(cameraDir, reflectDir), 0.0f), u_material.shininess);

	vec3 specular = u_light.specular * (spec * u_material.specular);

	vec3 finalColor = ( ambient +  diffuse +  specular);

	fragColor = vec4(finalColor, 1.);
}
