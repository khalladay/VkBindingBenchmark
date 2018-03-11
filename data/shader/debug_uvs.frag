#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(location=0) in vec2 fragUV;
layout(location=1) in vec3 fragNormal;
layout(location=0) out vec4 outColor;

void main()
{
	vec3 norm = normalize(fragNormal);
	outColor = vec4(fragUV.x, fragUV.y, 1.0, 1.0);
}