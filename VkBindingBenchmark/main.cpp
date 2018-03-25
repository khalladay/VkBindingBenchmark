#include "Common.h"
#include "mesh_loading.h"
#include "rendering.h"
#include "camera.h"
#include "vkh.h"
#include "config.h"

/*
	Single threaded. Try to keep as much equal as possible, save for the experimental changes
	Test - binding once with uniform buffers, binding once with ssbo, binding per object uniform data

	- Read in all textures, store in a global array of textures, give materials push constant indices
	- meshes should be read in, buffers stored in a global place, but not combined
	- materials created by hand (since for this demo, we'll likely only have the one material, with a lot of instances) 

*/

vkh::VkhContext appContext;

std::vector<vkh::MeshAsset> testMesh;
std::vector<uint32_t> uboIdx;

Camera::Cam worldCamera;

void mainLoop();

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE pInstance, LPSTR cmdLine, int showCode)
{
	HWND wndHdl = OS::makeWindow(Instance, "Texture Array Demo", SCREEN_W, SCREEN_H);
	OS::initializeInput();

	vkh::VkhContextCreateInfo ctxtInfo = {};
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_SAMPLER);

	ctxtInfo.typeCounts.push_back(512);
	ctxtInfo.typeCounts.push_back(512);
	ctxtInfo.typeCounts.push_back(512);
	ctxtInfo.typeCounts.push_back(512);
	ctxtInfo.typeCounts.push_back(512);
	ctxtInfo.typeCounts.push_back(1);

	initContext(ctxtInfo, "Uniform Buffer Array Demo", Instance, wndHdl, appContext);

	std::vector<vkh::EMeshVertexAttribute> meshLayout;
	meshLayout.push_back(vkh::EMeshVertexAttribute::POSITION);
	meshLayout.push_back(vkh::EMeshVertexAttribute::UV0);
	meshLayout.push_back(vkh::EMeshVertexAttribute::NORMAL);
	vkh::Mesh::setGlobalVertexLayout(meshLayout);

	//load a test obj mesh
#if BISTRO_TEST
	testMesh = loadMesh("..\\data\\mesh\\exterior.obj", false, appContext);
	auto interior = loadMesh("..\\data\\mesh\\interior.obj", false, appContext);
	testMesh.insert(testMesh.end(), interior.begin(), interior.end());
#else
	testMesh = loadMesh("..\\data\\mesh\\sponza.obj", false, appContext);
#endif

	uboIdx.resize(testMesh.size());

	printf("Num meshes: %d\n", testMesh.size());

	data_store::init(appContext);
	
	for (uint32_t i = 0; i < testMesh.size(); ++i)
	{
		bool didAcquire = data_store::acquire(uboIdx[i]);
		checkf(didAcquire, "Error acquiring ubo index");
	}

#if SHUFFLE_MESHES
	srand(8675309);

	for (uint32_t i = 0; i < testMesh.size(); ++i)
	{
		uint32_t newSlot = rand() % (testMesh.size());

		std::swap(testMesh[i], testMesh[newSlot]);
		std::swap(uboIdx[i], uboIdx[newSlot]);
		printf("%i\n", uboIdx[i] >> 3);

	}

#endif

	initRendering(appContext, testMesh.size());

	mainLoop();

	return 0;
}

void logFPSAverage(double avg)
{
	printf("AVG FRAMETIME FOR LAST %i FRAMES: %f ms\n", FPS_DATA_FRAME_HISTORY_SIZE, avg);
}

void mainLoop()
{
	bool running = true;

	FPSData fpsData = { 0 };

	fpsData.logCallback = logFPSAverage;

	startTimingFrame(fpsData);

	Camera::init(worldCamera);

	while (running)
	{
		double dt = endTimingFrame(fpsData);
		startTimingFrame(fpsData);

		OS::handleEvents();
		OS::pollInput();

		Camera::rotate(worldCamera, glm::vec3(0.0f, 1.0f, 0.0f), -OS::getMouseDX() * 0.01f);
		Camera::rotate(worldCamera, Camera::localRight(worldCamera), -OS::getMouseDY() * 0.01f);

		float leftRight = OS::getKey(KeyCode::KEY_A) ? 1.0f : (OS::getKey(KeyCode::KEY_D) ? -1.0f : 0.0f);
		float forwardBack = OS::getKey(KeyCode::KEY_W) ? 1.0f : (OS::getKey(KeyCode::KEY_S) ? -1.0f : 0.0f);

		glm::vec3 translation = (Camera::localForward(worldCamera) * forwardBack) + (Camera::localRight(worldCamera) * leftRight);
		Camera::translate(worldCamera, translation * 2.0f * (float)dt);

		if (OS::getKey(KEY_ESCAPE))
		{
			running = false;
			break;
		}
		
		render(worldCamera, testMesh,uboIdx);
	}
}