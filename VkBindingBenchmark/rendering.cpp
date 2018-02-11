#include "rendering.h"
#include "os_init.h"

struct RenderingData
{
	vkh::VkhContext*				owningContext;
	std::vector<VkFramebuffer>		frameBuffers;
	std::vector<VkCommandBuffer>	commandBuffers;
	vkh::VkhRenderBuffer			depthBuffer;
	VkRenderPass					mainRenderPass;
};

RenderingData appData;

void createMainRenderPass(vkh::VkhContext& ctxt);
void createDepthBuffer();

void initRendering(vkh::VkhContext& context)
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

}

void createDepthBuffer()
{
	vkh::VkhContext& ctxt = *appData.owningContext;

	vkh::createImage(appData.depthBuffer.handle, 
					OS::GAppInfo.curW, 
					OS::GAppInfo.curH, 
					VK_FORMAT_D32_SFLOAT, 
					VK_IMAGE_TILING_OPTIMAL, 
					VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
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


void render(DrawCall* drawCalls, uint32_t count)
{
	//acquire an image from the swap chain
	uint32_t imageIndex;

	vkh::VkhContext& appContext = *appData.owningContext;

	//using uint64 max for timeout disables it
	VkResult res = vkAcquireNextImageKHR(appContext.device, appContext.swapChain.swapChain, UINT64_MAX, appContext.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	vkh::waitForFence(appContext.frameFences[imageIndex], appContext.device);
	vkResetFences(appContext.device, 1, &appContext.frameFences[imageIndex]);

	//record drawing
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional
	vkResetCommandBuffer(appData.commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	res = vkBeginCommandBuffer(appData.commandBuffers[imageIndex], &beginInfo);


	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = appData.mainRenderPass;
	renderPassInfo.framebuffer = appData.frameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = appContext.swapChain.extent;

	std::vector<VkClearValue> clearColors;

	//color
	clearColors.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });

	//depth
	clearColors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearColors.size());
	renderPassInfo.pClearValues = &clearColors[0];

	vkCmdBeginRenderPass(appData.commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);


	for (uint32_t i = 0; i < count; ++i)
	{
		DrawCall& dc = drawCalls[i];


	}

	vkCmdEndRenderPass(appData.commandBuffers[imageIndex]);
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

}