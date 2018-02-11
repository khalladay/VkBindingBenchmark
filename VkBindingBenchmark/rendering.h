#pragma once
#include <stdint.h>
#include "vkh.h"

struct DrawCall
{
	uint16_t mesh;
	uint32_t material;
};

void initRendering(vkh::VkhContext& context);
void render(DrawCall* drawCalls, uint32_t count);