#pragma once
#include <stdint.h>
#include "vkh.h"
#include "vkh_mesh.h"

struct DrawCall
{
	uint16_t mesh;
	uint32_t material;
};

void initRendering(vkh::VkhContext& context);
void render(vkh::MeshAsset* drawCalls, uint32_t count);