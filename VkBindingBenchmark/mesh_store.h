#pragma once
#include "vkh.h"

//idea here is to keep all VkBuffers (and related allocations) in a central place for easy copying and cleanup 
//since this application only contains static buffers, we don't have to worry about dealing with different mem types
//nor will we have to update meshes between frames, which means we can just allocate a huge chunk of memory once and
//parcel things out. 

//nowhere in this application do we every need a vertex buffer with an index buffer, and vice versa

VkBuffer* findFreeVertexBuffer();
VkBuffer* findFreeIndexBuffer();

const VkBuffer* getVertexBuffer(uint32_t id);
const VkBuffer* getIndexBuffer(uint32_t id);