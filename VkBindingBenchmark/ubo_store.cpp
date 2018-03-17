#include "ubo_store.h"
#include "vkh.h"
#include "config.h"
#include <deque>
#include <glm/gtx/transform.hpp>
#include "shader_inputs.h"

namespace ubo_store
{
	uint32_t uboArrayLen;
	uint32_t countPerPage;
	uint32_t numPages;
	uint32_t size;

	vkh::VkhContext* ctxt;

	struct UBOPage
	{
		VkBuffer buf;
		vkh::Allocation alloc;
		std::deque<uint32_t> freeIndices;

#if DEVICE_LOCAL
		char* map;
#else
		void* map;
#endif

#if PERSISTENT_STAGING_BUFFER
		VkBuffer stagingBuf;
		vkh::Allocation stagingAlloc;
#endif
	};

	std::vector<UBOPage> pages;

	void init(vkh::VkhContext& _ctxt)
	{		
		ctxt = &_ctxt;
		countPerPage = 256;
		size = (sizeof(VShaderInput) * 256);
	}

	UBOPage& createNewPage()
	{
		UBOPage page;
		vkh::VkhContext& _ctxt = *ctxt;

		vkh::createBuffer(
			page.buf,
			page.alloc,
			size,
#if DEVICE_LOCAL
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
#else
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
#endif
			_ctxt);

#if PERSISTENT_STAGING_BUFFER
		vkh::createBuffer(
			page.stagingBuf,
			page.stagingAlloc,
			size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
			_ctxt
		);
#endif

#if DEVICE_LOCAL
		page.map = (char*)malloc(size);
#else
		vkMapMemory(_ctxt.device, page.alloc.handle, page.alloc.offset, page.alloc.size, 0, &page.map);
#endif

		for (uint32_t i = 0; i < countPerPage; ++i)
		{
			page.freeIndices.push_back(i);
		}

		pages.push_back(page);
		return pages[pages.size() - 1];
	}

	vkh::Allocation& getAlloc(uint32_t idx)
	{
		checkf(pages.size() >= (idx + 1), "Array index out of bounds");
		return pages[idx].alloc;
	}

	VkBuffer& getPage(uint32_t idx)
	{
		checkf(pages.size() >= (idx + 1), "Array index out of bounds");
		return pages[idx].buf;
	}

	bool acquire(uint32_t& outIdx)
	{
		UBOPage* p = nullptr;

		for (auto& page : pages)
		{
			if (page.freeIndices.size() > 0)
			{
				p = &page;
				break;
			}
		}

		if (!p)
		{
			p = &createNewPage();
		}
		
		uint32_t slot = p->freeIndices.front();
		p->freeIndices.pop_front();

		outIdx = slot << 3;
		outIdx += (slot / countPerPage);

		return true;

	}

	uint32_t getNumPages()
	{
		return pages.size();
	}

	void updateBuffers(const glm::mat4& viewMatrix, const glm::mat4& projMatrix, VkCommandBuffer* commandBuffer, vkh::VkhContext& ctxt)
	{
		std::vector<VkMappedMemoryRange> rangesToUpdate;
		rangesToUpdate.resize(pages.size());

		for (uint32_t p = 0; p < pages.size(); ++p)
		{
			UBOPage page = pages[p];
			VShaderInput* objPtr = (VShaderInput*)page.map;

			for (uint32_t i = 0; i < countPerPage; ++i)
			{
				objPtr[i].model = projMatrix * viewMatrix;

				//all objects use an identity model matrix
				objPtr[i].normal = glm::transpose(glm::inverse(viewMatrix));
			}

			VkMappedMemoryRange curRange = {};
			curRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			curRange.memory = page.alloc.handle;
			curRange.offset = page.alloc.offset;
			curRange.size = page.alloc.size;
			curRange.pNext = nullptr;

			rangesToUpdate[p] = curRange;
		}

		#if !DEVICE_LOCAL
			vkFlushMappedMemoryRanges(ctxt.device, rangesToUpdate.size(), rangesToUpdate.data());
		#endif

		#if DEVICE_LOCAL
			for (uint32_t p = 0; p < pages.size(); ++p)
			{
				#if PERSISTENT_STAGING_BUFFER	
					vkh::copyDataToBuffer(&pages[p].stagingBuf, size, 0, (char*)pages[p].map, commandBuffer, ctxt);
					vkh::copyBuffer(pages[p].stagingBuf, pages[p].buf, size, 0, 0, commandBuffer, ctxt);
				#else
					vkh::copyDataToBuffer(&pages[p].buf, size, 0, (char*)pages[p].map, commandBuffer, ctxt);
				#endif
			}
		#endif
	}

	VkDescriptorType getDescriptorType()
	{
		return DYNAMIC_UBO ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
}