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

void initRendering(vkh::VkhContext& context, uint32_t num);
void updateUBOs(Camera::Cam& cam);
void render(Camera::Cam& camera, const std::vector<vkh::MeshAsset>& drawCalls, const std::vector<uint32_t>& uboIdx);