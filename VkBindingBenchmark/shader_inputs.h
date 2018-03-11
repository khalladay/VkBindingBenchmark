#pragma once
#include <glm\glm.hpp>

//for UBO / SSBO testing, we'll use transform data
struct VShaderInput
{
	glm::mat4 model;
	glm::mat4 normal;
};
