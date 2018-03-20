#pragma once
#include "null_store.h"

namespace null_store
{
	void init(vkh::VkhContext& ctxt) {}
	bool acquire(uint32_t& outIdx) { return true; }
	uint32_t getNumPages() { return 0; }
	VkBuffer& getPage(uint32_t idx) { VkBuffer buf; return buf; }
	vkh::Allocation& getAlloc(uint32_t idx) { vkh::Allocation alloc = {}; return alloc; }

	void updateBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, VkCommandBuffer* commandBuffer, vkh::VkhContext& ctxt) {}
	VkDescriptorType getDescriptorType() { return VK_DESCRIPTOR_TYPE_MAX_ENUM; }

}