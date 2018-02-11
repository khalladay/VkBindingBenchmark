#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vk_sdk_platform.h>
#include <vector>
#include "debug.h"

#include "vkh_types.h"

namespace vkh
{
	void createDescriptorPool(VkDescriptorPool& outPool, const VkDevice& device, std::vector<VkDescriptorType>& descriptorTypes, std::vector<uint32_t>& maxDescriptors);
	void createImageView(VkImageView& outView, VkFormat imageFormat, VkImageAspectFlags aspectMask, uint32_t mipCount, const VkImage& imageHdl, const VkDevice& device);
	void createVkSemaphore(VkSemaphore& outSemaphore, const VkDevice& device);
	void createFence(VkFence& outFence, VkDevice& device);
	void waitForFence(VkFence& fence, const VkDevice& device);
	void createCommandPool(VkCommandPool& outPool, const VkDevice& lDevice, const VkhPhysicalDevice& physDevice, uint32_t queueFamilyIdx);
	void freeDeviceMemory(Allocation& mem);
	void createRenderPass(VkRenderPass& outPass, std::vector<VkAttachmentDescription>& colorAttachments, VkAttachmentDescription* depthAttachment, const VkDevice& device);
	void createCommandBuffer(VkCommandBuffer& outBuffer, VkCommandPool& pool, const VkDevice& lDevice);
	uint32_t getMemoryType(const VkPhysicalDevice& device, uint32_t memoryTypeBitsRequirement, VkMemoryPropertyFlags requiredProperties);
	void createFrameBuffers(std::vector<VkFramebuffer>& outBuffers, const VkhSwapChain& swapChain, const VkImageView* depthBufferView, const VkRenderPass& renderPass, const VkDevice& device);
	void allocateDeviceMemory(Allocation& outMem, AllocationCreateInfo info, VkhContext& ctxt);
	VkhCommandBuffer beginScratchCommandBuffer(ECommandPoolType type, VkhContext& ctxt);
	void submitScratchCommandBuffer(VkhCommandBuffer& commandBuffer);
	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, uint32_t srcOffset, uint32_t dstOffset, VkhCommandBuffer& buffer);
	void copyBuffer(VkBuffer& srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, uint32_t srcOffset, uint32_t dstOffset, VkhContext& ctxt);
	void createShaderModule(VkShaderModule& outModule, const char* binaryData, size_t dataSize, const VkhContext& ctxt);
	void createBuffer(VkBuffer& outBuffer, Allocation& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkhContext& ctxt);
	void copyDataToBuffer(VkBuffer* buffer, uint32_t dataSize, uint32_t dstOffset, char* data, VkhContext& ctxt);
	void createImage(VkImage& outImage, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, const VkhContext& ctxt);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, VkhContext& ctxt);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkhContext& ctxt);

	void allocMemoryForImage(Allocation& outMem, const VkImage& image, VkMemoryPropertyFlags properties, VkhContext& ctxt);
	void createBuffer(VkBuffer& outBuffer, Allocation& bufferMemory, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, const VkhContext& ctxt);
}