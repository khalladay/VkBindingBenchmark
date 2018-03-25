#version 450 core
#extension GL_ARB_separate_shader_objects : enable

struct tdata
{
	mat4 mvp;
	mat4 it_mv;
};

layout(binding=0,set=0) uniform TRANSFORM_DATA
{
	tdata d[256];
}transform;

layout(push_constant) uniform transformData
{
	uint tform;
}idx;

layout(location=0) in vec3 vertex;
layout(location=1) in vec2 uv;
layout(location=2) in vec3 normal;

layout(location=0) out vec2 fragUV;
layout(location=1) out vec3 fragNorm;

void main()
{
	gl_Position = transform.d[idx.tform].mvp * vec4(vertex, 1.0);
	fragNorm =  (transform.d[idx.tform].it_mv * vec4(normal, 0.0)).xyz; 

	fragUV = uv;
}