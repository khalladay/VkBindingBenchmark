#pragma once
#include <stdint.h>
#include "vkh.h"
#include "vkh_mesh.h"

void loadMesh(const char* filepath, vkh::VkhContext& ctxt, vkh::MeshAsset& outMesh);