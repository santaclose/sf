#ifdef SF_USE_VULKAN

#include "VulkanShaderBuffer.h"
#include "VulkanDisplay.h"

void sf::Renderer::VulkanShaderBuffer::Create(VulkanShaderBufferType type, bool inflightAware, size_t bufferSize)
{
	this->bufferSize = bufferSize;
	this->bufferType = type;
	this->inflightAware = inflightAware;
	size_t i = 0;
	do
	{
		VulkanUtils::CreateBuffer(bufferSize,
			type == VulkanShaderBufferType::StorageBuffer ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			buffers[i], buffersMemory[i]);
		vkMapMemory(VulkanDisplay::Instance->device.device, buffersMemory[i], 0, bufferSize, 0, &buffersMapped[i]);
		i++;
	} while (inflightAware && i < VulkanDisplay::MaxFramesInFlight());
}

void sf::Renderer::VulkanShaderBuffer::Destroy()
{
	size_t i = 0;
	do
	{
		vkDestroyBuffer(VulkanDisplay::Instance->disp.device, buffers[i], nullptr);
		vkFreeMemory(VulkanDisplay::Instance->disp.device, buffersMemory[i], nullptr);
		i++;
	} while (inflightAware && i < VulkanDisplay::MaxFramesInFlight());
}

#endif