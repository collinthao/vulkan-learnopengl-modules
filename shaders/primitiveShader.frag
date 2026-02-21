#version 450 core

layout (location = 0) out vec4 fragColor;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 FragPos;

void main()
{
	vec3 fc = vec3(vec2(1.), 0.);
		
	float ambientStrength = 0.1;
	
	vec3 ambient = fc * ambientStrength;

	vec3 lightPos = vec3(10., 0., 3.);
	
	vec3 lightDir = normalize(lightPos - FragPos);

	float diffuse = max(dot(lightDir, Normal), 0.);

	vec3 finalColor = (ambient + diffuse) * fc;

	fragColor = vec4(finalColor, 1.);
}
