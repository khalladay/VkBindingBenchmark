#pragma once
#include "vkh.h"
#include "file_utils.h"
#include "vkh_initializers.h"
#include "vkh_mesh.h"
#include <vector>

namespace vkh
{
	struct GlobalShaderData
	{
		__declspec(align(16)) glm::float32 time;
		__declspec(align(16)) glm::float32 lightIntensity;
		__declspec(align(16)) glm::vec2 resolution;
		__declspec(align(16)) glm::vec4 lightDir;
		__declspec(align(16)) glm::vec3 lightCol;
		__declspec(align(16)) glm::vec4 mouse;
		__declspec(align(16)) glm::vec4 worldSpaceCameraPos;
		__declspec(align(16)) glm::mat4 vpMatrix;
	};

	struct GlobalShaderDataStore
	{
		//storage for global shader data
		vkh::Allocation			mem;
		VkBuffer				buffer;
		GlobalShaderData		shaderData;
		uint32_t				size;
		void*					mappedMemory;

		VkDescriptorSetLayout	layout;
		VkDescriptorSet			descSet;
		VkSampler				sampler;
	};

	struct VkhMaterialCreateInfo
	{
		VkRenderPass renderPass;

		std::vector<VkDescriptorSetLayout> descSetLayouts;
		VkPipelineLayout* outPipelineLayout;
		VkPipeline* outPipeline;
		uint32_t pushConstantRange;
		VkShaderStageFlagBits pushConstantStages;
	};

	void initGlobalShaderData();
	void createBasicMaterial(const char* vShaderPath, const char* fShaderPath, VkhContext& ctxt, VkhMaterialCreateInfo& createInfo);
}