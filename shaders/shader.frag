#version 450

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec3 fragColor;



void main()
{

	vec3 lightPos = vec3(10., 0., 0.);

	outColor = vec4(fragColor, 1.);
}
