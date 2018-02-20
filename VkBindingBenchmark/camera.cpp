#include "camera.h"
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace Camera
{
	struct CamImpl
	{
		glm::mat4 viewMatrix = glm::mat4();
		glm::vec3 localPos = glm::vec3();
		glm::quat localRot = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);

		glm::vec4 localForward = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
		glm::vec4 localUp = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
		glm::vec4 localRight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
	};

	void updateMatrix(CamImpl& cam)
	{
		
		cam.viewMatrix = mat4_cast(cam.localRot);
		cam.viewMatrix = glm::translate(cam.viewMatrix, cam.localPos);

		cam.localForward = glm::vec4(0.0, 0.0, 1.0, 0.0) * cam.viewMatrix;
		cam.localRight = glm::vec4(1.0, 0.0, 0.0, 0.0) * cam.viewMatrix;
		cam.localUp = glm::vec4(0.0, 1.0, 0.0, 0.0) * cam.viewMatrix;

	}

	glm::vec3 localRight(Cam& cam)
	{
		return glm::vec3(cam.impl->localRight.x, cam.impl->localRight.y, cam.impl->localRight.z);
	}

	glm::vec3 localForward(Cam& cam)
	{
		return glm::vec3(cam.impl->localForward.x, cam.impl->localForward.y, cam.impl->localForward.z);
	}

	glm::vec3 localUp(Cam& cam)
	{
		return glm::vec3(cam.impl->localUp.x, cam.impl->localUp.y, cam.impl->localUp.z);
	}

	void setPosition(Cam& cam, glm::vec3 pos)
	{
		CamImpl& impl = *cam.impl;
		impl.localPos = pos;
		updateMatrix(impl);
	}

	void translate(Cam& cam, glm::vec3 translation)
	{
		CamImpl& impl = *cam.impl;
		impl.localPos += translation;
		updateMatrix(impl);
	}

	void lookAt(Cam& cam, glm::vec3 point, glm::vec3 camPos)
	{
		using namespace glm;
		CamImpl& impl = *cam.impl;

		//construct a rotation matrix to get to this point (look matrix without translation)
		//convert that to a quaterion
		//set rotation
		vec3 forward = glm::normalize(impl.localPos - point);
		vec3 world_up = vec3(0.0, 1.0, 0.0);
		vec3 our_right = glm::normalize(cross(world_up, forward));
		vec3 camUp = glm::normalize(cross(forward, our_right));

		mat3 basis;
		basis[0].x = our_right.x;
		basis[1].x = our_right.y;
		basis[2].x = our_right.z;

		basis[0].y = camUp.x;
		basis[1].y = camUp.y;
		basis[2].y = camUp.z;

		basis[0].z = forward.x;
		basis[1].z = forward.y;
		basis[2].z = forward.z;


		quat q = glm::quat_cast(basis);
		impl.localRot = q;
		updateMatrix(impl);
	}

	void rotate(Cam& cam, glm::vec3 axis, float angle)
	{
		CamImpl& impl = *cam.impl;
		impl.localRot = glm::rotate(impl.localRot, angle, axis);
		updateMatrix(impl);
	}

	void init(Cam& cam)
	{
		cam.impl = new CamImpl();
	}

	glm::mat4 viewMatrix(Cam& cam)
	{
		return cam.impl->viewMatrix;
	}
}