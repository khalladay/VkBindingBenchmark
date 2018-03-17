#include "ssbo_store.h"
#include <glm/gtx/transform.hpp>
#include <deque>
#include "shader_inputs.h"
#include "config.h"
namespace ssbo_store
{
#if DEVICE_LOCAL
	char* map;
#else
	void* map;
#endif

	vkh::VkhContext* ctxt;
	uint32_t num;

	VkBuffer buf;
	vkh::Allocation alloc;
	std::deque<uint32_t> freeIndices;

#if PERSISTENT_STAGING_BUFFER
	VkBuffer stagingBuffer;
	vkh::Allocation stagingAlloc;
#endif

	void init(vkh::VkhContext& _ctxt)
	{
		ctxt = &_ctxt;
		num = 32000;

		vkh::createBuffer(
			buf, 
			alloc, 
			sizeof(VShaderInput) * num, 
#if DEVICE_LOCAL
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
#else
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
#endif
			_ctxt);

#if DEVICE_LOCAL
		map = (char*)malloc(sizeof(VShaderInput) * num);
#else
		vkMapMemory(_ctxt.device, alloc.handle, alloc.offset, alloc.size, 0, &map);
#endif

#if PERSISTENT_STAGING_BUFFER
		vkh::createBuffer(
			stagingBuffer,
			stagingAlloc,
			sizeof(VShaderInput) * num,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
			_ctxt);
#endif

		for (uint32_t i = 0; i < num; ++i)
		{
			freeIndices.push_back(i);
		}

	}

	bool acquire(uint32_t& outIdx)
	{
		if (freeIndices.size() == 0)
		{
			printf("NO SLOTS LEFT IN SSBO\n");
			return false;
		}

		outIdx = freeIndices.front();
		freeIndices.pop_front();

		//we may have to page this at some point, reserve
		//3 bits for page id. 
		outIdx = outIdx << 3;

		return true;

	}

	uint32_t getNumPages()
	{
		return 1;
	}

	VkBuffer& getPage(uint32_t idx)
	{
		return buf;
	}

	vkh::Allocation& getAlloc(uint32_t idx)
	{
		return alloc;
	}

	void updateBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, VkCommandBuffer* commandBuffer, vkh::VkhContext& ctxt)
	{
		VShaderInput* objPtr = (VShaderInput*)map;

		for (uint32_t i = 0; i < num; ++i)
		{
			objPtr[i].model = projMatrix * viewMatrix;
			objPtr[i].normal = glm::transpose(glm::inverse(viewMatrix));
		}

		VkMappedMemoryRange range;
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.offset = alloc.offset;
		range.pNext = nullptr;
		range.size = num * sizeof(VShaderInput);
		range.memory = alloc.handle;


		#if PERSISTENT_STAGING_BUFFER || !DEVICE_LOCAL
				vkFlushMappedMemoryRanges(ctxt.device, 1, &range);	
		#endif

		#if DEVICE_LOCAL
			#if PERSISTENT_STAGING_BUFFER		
				vkh::copyBuffer(stagingBuffer, buf, range.size, 0, 0, nullptr, ctxt);		
			#else		
				vkh::copyDataToBuffer(&buf, range.size, 0, (char*)map, commandBuffer, ctxt);	
			#endif	
		#endif
		
	}

	VkDescriptorType getDescriptorType()
	{
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}

}