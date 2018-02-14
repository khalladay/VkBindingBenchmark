#pragma once
#include "vkh.h"
#include "file_utils.h"
#include "vkh_initializers.h"
#include "vkh_mesh.h"
#include <vector>

namespace vkh
{
	struct VkhMaterialCreateInfo
	{
		VkRenderPass renderPass;

		std::vector<VkDescriptorSetLayout> descSetLayouts;
		VkPipelineLayout* outPipelineLayout;
		VkPipeline* outPipeline;
		uint32_t pushConstantRange;
		VkShaderStageFlagBits pushConstantStages;
	};

	void createBasicMaterial(const char* vShaderPath, const char* fShaderPath, VkhContext& ctxt, VkhMaterialCreateInfo& createInfo);
}