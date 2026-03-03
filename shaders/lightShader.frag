#version 450 core

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec3 lightColor;

void main()
{
	fragColor = vec4(lightColor, 1.);
}
