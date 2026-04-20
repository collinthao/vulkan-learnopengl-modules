#version 450

layout (location = 0) out vec4 fragColor;
layout (location = 0) in vec2 texCoords ;

layout (binding = 0) uniform sampler2D texture_sampler;

const float offset = 1./300.;

void main()
{
	vec2 offsets[9] = vec2[](
	vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
	);
/*
    float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
        -1, -1, -1
    );
 */
    float kernel[9] = float[](
        -2, -1, 0,
        -1,  1, 1,
        0, 1, 2
    );
    
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(texture_sampler, texCoords + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];
	
fragColor = vec4(col, 1.);
fragColor = texture(texture_sampler, texCoords);
}
