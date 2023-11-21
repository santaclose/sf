#include "VulkanUniformBuffer.h"
#include "VulkanDisplay.h"

void sf::Renderer::VulkanUniformBuffer::Create(size_t bufferSize)
{
	for (size_t i = 0; i < VulkanDisplay::MaxFramesInFlight(); i++)
	{
		VulkanUtils::CreateBuffer(*VulkanDisplay::Instance, bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], buffersMemory[i]);
		vkMapMemory(VulkanDisplay::Instance->device.device, buffersMemory[i], 0, bufferSize, 0, &buffersMapped[i]);
	}
}

void sf::Renderer::VulkanUniformBuffer::Destroy()
{
	for (size_t i = 0; i < VulkanDisplay::MaxFramesInFlight(); i++)
	{
		vkDestroyBuffer(VulkanDisplay::Instance->disp.device, buffers[i], nullptr);
		vkFreeMemory(VulkanDisplay::Instance->disp.device, buffersMemory[i], nullptr);
	}
}
