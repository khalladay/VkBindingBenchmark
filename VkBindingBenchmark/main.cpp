#include "Common.h"
#include "mesh_loading.h"
#include "rendering.h"
#include "camera.h"
/*
	Single threaded. Try to keep as much equal as possible, save for the experimental changes
	Test - binding once with uniform buffers, binding once with ssbo, binding per object uniform data

	- Read in all textures, store in a global array of textures, give materials push constant indices
	- meshes should be read in, buffers stored in a global place, but not combined
	- materials created by hand (since for this demo, we'll likely only have the one material, with a lot of instances) 

*/

vkh::VkhContext appContext;
vkh::MeshAsset testMesh;
Camera::Cam worldCamera;

void mainLoop();

int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE pInstance, LPSTR cmdLine, int showCode)
{
	HWND wndHdl = OS::makeWindow(Instance, "Texture Array Demo", 800, 600);
	OS::initializeInput();

	vkh::VkhContextCreateInfo ctxtInfo = {};
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE);
	ctxtInfo.types.push_back(VK_DESCRIPTOR_TYPE_SAMPLER);

	ctxtInfo.typeCounts.push_back(128);
	ctxtInfo.typeCounts.push_back(64);
	ctxtInfo.typeCounts.push_back(64);

	initContext(ctxtInfo, "Uniform Buffer Array Demo", Instance, wndHdl, appContext);

	std::vector<vkh::EMeshVertexAttribute> meshLayout;
	meshLayout.push_back(vkh::EMeshVertexAttribute::POSITION);
	meshLayout.push_back(vkh::EMeshVertexAttribute::UV0);
	vkh::Mesh::setGlobalVertexLayout(meshLayout);

	initRendering(appContext);

	//load a test obj mesh
	loadMesh("..\\data\\mesh\\cube.obj", appContext, testMesh);
	//vkh::Mesh::quad(testMesh, appContext);
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
		Camera::rotate(worldCamera, Camera::localRight(worldCamera), OS::getMouseDY() * 0.01f);

		if (OS::getKey(KeyCode::KEY_W))
		{
			Camera::translate(worldCamera, Camera::localForward(worldCamera) * 0.01f);
		}

		if (OS::getKey(KEY_ESCAPE))
		{
			running = false;
			break;
		}
		
		render(worldCamera, &testMesh, 1);
	}
}