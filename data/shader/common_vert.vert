#version 450 core
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform globalData
{
	mat4 view_projection;
}global_data;

layout(set = 1, binding = 0) uniform transformData
{
	mat4 model;
}transform_data;

layout(location=0) in vec3 vertex;
layout(location=1) in vec2 uv;

layout(location=0) out vec2 fragUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = vec4(vertex, 1.0) * transform_data.model * global_data.view_projection;
	fragUV = uv;
}