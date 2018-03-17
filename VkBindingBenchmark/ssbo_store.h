#pragma once
#include <stdint.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "vkh.h"

namespace ssbo_store
{
	void init(vkh::VkhContext& ctxt);
	bool acquire(uint32_t& outIdx);
	uint32_t getNumPages();
	VkBuffer& getPage(uint32_t idx);
	vkh::Allocation& getAlloc(uint32_t idx);
	void updateBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, VkCommandBuffer* commandBuffer, vkh::VkhContext& ctxt);
	VkDescriptorType getDescriptorType();
}
