#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform transformData
{
	mat4 mvp;
}transform_data;

layout(location=0) in vec3 vertex;
layout(location=1) in vec2 uv;

layout(location=0) out vec2 fragUV;


void main()
{
	gl_Position =  transform_data.mvp * vec4(vertex, 1.0);
	fragUV = uv;
}