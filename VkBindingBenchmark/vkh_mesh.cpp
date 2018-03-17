#include "vkh_mesh.h"

namespace vkh::Mesh
{
	VertexRenderData* _vkRenderData;

	void setGlobalVertexLayout(std::vector<EMeshVertexAttribute> layout)
	{
		checkf(_vkRenderData == nullptr, "Attempting to set global vertex layout, but this has already been set");

		_vkRenderData = (VertexRenderData*)malloc(sizeof(VertexRenderData));

		_vkRenderData->attrCount = layout.size();
		_vkRenderData->attrDescriptions = (VkVertexInputAttributeDescription*)malloc(sizeof(VkVertexInputAttributeDescription) * _vkRenderData->attrCount);
		_vkRenderData->attributes = (EMeshVertexAttribute*)malloc(sizeof(EMeshVertexAttribute) * _vkRenderData->attrCount);
		memcpy(_vkRenderData->attributes, layout.data(), sizeof(EMeshVertexAttribute) * _vkRenderData->attrCount);

		uint32_t curOffset = 0;
		for (uint32_t i = 0; i < layout.size(); ++i)
		{
			switch (layout[i])
			{
			case EMeshVertexAttribute::POSITION:
			case EMeshVertexAttribute::NORMAL:
			case EMeshVertexAttribute::TANGENT:
			case EMeshVertexAttribute::BITANGENT:
			{
				_vkRenderData->attrDescriptions[i] = { i, 0, VK_FORMAT_R32G32B32_SFLOAT, curOffset };
				curOffset += sizeof(glm::vec3);
			}break;

			case EMeshVertexAttribute::UV0:
			case EMeshVertexAttribute::UV1:
			{
				_vkRenderData->attrDescriptions[i] = { i, 0, VK_FORMAT_R32G32_SFLOAT, curOffset };
				curOffset += sizeof(glm::vec2);
			}break;
			case EMeshVertexAttribute::COLOR:
			{
				_vkRenderData->attrDescriptions[i] = { i, 0, VK_FORMAT_R32G32B32A32_SFLOAT, curOffset };
				curOffset += sizeof(glm::vec4);
			}break;
			default: checkf(0, "Invalid vertex attribute specified"); break;
			}
		}

		_vkRenderData->vertexSize = curOffset;
	}

	const VertexRenderData* vertexRenderData()
	{
		checkf(_vkRenderData, "Attempting to get global vertex layout, but it has not been set yet, call setGlobalVertexLayout first.");

		return _vkRenderData;
	}

	uint32_t make(MeshAsset& outAsset, VkhContext& ctxt, float* vertices, uint32_t vertexCount, uint32_t* indices, uint32_t indexCount)
	{
		size_t vBufferSize = vertexRenderData()->vertexSize * vertexCount;
		size_t iBufferSize = sizeof(uint32_t) * indexCount;

		MeshAsset& m = outAsset;
		m.iCount = indexCount;
		m.vCount = vertexCount;

		createBuffer(m.buffer,
			m.bufferMemory,
			vBufferSize + iBufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			ctxt
		);

		//transfer data to the above buffers
		VkBuffer stagingBuffer;
		Allocation stagingMemory;

		createBuffer(stagingBuffer, 
			stagingMemory,
			vBufferSize + iBufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			ctxt
		);

		void* data;
		
		vkMapMemory(ctxt.device, stagingMemory.handle, stagingMemory.offset, vBufferSize, 0, &data);
		memcpy(data, vertices, (size_t)vBufferSize);
		vkUnmapMemory(ctxt.device, stagingMemory.handle);
		
		vkMapMemory(ctxt.device, stagingMemory.handle, stagingMemory.offset + vBufferSize, iBufferSize, 0, &data);
		memcpy(data, indices, (size_t)iBufferSize);
		vkUnmapMemory(ctxt.device, stagingMemory.handle);

		//copy to device local here
		copyBuffer(stagingBuffer, m.buffer, vBufferSize+iBufferSize, 0, 0, nullptr, ctxt);
		freeDeviceMemory(stagingMemory);
		vkDestroyBuffer(ctxt.device, stagingBuffer, nullptr);

		m.vOffset = 0;
		m.iOffset = vBufferSize;

		return 0;

	}

	void quad(MeshAsset& outAsset, VkhContext& ctxt, float width, float height, float xOffset, float yOffset)
	{
		const VertexRenderData* vertexData = vertexRenderData();

		std::vector<float> verts;

		float wComp = width / 2.0f;
		float hComp = height / 2.0f;

		const glm::vec3 lbCorner = glm::vec3(-wComp + xOffset, -hComp + yOffset, 0.0f);
		const glm::vec3 ltCorner = glm::vec3(lbCorner.x, hComp + yOffset, 0.0f);
		const glm::vec3 rbCorner = glm::vec3(wComp + xOffset, lbCorner.y, 0.0f);
		const glm::vec3 rtCorner = glm::vec3(rbCorner.x, ltCorner.y, 0.0f);

		const glm::vec3 pos[4] = { rtCorner, ltCorner, lbCorner, rbCorner };
		const glm::vec2 uv[4] = { glm::vec2(1.0f,1.0f), glm::vec2(0.0f,1.0f), glm::vec2(0.0f,0.0f), glm::vec2(1.0f,0.0f) };


		static uint32_t indices[6] = { 0,2,1,2,0,3 };
		uint32_t curIdx = 0;

		for (uint32_t i = 0; i < 4; ++i)
		{

			for (uint32_t j = 0; j < vertexData->attrCount; ++j)
			{
				EMeshVertexAttribute attrib = vertexData->attributes[j];

				switch (attrib)
				{
				case EMeshVertexAttribute::POSITION:
				{
					verts.push_back(pos[i].x);
					verts.push_back(pos[i].y);
					verts.push_back(pos[i].z);
				}break;
				case EMeshVertexAttribute::NORMAL:
				case EMeshVertexAttribute::TANGENT:
				case EMeshVertexAttribute::BITANGENT:
				{
					verts.push_back(0);
					verts.push_back(0);
					verts.push_back(0);
				}break;

				case EMeshVertexAttribute::UV0:
				{
					verts.push_back(uv[i].x);
					verts.push_back(uv[i].y);
				}break;
				case EMeshVertexAttribute::UV1:
				{
					verts.push_back(0);
					verts.push_back(0);
				}break;
				case EMeshVertexAttribute::COLOR:
				{
					verts.push_back(0);
					verts.push_back(0);
					verts.push_back(0);
					verts.push_back(0);

				}break;
				default: checkf(0, "Invalid vertex attribute specified"); break;

				}
			}
		}

		make(outAsset, ctxt, verts.data(), 4, &indices[0], 6);
	//	free(verts);
	}
}