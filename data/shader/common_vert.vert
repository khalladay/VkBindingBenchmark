#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform transformData
{
	mat4 mvp;
	mat4 it_mv;
}global;

layout(location=0) in vec3 vertex;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;

layout(location=0) out vec2 fragUV;
layout(location=1) out vec3 fragNorm;

void main()
{
	gl_Position =  global.mvp * vec4(vertex, 1.0);

	fragUV = uv;
	fragNorm =  (global.it_mv * vec4(normal, 0.0)).xyz; 
}