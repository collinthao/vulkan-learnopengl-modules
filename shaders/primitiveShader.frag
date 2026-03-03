#version 450 core

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

	vec3 fc = FragColor;
		
	float ambientStrength = 0.1;
	float specularStrength = 0.5;

	vec3 lightColor = LightColor;

	vec3 ambient = ambientStrength * lightColor;

	vec3 lightPos = LightPos;
	
	vec3 lightDir = normalize(lightPos - FragPos);

	float diff = max(dot(norm, lightDir), 0.);
	
	vec3 diffuse = diff * lightColor;

	vec3 cameraDir = normalize(CameraPos - FragPos);

	vec3 reflectDir = reflect(-lightDir, norm);

	float spec = pow(max(dot(cameraDir, reflectDir), 0.), 32.);

	vec3 specular = specularStrength * spec * lightColor;

	vec3 finalColor = (ambient + diffuse + specular) * fc;

	fragColor = vec4(finalColor, 1.);
}
