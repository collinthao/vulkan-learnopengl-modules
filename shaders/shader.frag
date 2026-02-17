#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 normal;
layout(location = 1) in vec3 fragPos;



void main()
{

	vec3 norm = normalize(normal);

	vec3 ambient = vec3(1., 0., 0.) * 0.1f;

	vec3 lightPos = vec3(10., 3., 4.);
	
	vec3 lightDir = normalize(lightPos - fragPos);

	float diffuse = max(dot(lightDir, norm), 0.);

	vec3 fragColor = (ambient + diffuse) * vec3(1., 0., 0.);	

	outColor = vec4(fragColor, 1.);
}
