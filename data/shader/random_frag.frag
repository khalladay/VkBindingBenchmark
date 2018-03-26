#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec2 fragUV;
layout(location=1) in vec3 fragNormal;
layout(location=0) out vec4 outColor;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	vec3 norm = normalize(fragNormal);

	outColor.x = rand(vec2(sin(norm.x), cos(norm.y)));
	outColor.y = rand(vec2(sin(norm.y), tan(norm.z)));
	outColor.z = rand(vec2(sin(norm.z), cos(norm.x)));

	for (int i = 0; i < 24; ++i)
	{
		outColor.x = rand(vec2(sin(outColor.x), cos(outColor.y)));
		outColor.y = rand(vec2(sin(outColor.y), tan(outColor.z)));
		outColor.z = rand(vec2(sin(outColor.z), cos(outColor.x)));
	}

	outColor.a = 1.0f;
}