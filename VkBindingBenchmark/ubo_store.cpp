#include "ubo_store.h"
#include "vkh.h"
#include "config.h"


namespace ubo_store
{
	VkBuffer globalUBOPages;
	vkh::Allocation globalUBOAllocs;

	uint32_t uboArrayLen;
	uint32_t countPerPage;
	uint32_t numPages;
	uint32_t size;

	std::deque<uint32_t> freeIndices;

	void init(int num, vkh::VkhContext& ctxt)
	{
		uboArrayLen = num;
		for (uint32_t i = 0; i < num; ++i)
		{
			freeIndices.push_back(i);
		}
		countPerPage = 511;
		numPages = (num / countPerPage) + 1;
		size = (sizeof(ObjectUBO) * 511);

		/*
#if GLOBAL_UBO_ARRAY

		countPerPage = 32;
		numPages = (num / countPerPage) + 1;
		size = (sizeof(ObjectUBO) * 32);

		vkh::createBuffer(
			wtf,
			wtfA,
			size,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			ctxt);
#endif*/
	}

	vkh::Allocation& getAlloc()
	{
		return globalUBOAllocs;
	}

	VkBuffer& getUBOPage(uint32_t idx)
	{
		return globalUBOPages;
	}

	bool acquire(uint32_t& outIdx)
	{
		if (freeIndices.size() > 0)
		{
			outIdx = freeIndices.front();
			freeIndices.pop_front();

			//bit shift the page index
			outIdx = outIdx << 3;
			outIdx += (outIdx / countPerPage);

			return true;
		}
		else
		{
			return false;
		}
	}

	uint32_t getNumPages()
	{
		return numPages;
	}

	void updateBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, vkh::VkhContext& ctxt)
	{
	/*	uint32_t size = globalUBOAllocs.size;

		void* map;
		VkResult res = vkMapMemory(ctxt.device, globalUBOAllocs.handle, globalUBOAllocs.offset, globalUBOAllocs.size, 0, &map);
		ObjectUBO* objPtr = (ObjectUBO*)map;

		for (uint32_t i = 0; i < 32; ++i)
		{
			objPtr[i].model = projMatrix * viewMatrix;
			objPtr[i].normal = glm::transpose(glm::inverse(viewMatrix));
		}

		vkUnmapMemory(ctxt.device, globalUBOAllocs.handle);*/
	//	vkh::copyDataToBuffer(&globalUBOPages, sizeof(ObjectUBO) * uboArrayLen, 0, (char*)uboArray, ctxt);	
	}
}