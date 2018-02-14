#pragma once

#include <vector>
#include <map>

namespace Material
{
	enum class ShaderStage : uint8_t
	{
		VERTEX,
		FRAGMENT,
		MAX
	};

	enum class InputType : uint8_t
	{
		UNIFORM,
		SAMPLER,
		SAMPLERARRAY,
		TEXTURE,
		TEXTUREARRAY,
		SEPARATETEXTURE,
		MAX
	};

	struct BlockMember
	{
		char name[32];
		uint32_t size;
		uint32_t offset;
		char defaultValue[64];
	};

	struct PushConstantBlock
	{
		uint32_t sizeBytes;
		std::vector<ShaderStage> owningStages;
		std::vector<BlockMember> blockMembers;
	};

	struct DescriptorSetBinding
	{
		InputType type;
		uint32_t set;
		uint32_t binding;
		uint32_t sizeBytes;
		uint32_t arrayLen;
		char name[32];
		char defaultValue[64];
		std::vector<ShaderStage> owningStages;
		std::vector<BlockMember> blockMembers;
	};

	struct ShaderStageDefinition
	{
		ShaderStage stage;
		char shaderPath[256];
	};

	struct ShaderInputDefinition
	{
		char name[32];
		char value[64];
	};

	struct InstanceDefinition
	{
		uint32_t parent;
		std::vector<ShaderInputDefinition> defaults;
	};

	struct Definition
	{
		PushConstantBlock pcBlock;
		std::vector<ShaderStageDefinition> stages;
		std::map<uint32_t, std::vector<DescriptorSetBinding>> descSets;
	};
}