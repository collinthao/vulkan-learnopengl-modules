#version 450 core

int DIRECTIONAL = 0;
int ATTENUATION = 1;
int SPOTLIGHT = 2;

layout (binding = 1) uniform Material
{
	vec3 specular;
	float shininess;
} u_material;

layout(binding = 2) uniform sampler2D texSampler;

layout (binding = 3) uniform Light
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
} u_light;

layout(binding = 4) uniform sampler2D specular;

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec3 FragColor;
layout (location = 1) in vec2 fragTexCoord;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 FragPos;
layout (location = 4) in vec3 LightPos;
layout (location = 5) in vec3 LightColor;
layout (location = 6) in vec3 CameraPos;

void main()
{
	vec3 tex = vec3(texture(texSampler, fragTexCoord));
	vec3 specTex = vec3(texture(specular, fragTexCoord));

	vec3 norm = normalize(Normal);

	vec3 lightPos = u_light.position;
	vec3 lightDir = normalize(lightPos - FragPos);
	vec3 lightColor = LightColor;
// spotoff
	float theta = dot(lightDir, normalize(-u_light.direction));
	float epsilon = u_light.cutOff - u_light.outerCutOff;
	float intensity = clamp((theta - u_light.outerCutOff)/epsilon, 0.0, 1.);

		vec3 ambient = tex * u_light.ambient;

//		vec3 lightDir = normalize(-u_light.direction);

		float diff = max(dot(norm, lightDir), 0.0f);
		
		vec3 diffuse = (diff * tex) * u_light.diffuse;

		vec3 cameraDir = normalize(CameraPos - FragPos);

		vec3 reflectDir = reflect(-lightDir, norm);

		float spec = pow(max(dot(cameraDir, reflectDir), 0.0f), u_material.shininess);

		vec3 specular = u_light.specular * (spec * specTex);

		// attenuation
		float lightDistance = length(u_light.position - FragPos);
		float attenuation = 1.0/(u_light.constant + u_light.linear * lightDistance + u_light.quadratic * (lightDistance * lightDistance));

		diffuse *= intensity;
		specular *= intensity;

		vec3 finalColor = ( ambient +  diffuse +  specular);

		diffuse *= attenuation;
		specular *= attenuation;

		finalColor = (ambient + diffuse + specular);


		fragColor = vec4(finalColor, 1.);
}
