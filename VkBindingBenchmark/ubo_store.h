#pragma once
#include <stdint.h>
#include "vkh.h"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <deque>

#include "config.h"


//for UBO / SSBO testing, we'll use transform data
__declspec(align(16)) struct ObjectUBO
{
	__declspec(align(16)) glm::mat4 model;
	__declspec(align(16)) glm::mat4 normal;
};

namespace ubo_store
{
	struct global
	{
		VkBuffer globalUBOPages;
		vkh::Allocation globalUBOAllocs;

		uint32_t uboArrayLen;
		uint32_t countPerPage;
		uint32_t numPages;
		uint32_t size;

		std::deque<uint32_t> freeIndices;

	};

	extern global uboGlobal;

	void init(int num, vkh::VkhContext& ctxt);
	bool acquire(uint32_t& outIdx);
	uint32_t getNumPages();
	VkBuffer& getUBOPage(uint32_t idx);
	vkh::Allocation& getAlloc();
	void updateBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, vkh::VkhContext& ctxt);
}