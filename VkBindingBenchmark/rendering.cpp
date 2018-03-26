#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "config.h"
#include "rendering.h"
#include "os_init.h"
#include "vkh_material.h"
#include "config.h"
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include "shader_inputs.h"

struct RenderingData
{
	vkh::VkhContext*				owningContext;
	std::vector<VkFramebuffer>		frameBuffers;
	std::vector<VkCommandBuffer>	commandBuffers;
	vkh::VkhRenderBuffer			depthBuffer;
	VkRenderPass					mainRenderPass;

	VkBuffer						ubo;
	vkh::Allocation					uboAlloc;
};

RenderingData appData;

struct MatData
{
	std::vector<VkDescriptorSet>	descSets;
	VkDescriptorSetLayout			descSetLayout;
	VkPipelineLayout				pipelineLayout;
	VkPipeline						graphicsPipeline;
};

MatData appMaterial;

void createMainRenderPass(vkh::VkhContext& ctxt);
void createDepthBuffer();
void loadDebugMaterial();
void loadUBOTestMaterial(int num);
void createGlobalShaderData();
int bindDescriptorSets(int curPage, int pageToBind, int slotToBind, VkCommandBuffer& cmd);

void initRendering(vkh::VkhContext& context, uint32_t num)
{
	appData.owningContext = &context;

	createMainRenderPass(context);
	createDepthBuffer();

	vkh::createFrameBuffers(appData.frameBuffers, context.swapChain, &appData.depthBuffer.view, appData.mainRenderPass, context.device);

	uint32_t swapChainImageCount = static_cast<uint32_t>(context.swapChain.imageViews.size());
	appData.commandBuffers.resize(swapChainImageCount);
	for (uint32_t i = 0; i < swapChainImageCount; ++i)
	{
		vkh::createCommandBuffer(appData.commandBuffers[i], context.gfxCommandPool, context.device);
	}

#if PUSH_TEST
	loadDebugMaterial();
#else
	loadUBOTestMaterial(num);
#endif
}

void createGlobalShaderData()
{

}

VkDescriptorBufferInfo bufferInfo;
static VkWriteDescriptorSet setWrite;

void loadUBOTestMaterial(int num)
{
	//create descriptor set layout
	VkDescriptorSetLayoutBinding layoutBinding;
	layoutBinding = vkh::descriptorSetLayoutBinding(data_store::getDescriptorType(), VK_SHADER_STAGE_VERTEX_BIT, 0, 1);

	VkDescriptorSetLayoutCreateInfo layoutInfo = vkh::descriptorSetLayoutCreateInfo(&layoutBinding, 1);

	VkResult res = vkCreateDescriptorSetLayout(appData.owningContext->device, &layoutInfo, nullptr, &appMaterial.descSetLayout);
	checkf(res == VK_SUCCESS, "Error creating desc set layout");


	vkh::VkhMaterialCreateInfo createInfo = {};
	createInfo.renderPass = appData.mainRenderPass;
	createInfo.outPipeline = &appMaterial.graphicsPipeline;
	createInfo.outPipelineLayout = &appMaterial.pipelineLayout;

	createInfo.pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;
	createInfo.pushConstantRange = sizeof(uint32_t);
	createInfo.descSetLayouts.push_back(appMaterial.descSetLayout);

#if WITH_COMPLEX_SHADER
	vkh::createBasicMaterial(VERT_SHADER_NAME, "..\\data\\_generated\\builtshaders\\random_frag.frag.spv", *appData.owningContext, createInfo);
#else
	vkh::createBasicMaterial(VERT_SHADER_NAME, "..\\data\\_generated\\builtshaders\\debug_normals.frag.spv", *appData.owningContext, createInfo);
#endif

	//allocate a descriptor set for each ubo transform array
	appMaterial.descSets.resize(data_store::getNumPages());

	for (uint32_t i = 0; i < data_store::getNumPages(); ++i)
	{
		VkDescriptorSetAllocateInfo allocInfo = vkh::descriptorSetAllocateInfo(&appMaterial.descSetLayout, 1, appData.owningContext->descriptorPool);
		res = vkAllocateDescriptorSets(appData.owningContext->device, &allocInfo, &appMaterial.descSets[i]);
		checkf(res == VK_SUCCESS, "Error allocating global descriptor set");

		bufferInfo = {};
		bufferInfo.buffer = data_store::getPage(i);
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.dstBinding = 0;
		setWrite.dstArrayElement = 0;
		setWrite.descriptorType = data_store::getDescriptorType();
		setWrite.descriptorCount = 1;
		setWrite.dstSet = appMaterial.descSets[i];
		setWrite.pBufferInfo = &bufferInfo;
		setWrite.pImageInfo = 0;

		vkUpdateDescriptorSets(appData.owningContext->device, 1, &setWrite, 0, nullptr);
	}
}

void loadDebugMaterial()
{
	vkh::VkhMaterialCreateInfo createInfo = {};
	createInfo.outPipeline = &appMaterial.graphicsPipeline;
	createInfo.outPipelineLayout = &appMaterial.pipelineLayout;
	createInfo.renderPass = appData.mainRenderPass;
	createInfo.pushConstantStages = VK_SHADER_STAGE_VERTEX_BIT;
	createInfo.pushConstantRange = sizeof(glm::mat4)*2;

#if WITH_COMPLEX_SHADER
	vkh::createBasicMaterial("..\\data\\_generated\\builtshaders\\common_vert.vert.spv", "..\\data\\_generated\\builtshaders\\random_frag.frag.spv", *appData.owningContext, createInfo);
#else
	vkh::createBasicMaterial("..\\data\\_generated\\builtshaders\\common_vert.vert.spv", "..\\data\\_generated\\builtshaders\\debug_normals.frag.spv", *appData.owningContext, createInfo);
#endif
}


void createDepthBuffer()
{
	vkh::VkhContext& ctxt = *appData.owningContext;

	vkh::createImage(appData.depthBuffer.handle, 
					ctxt.swapChain.extent.width, 
					ctxt.swapChain.extent.height, 
					VK_FORMAT_D32_SFLOAT, 
					VK_IMAGE_TILING_OPTIMAL, 
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					*appData.owningContext);

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(ctxt.device, appData.depthBuffer.handle, &memRequirements);

	vkh::AllocationCreateInfo createInfo;
	createInfo.size = memRequirements.size;
	createInfo.memoryTypeIndex = vkh::getMemoryType(ctxt.gpu.device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	createInfo.usage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	vkh::allocateDeviceMemory(appData.depthBuffer.imageMemory, createInfo, ctxt);
	vkBindImageMemory(ctxt.device, appData.depthBuffer.handle, appData.depthBuffer.imageMemory.handle, appData.depthBuffer.imageMemory.offset);
	vkh::createImageView(appData.depthBuffer.view, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, 1, appData.depthBuffer.handle, ctxt.device);

	vkh::transitionImageLayout(appData.depthBuffer.handle, VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, ctxt);
}

void createMainRenderPass(vkh::VkhContext& ctxt)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = ctxt.swapChain.imageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


	std::vector<VkAttachmentDescription> renderPassAttachments;
	renderPassAttachments.push_back(colorAttachment);

	vkh::createRenderPass(appData.mainRenderPass, renderPassAttachments, &depthAttachment, ctxt.device);

}


void render(Camera::Cam& cam, const std::vector<vkh::MeshAsset>& drawCalls, const std::vector<uint32_t>& uboIdx)
{
	const glm::mat4 view = Camera::viewMatrix(cam);
	glm::mat4 p = glm::perspectiveRH(glm::radians(60.0f), SCREEN_W / (float)SCREEN_H, 0.05f, 3000.0f);
	//from https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
	//this flips the y coordinate back to positive == up, and readjusts depth range to match opengl
	glm::mat4 vulkanCorrection = glm::mat4(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.5f, 0.0f,
		0.0f, 0.0f, 0.5f, 1.0f);

	static float rads = 0.001f;
	glm::mat4 proj = vulkanCorrection * p;	
	vkh::VkhContext& appContext = *appData.owningContext;

#if !COPY_ON_MAIN_COMMANDBUFFER
	data_store::updateBuffers(view, proj, nullptr, appContext);
#endif

	VkResult res;	

	//acquire an image from the swap chain
	uint32_t imageIndex;

	res = vkAcquireNextImageKHR(appContext.device, appContext.swapChain.swapChain, UINT64_MAX, appContext.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	vkh::waitForFence(appContext.frameFences[imageIndex], appContext.device);
	vkResetFences(appContext.device, 1, &appContext.frameFences[imageIndex]);

	//record drawing
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional
	vkResetCommandBuffer(appData.commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	res = vkBeginCommandBuffer(appData.commandBuffers[imageIndex], &beginInfo);

#if COPY_ON_MAIN_COMMANDBUFFER
	data_store::updateBuffers(view, proj, &appData.commandBuffers[imageIndex], appContext);
#endif


	vkCmdResetQueryPool(appData.commandBuffers[imageIndex], appContext.queryPool, 0, 10);
	vkCmdWriteTimestamp(appData.commandBuffers[imageIndex], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, appContext.queryPool, 0);


	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = appData.mainRenderPass;
	renderPassInfo.framebuffer = appData.frameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = appContext.swapChain.extent;

	VkClearValue clearColors[2];
	clearColors[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearColors[1].depthStencil.depth = 1.0f;
	clearColors[1].depthStencil.stencil = 0;

	renderPassInfo.clearValueCount = static_cast<uint32_t>(2);
	renderPassInfo.pClearValues = &clearColors[0];

	vkCmdBeginRenderPass(appData.commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	int currentlyBound = -1;

	vkCmdBindPipeline(appData.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, appMaterial.graphicsPipeline);

	for (uint32_t i = 0; i < drawCalls.size(); ++i)
	{
#if UBO_TEST || SSBO_TEST
		glm::uint32 uboSlot = uboIdx[i] >> 3;
		glm::uint32 uboPage = uboIdx[i] & 0x7;

		currentlyBound = bindDescriptorSets(currentlyBound, uboPage, uboSlot, appData.commandBuffers[imageIndex]);

		vkCmdPushConstants(
			appData.commandBuffers[imageIndex],
			appMaterial.pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(glm::uint32),
			(void*)&uboSlot);

#elif PUSH_TEST
		
		//0 is MVP, 1 is normal
		glm::mat4 frameData[2];

		frameData[0] = vulkanCorrection * p * view;
		frameData[1] = glm::transpose(glm::inverse(view));

		vkCmdPushConstants(
			appData.commandBuffers[imageIndex],
			appMaterial.pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT,
			0,
			sizeof(glm::mat4)*2,
			(void*)&frameData);
#endif

		VkBuffer vertexBuffers[] = { drawCalls[i].buffer };
		VkDeviceSize vertexOffsets[] = { 0 };
		vkCmdBindVertexBuffers(appData.commandBuffers[imageIndex], 0, 1, vertexBuffers, vertexOffsets);
		vkCmdBindIndexBuffer(appData.commandBuffers[imageIndex], drawCalls[i].buffer, drawCalls[i].iOffset, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(appData.commandBuffers[imageIndex], static_cast<uint32_t>(drawCalls[i].iCount), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(appData.commandBuffers[imageIndex]);
	vkCmdWriteTimestamp(appData.commandBuffers[imageIndex], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, appContext.queryPool, 1);

	res = vkEndCommandBuffer(appData.commandBuffers[imageIndex]);
	checkf(res == VK_SUCCESS, "Error ending render pass");

	// submit

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	//wait on writing colours to the buffer until the semaphore says the buffer is available
	VkSemaphore waitSemaphores[] = { appContext.imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;

	VkSemaphore signalSemaphores[] = { appContext.renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	submitInfo.pCommandBuffers = &appData.commandBuffers[imageIndex];
	submitInfo.commandBufferCount = 1;

	res = vkQueueSubmit(appContext.deviceQueues.graphicsQueue, 1, &submitInfo, appContext.frameFences[imageIndex]);
	checkf(res == VK_SUCCESS, "Error submitting queue");

	//present

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { appContext.swapChain.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	res = vkQueuePresentKHR(appContext.deviceQueues.transferQueue, &presentInfo);

#if WITH_VK_TIMESTAMP
	//log performance data:

	uint32_t end = 0;
	uint32_t begin = 0;

	static int count = 0;
	static float totalTime = 0.0f;

	if (count++ > 1024)
	{
		printf("VK Render Time (avg of past 1024 frames): %f ms\n", totalTime / 1024.0f);
		count = 0;
		totalTime = 0;
	}
	float timestampFrequency = appContext.gpu.deviceProps.limits.timestampPeriod;


	vkGetQueryPoolResults(appContext.device, appContext.queryPool, 1, 1, sizeof(uint32_t), &end, 0, VK_QUERY_RESULT_WAIT_BIT);
	vkGetQueryPoolResults(appContext.device, appContext.queryPool, 0, 1, sizeof(uint32_t), &begin, 0, VK_QUERY_RESULT_WAIT_BIT);
	uint32_t diff = end - begin;
	totalTime += (diff) / (float)1e6;
#endif

}

int bindDescriptorSets(int currentlyBound, int page, int slot, VkCommandBuffer& cmd)
{
	vkh::VkhContext& appContext = *appData.owningContext;

	size_t uboAlignment = appContext.gpu.deviceProps.limits.minUniformBufferOffsetAlignment;
	size_t dynamicAlignment = (sizeof(VShaderInput) / uboAlignment + ((sizeof(VShaderInput) % uboAlignment) > 0 ? uboAlignment : 0));

	uint32_t offsetCount = DYNAMIC_UBO;
	uint32_t offset = offsetCount > 0 ? slot * dynamicAlignment :0;

	if (currentlyBound != page)
	{
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, appMaterial.pipelineLayout, 0, 1, &appMaterial.descSets[page], offsetCount, &offset);
	}
	return page;
}