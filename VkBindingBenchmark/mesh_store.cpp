#include "mesh_store.h"
#include <vector>
#include "vkh.h"

//the mesh store will use it's own pool allocator

std::vector<VkBuffer> buffers;
std::vector<uint32_t> freeIndices;