#pragma once
#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>

const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 forward = glm::vec3(0.0f, 0.0f, -1.0f);


namespace Camera
{
	struct CamImpl;

	struct Cam
	{
		CamImpl* impl;
	};

	void init(Cam& cam);

	void setPosition(Cam& cam, glm::vec3 pos);
	void translate(Cam& cam, glm::vec3 translation);
	void lookAt(Cam& cam, glm::vec3 point, glm::vec3 camPos);
	void rotate(Cam& cam, glm::vec3 axis, float angle);

	glm::vec3 localRight(Cam& cam);
	glm::vec3 localForward(Cam& cam);
	glm::vec3 localUp(Cam& cam);

	glm::mat4 viewMatrix(Cam& cam);
}