#version 450 core

#define MAX_POINT_LIGHTS 4 

struct SpotLight
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 position; 
	vec3 direction; 
	float cutOff;
	float outerCutOff;
};

struct DirectionalLight
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 direction; 
};

struct PointLight
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
};

layout (binding = 1) uniform Material
{
	vec3 specular;
	float shininess;
} u_material;

layout(binding = 2) uniform sampler2D texSampler;

layout (binding = 3) uniform Lights
{
	PointLight pointLights[MAX_POINT_LIGHTS];
	DirectionalLight directionalLight;
	SpotLight spotLight;
} lights;

layout(binding = 4) uniform sampler2D specularTexture;

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 FragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 FragPos;
layout (location = 6) in vec3 CameraPos;

vec3 calculatePointLights(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
	
vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir);
	
vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir);

void main()
{
	vec3 norm = normalize(Normal);
	
	vec3 cameraDir = normalize(CameraPos - FragPos);

	vec3 result = calculateDirectionalLight(lights.directionalLight, norm, cameraDir);

	for (int i = 0; i < MAX_POINT_LIGHTS; i++)
	{
		result += calculatePointLights(lights.pointLights[i], norm, FragPos, cameraDir);	
	}

	//result += calculateSpotLight(lights.spotLight, norm, cameraDir);

	fragColor = vec4(result, 1.);
}

vec3 calculateSpotLight(SpotLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(light.position - FragPos);
	float theta = dot(lightDir, normalize(-light.direction));
	float epsilon = light.cutOff - light.outerCutOff;
	float intensity = clamp((theta - light.outerCutOff)/epsilon, 0.0, 1.);

	float diff = max(dot(normal, lightDir), 0.);

	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.), 32.f);

	vec3 ambient = light.ambient * vec3(texture(texSampler, fragTexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, fragTexCoord));

	ambient *= intensity;
        diffuse *= intensity;
        specular *= intensity;

	return ambient + diffuse + specular;
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir)
{
	vec3 lightDir = normalize(-light.direction);

	float diff = max(dot(normal, lightDir), 0.);

	vec3 reflectDir = reflect(-lightDir, normal);

	float spec = pow(max(dot(reflectDir, viewDir), 0.), 32.f);

	vec3 ambient = light.ambient * vec3(texture(texSampler, fragTexCoord));
	vec3 diffuse = light.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
	vec3 specular = light.specular * spec * vec3(texture(specularTexture, fragTexCoord));

	return ambient + diffuse + specular;
}

vec3 calculatePointLights(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
		vec3 lightDir = normalize(pointLight.position - fragPos);
		
		float diff = max(dot(normal, lightDir), 0.);
		
		vec3 reflectDir = reflect(lightDir, normal);

		float spec = pow(max(dot(viewDir, reflectDir), 0.f), u_material.shininess);

		float lightDistance = length(pointLight.position - fragPos);
		float attenuation = 1.0/(pointLight.constant + pointLight.linear * lightDistance + pointLight.quadratic * (lightDistance * lightDistance));

		vec3 ambient = pointLight.ambient * vec3(texture(texSampler, fragTexCoord));
		vec3 diffuse = pointLight.diffuse * diff * vec3(texture(texSampler, fragTexCoord));
		vec3 specular = pointLight.specular * spec * vec3(texture(specularTexture, fragTexCoord));

		ambient *= attenuation;
		diffuse *= attenuation;
		specular *= attenuation;
	
		vec3 finalResult = ambient + diffuse + specular;

		return finalResult;
}
