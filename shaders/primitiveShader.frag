#version 450 core

layout (location = 0) out vec4 fragColor;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 FragPos;
layout (location = 4) in vec3 LightPos;
layout (location = 5) in vec3 LightColor;

void main()
{
	vec3 norm = normalize(Normal);

	vec3 fc = vec3(vec2(1., 1.), 0.);
		
	float ambientStrength = 0.1;
	
	vec3 lightColor = LightColor;

	vec3 ambient = lightColor * ambientStrength;

	vec3 lightPos = LightPos;
	
	vec3 lightDir = normalize(lightPos - FragPos);

	float diff = max(dot(norm, lightDir), 0.);
	
	vec3 diffuse = diff * lightColor;

	vec3 finalColor = (ambient + diffuse) * fc;

	fragColor = vec4(finalColor, 1.);
}
