#pragma once
#include <stdint.h>
#include "vkh.h"
#include "vkh_mesh.h"
#include "camera.h"

struct DrawCall
{
	uint16_t mesh;
	uint32_t material;
};

void initRendering(vkh::VkhContext& context);
void render(Camera::Cam& camera, vkh::MeshAsset* drawCalls, uint32_t count);