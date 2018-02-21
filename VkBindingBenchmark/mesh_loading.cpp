#include "mesh_loading.h"
#include "vkh_mesh.h"
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

static const int defaultFlags =  aiProcess_JoinIdenticalVertices| aiProcess_PreTransformVertices | aiProcess_FlipWindingOrder | aiProcess_Triangulate | aiProcess_CalcTangentSpace;

void loadMesh(const char* filepath, vkh::VkhContext& ctxt, vkh::MeshAsset& outMesh)
{
	using namespace vkh;

	const VertexRenderData* globalVertLayout = Mesh::vertexRenderData();

	Assimp::Importer aiImporter;
	const struct aiScene* scene = NULL;

	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&stream);

	scene = aiImporter.ReadFile(filepath, defaultFlags);

	const aiVector3D ZeroVector(0.0, 0.0, 0.0);
	const aiColor4D ZeroColor(0.0, 0.0, 0.0, 0.0);

	if (scene)
	{
		uint32_t floatsPerVert = globalVertLayout->vertexSize / sizeof(float);
		std::vector<float> vertexBuffer;
		std::vector<uint32_t> indexBuffer;
		uint32_t numVerts = 0;
		uint32_t numFaces = 0;

		for (uint32_t mIdx = 0; mIdx < scene->mNumMeshes; mIdx++)
		{
			const aiMesh* mesh = scene->mMeshes[mIdx];
			for (uint32_t vIdx = 0; vIdx < mesh->mNumVertices; ++vIdx)
			{
				const aiVector3D* pos = &(mesh->mVertices[vIdx]);
				const aiVector3D* nrm = &(mesh->mNormals[vIdx]);
				const aiVector3D* uv0 = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][vIdx]) : &ZeroVector;
				const aiVector3D* uv1 = mesh->HasTextureCoords(1) ? &(mesh->mTextureCoords[1][vIdx]) : &ZeroVector;
				const aiVector3D* tan = mesh->HasTangentsAndBitangents() ? &(mesh->mTangents[vIdx]) : &ZeroVector;
				const aiVector3D* biTan = mesh->HasTangentsAndBitangents() ? &(mesh->mBitangents[vIdx]) : &ZeroVector;
				const aiColor4D* col = mesh->HasVertexColors(0) ? &(mesh->mColors[0][vIdx]) : &ZeroColor;

				for (uint32_t lIdx = 0; lIdx < globalVertLayout->attrCount; ++lIdx)
				{
					EMeshVertexAttribute comp = globalVertLayout->attributes[lIdx];

					switch (comp)
					{
						case EMeshVertexAttribute::POSITION:
						{
							vertexBuffer.push_back(pos->x);
							vertexBuffer.push_back(pos->y);
							vertexBuffer.push_back(pos->z);
						}; break;
						case EMeshVertexAttribute::NORMAL:
						{
							vertexBuffer.push_back(nrm->x);
							vertexBuffer.push_back(nrm->y);
							vertexBuffer.push_back(nrm->z);
						}; break;
						case EMeshVertexAttribute::UV0:
						{
							vertexBuffer.push_back(uv0->x);
							vertexBuffer.push_back(uv0->y);
						}; break;
						case EMeshVertexAttribute::UV1:
						{
							vertexBuffer.push_back(uv1->x);
							vertexBuffer.push_back(uv1->y);
						}; break;
						case EMeshVertexAttribute::TANGENT:
						{
							vertexBuffer.push_back(tan->x);
							vertexBuffer.push_back(tan->y);
							vertexBuffer.push_back(tan->z);
						}; break;
						case EMeshVertexAttribute::BITANGENT:
						{
							vertexBuffer.push_back(biTan->x);
							vertexBuffer.push_back(biTan->y);
							vertexBuffer.push_back(biTan->z);
						}; break;

						case EMeshVertexAttribute::COLOR:
						{
							vertexBuffer.push_back(col->r);
							vertexBuffer.push_back(col->g);
							vertexBuffer.push_back(col->b);
							vertexBuffer.push_back(col->a);
						}; break;
					}

				}
			}

			for (unsigned int fIdx = 0; fIdx < mesh->mNumFaces; fIdx++)
			{
				const aiFace& face = mesh->mFaces[fIdx];
				checkf(face.mNumIndices == 3, "unsupported number of indices in mesh face");

				indexBuffer.push_back(numVerts + face.mIndices[0]);
				indexBuffer.push_back(numVerts + face.mIndices[1]);
				indexBuffer.push_back(numVerts + face.mIndices[2]);
			}

			numVerts += mesh->mNumVertices;
			numFaces += mesh->mNumFaces;

		//	checkf(vertexBuffer.size() / floatsPerVert == mesh->mNumVertices, "Incorrect number of verts created for mesh");

		}
		vkh::Mesh::make(outMesh, ctxt, vertexBuffer.data(), numVerts, indexBuffer.data(), indexBuffer.size());

	}

	aiDetachAllLogStreams();
}