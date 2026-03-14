#version 450 core

#define MAX_POINT_LIGHTS 4

struct DirectionalLight
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 position; 
	vec3 direction; 
	float constant;
	float linear;
	float quadratic;
	float cutOff;
	float outerCutOff;
	// figure out later; not reading type as correct value
	//int type;
};

struct PointLight
{
	vec3 ambient; 
	vec3 diffuse; 
	vec3 specular; 
	vec3 position; 
	vec3 direction; 
	float constant;
	float linear;
	float quadratic;
	float cutOff;
	float outerCutOff;
	// figure out later; not reading type as correct value
	//int type;
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
	//DirectionalLight directionalLight;
} lights;

layout(binding = 4) uniform sampler2D specularTexture;

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 FragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 FragPos;
layout (location = 4) in vec3 LightPos;
layout (location = 5) in vec3 LightColor;
layout (location = 6) in vec3 CameraPos;


vec3 calculatePointLights(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir);
	
void main()
{
	vec3 norm = normalize(Normal);

// spotoff
/*	float theta = dot(lightDir, normalize(-u_light.direction));
	float epsilon = u_light.cutOff - u_light.outerCutOff;
	float intensity = clamp((theta - u_light.outerCutOff)/epsilon, 0.0, 1.);
*/
		vec3 cameraDir = normalize(CameraPos - FragPos);

//		diffuse *= intensity;
//		specular *= intensity;

		vec3 result = calculatePointLights(lights.pointLights[0], norm, FragPos, cameraDir);

		for (int i = 1; i < MAX_POINT_LIGHTS; i++)
		{
			result += calculatePointLights(lights.pointLights[i], norm, FragPos, cameraDir);	
		}

	fragColor = vec4(result, 1.);
}

vec3 calculateDirectionalLight()
{
	return vec3(1.0);
}

vec3 calculatePointLights(PointLight pointLight, vec3 normal, vec3 fragPos, vec3 viewDir)
{
		vec3 lightDir = normalize(pointLight.position - fragPos);
		
		float diff = max(dot(normal, lightDir), 0.);
		
		vec3 reflectDir = reflect(-pointLight.position, normal);

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
