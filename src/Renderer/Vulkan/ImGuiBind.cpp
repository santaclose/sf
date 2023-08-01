//#ifdef SF_USE_VULKAN
//
//#include "../ImGuiBind.h"
//
//#include <iostream>
//#include <backends/imgui_impl_glfw.h>
//#include <backends/imgui_impl_vulkan.h>
//#include <vulkan/vulkan.h>
//#include <glm/glm.hpp>
//
//#include "VulkanState.h"
//#include "../Renderer.h"
//
//#include <Config.h>
//
//namespace sf::Renderer {
//
//	VulkanState vulkanState;
//
//	void check_vk_result(VkResult err)
//	{
//		if (err == 0)
//			return;
//		std::cout << "[vulkan] Error: VkResult = " << err << std::endl;
//		if (err < 0)
//			abort();
//	}
//
//	void SetupVulkanWindow(VkSurfaceKHR surface, int width, int height)
//	{
//		vulkanState.mainWindowData.Surface = surface;
//		// Check for WSI support
//		VkBool32 res;
//		vkGetPhysicalDeviceSurfaceSupportKHR(vulkanState.physicalDevice, vulkanState.queueFamily, vulkanState.mainWindowData.Surface, &res);
//		if (res != VK_TRUE)
//		{
//			fprintf(stderr, "Error no WSI support on physical device 0\n");
//			exit(-1);
//		}
//		// Select Surface Format
//		const VkFormat requestSurfaceImageFormat[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_UNORM };
//		const VkColorSpaceKHR requestSurfaceColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
//		vulkanState.mainWindowData.SurfaceFormat = ImGui_ImplVulkanH_SelectSurfaceFormat(vulkanState.physicalDevice, vulkanState.mainWindowData.Surface, requestSurfaceImageFormat, (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat), requestSurfaceColorSpace);
//		// Select Present Mode
//#ifdef IMGUI_UNLIMITED_FRAME_RATE
//		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR, VK_PRESENT_MODE_FIFO_KHR };
//#else
//		VkPresentModeKHR present_modes[] = { VK_PRESENT_MODE_FIFO_KHR };
//#endif
//		vulkanState.mainWindowData.PresentMode = ImGui_ImplVulkanH_SelectPresentMode(vulkanState.physicalDevice, vulkanState.mainWindowData.Surface, &present_modes[0], IM_ARRAYSIZE(present_modes));
//		//printf("[vulkan] Selected PresentMode = %d\n", vulkanState.mainWindowData.PresentMode);
//		// Create SwapChain, RenderPass, Framebuffer, etc.
//		IM_ASSERT(vulkanState.minImageCount >= 2);
//		ImGui_ImplVulkanH_CreateOrResizeWindow(vulkanState.instance, vulkanState.physicalDevice, vulkanState.device, &vulkanState.mainWindowData, vulkanState.queueFamily, vulkanState.allocator, width, height, vulkanState.minImageCount);
//	}
//
//	void FramePresent()
//	{
//		if (vulkanState.swapChainRebuild)
//			return;
//		VkSemaphore render_complete_semaphore = vulkanState.mainWindowData.FrameSemaphores[vulkanState.mainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
//		VkPresentInfoKHR info = {};
//		info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//		info.waitSemaphoreCount = 1;
//		info.pWaitSemaphores = &render_complete_semaphore;
//		info.swapchainCount = 1;
//		info.pSwapchains = &vulkanState.mainWindowData.Swapchain;
//		info.pImageIndices = &vulkanState.mainWindowData.FrameIndex;
//		VkResult err = vkQueuePresentKHR(vulkanState.queue, &info);
//		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
//		{
//			vulkanState.swapChainRebuild = true;
//			return;
//		}
//		check_vk_result(err);
//		vulkanState.mainWindowData.SemaphoreIndex = (vulkanState.mainWindowData.SemaphoreIndex + 1) % vulkanState.mainWindowData.ImageCount; // Now we can use the next set of semaphores
//	}
//}
//
//void sf::Renderer::ImGuiBind::Initialize(GLFWwindow* window)
//{
//	uint32_t extensions_count = 0;
//	const char** extensions = glfwGetRequiredInstanceExtensions(&extensions_count);
//
//	VkResult err;
//	// Create Vulkan Instance
//	{
//		VkInstanceCreateInfo create_info = {};
//		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//		create_info.enabledExtensionCount = extensions_count;
//		create_info.ppEnabledExtensionNames = extensions;
//#ifdef IMGUI_VULKAN_DEBUG_REPORT
//		// Enabling validation layers
//		const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
//		create_info.enabledLayerCount = 1;
//		create_info.ppEnabledLayerNames = layers;
//		// Enable debug report extension (we need additional storage, so we duplicate the user array to add our new extension to it)
//		const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensions_count + 1));
//		memcpy(extensions_ext, extensions, extensions_count * sizeof(const char*));
//		extensions_ext[extensions_count] = "VK_EXT_debug_report";
//		create_info.enabledExtensionCount = extensions_count + 1;
//		create_info.ppEnabledExtensionNames = extensions_ext;
//		// Create Vulkan Instance
//		err = vkCreateInstance(&create_info, vulkanState.allocator, &vulkanState.instance);
//		check_vk_result(err);
//		free(extensions_ext);
//		// Get the function pointer (required for any extensions)
//		auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanState.instance, "vkCreateDebugReportCallbackEXT");
//		IM_ASSERT(vkCreateDebugReportCallbackEXT != NULL);
//		// Setup the debug report callback
//		VkDebugReportCallbackCreateInfoEXT debug_report_ci = {};
//		debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
//		debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
//		debug_report_ci.pfnCallback = debug_report;
//		debug_report_ci.pUserData = NULL;
//		err = vkCreateDebugReportCallbackEXT(vulkanState.instance, &debug_report_ci, vulkanState.allocator, &vulkanState.debugReport);
//		check_vk_result(err);
//#else
//		// Create Vulkan Instance without any debug feature
//		err = vkCreateInstance(&create_info, vulkanState.allocator, &vulkanState.instance);
//		check_vk_result(err);
//		IM_UNUSED(vulkanState.debugReport);
//#endif
//	}
//	// Select GPU
//	{
//		uint32_t gpu_count;
//		err = vkEnumeratePhysicalDevices(vulkanState.instance, &gpu_count, NULL);
//		check_vk_result(err);
//		IM_ASSERT(gpu_count > 0);
//		VkPhysicalDevice* gpus = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gpu_count);
//		err = vkEnumeratePhysicalDevices(vulkanState.instance, &gpu_count, gpus);
//		check_vk_result(err);
//		// If a number >1 of GPUs got reported, find discrete GPU if present, or use first one available. This covers
//		// most common cases (multi-gpu/integrated+dedicated graphics). Handling more complicated setups (multiple
//		// dedicated GPUs) is out of scope of this sample.
//		int use_gpu = 0;
//		for (int i = 0; i < (int)gpu_count; i++)
//		{
//			VkPhysicalDeviceProperties properties;
//			vkGetPhysicalDeviceProperties(gpus[i], &properties);
//			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
//			{
//				use_gpu = i;
//				std::cout << "[ImGuiBind] Using gpu " << properties.deviceName << std::endl;
//				break;
//			}
//		}
//		vulkanState.physicalDevice = gpus[use_gpu];
//		free(gpus);
//	}
//	// Select graphics queue family
//	{
//		uint32_t count;
//		vkGetPhysicalDeviceQueueFamilyProperties(vulkanState.physicalDevice, &count, NULL);
//		VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
//		vkGetPhysicalDeviceQueueFamilyProperties(vulkanState.physicalDevice, &count, queues);
//		for (uint32_t i = 0; i < count; i++)
//			if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//			{
//				vulkanState.queueFamily = i;
//				break;
//			}
//		free(queues);
//		IM_ASSERT(vulkanState.queueFamily != (uint32_t)-1);
//	}
//	// Create Logical Device (with 1 queue)
//	{
//		int device_extension_count = 1;
//		const char* device_extensions[] = { "VK_KHR_swapchain" };
//		const float queue_priority[] = { 1.0f };
//		VkDeviceQueueCreateInfo queue_info[1] = {};
//		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//		queue_info[0].queueFamilyIndex = vulkanState.queueFamily;
//		queue_info[0].queueCount = 1;
//		queue_info[0].pQueuePriorities = queue_priority;
//		VkDeviceCreateInfo create_info = {};
//		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//		create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
//		create_info.pQueueCreateInfos = queue_info;
//		create_info.enabledExtensionCount = device_extension_count;
//		create_info.ppEnabledExtensionNames = device_extensions;
//		err = vkCreateDevice(vulkanState.physicalDevice, &create_info, vulkanState.allocator, &vulkanState.device);
//		check_vk_result(err);
//		vkGetDeviceQueue(vulkanState.device, vulkanState.queueFamily, 0, &vulkanState.queue);
//	}
//	// Create Descriptor Pool
//	{
//		VkDescriptorPoolSize pool_sizes[] =
//		{
//			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
//			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
//		};
//		VkDescriptorPoolCreateInfo pool_info = {};
//		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
//		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
//		pool_info.pPoolSizes = pool_sizes;
//		err = vkCreateDescriptorPool(vulkanState.device, &pool_info, vulkanState.allocator, &vulkanState.descriptorPool);
//		check_vk_result(err);
//	}
//
//	// Create Window Surface
//	//VkSurfaceKHR surface;
//	//err = glfwCreateWindowSurface(vulkanState.instance, window, vulkanState.allocator, &surface);
//	//check_vk_result(err);
//
//	VkSurfaceKHR surface = *((VkSurfaceKHR*)Renderer::GetWindowSurface());
//
//	// Create Framebuffers
//	int w, h;
//	glfwGetFramebufferSize(window, &w, &h);
//	SetupVulkanWindow(surface, w, h);
//}
//
//void sf::Renderer::ImGuiBind::AfterConfigure(GLFWwindow* window)
//{
//	// Setup Platform/Renderer bindings
//	ImGui_ImplGlfw_InitForVulkan(window, true);
//	ImGui_ImplVulkan_InitInfo init_info = {};
//	init_info.Instance = vulkanState.instance;
//	init_info.PhysicalDevice = vulkanState.physicalDevice;
//	init_info.Device = vulkanState.device;
//	init_info.QueueFamily = vulkanState.queueFamily;
//	init_info.Queue = vulkanState.queue;
//	init_info.PipelineCache = vulkanState.pipelineCache;
//	init_info.DescriptorPool = vulkanState.descriptorPool;
//	init_info.Allocator = vulkanState.allocator;
//	init_info.MinImageCount = vulkanState.minImageCount;
//	init_info.ImageCount = vulkanState.mainWindowData.ImageCount;
//	init_info.CheckVkResultFn = check_vk_result;
//	ImGui_ImplVulkan_Init(&init_info, vulkanState.mainWindowData.RenderPass);
//
//
//	// Upload fonts
//	VkResult err;
//
//	// Use any command queue
//	VkCommandPool command_pool = vulkanState.mainWindowData.Frames[vulkanState.mainWindowData.FrameIndex].CommandPool;
//	VkCommandBuffer command_buffer = vulkanState.mainWindowData.Frames[vulkanState.mainWindowData.FrameIndex].CommandBuffer;
//
//	err = vkResetCommandPool(vulkanState.device, command_pool, 0);
//	check_vk_result(err);
//	VkCommandBufferBeginInfo begin_info = {};
//	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//	err = vkBeginCommandBuffer(command_buffer, &begin_info);
//	check_vk_result(err);
//
//	ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
//
//	VkSubmitInfo end_info = {};
//	end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//	end_info.commandBufferCount = 1;
//	end_info.pCommandBuffers = &command_buffer;
//	err = vkEndCommandBuffer(command_buffer);
//	check_vk_result(err);
//	err = vkQueueSubmit(vulkanState.queue, 1, &end_info, VK_NULL_HANDLE);
//	check_vk_result(err);
//
//	err = vkDeviceWaitIdle(vulkanState.device);
//	check_vk_result(err);
//	ImGui_ImplVulkan_DestroyFontUploadObjects();
//}
//
//void sf::Renderer::ImGuiBind::NewFrame()
//{
//	ImGui_ImplVulkan_NewFrame();
//	ImGui_ImplGlfw_NewFrame();
//}
//
//void sf::Renderer::ImGuiBind::FrameRender(ImDrawData* draw_data)
//{
//	const glm::vec4& clearColor = Config::GetClearColor();
//	vulkanState.mainWindowData.ClearValue.color.float32[0] = clearColor.x * clearColor.w;
//	vulkanState.mainWindowData.ClearValue.color.float32[1] = clearColor.y * clearColor.w;
//	vulkanState.mainWindowData.ClearValue.color.float32[2] = clearColor.z * clearColor.w;
//	vulkanState.mainWindowData.ClearValue.color.float32[3] = clearColor.w;
//
//	VkResult err;
//
//	VkSemaphore image_acquired_semaphore = vulkanState.mainWindowData.FrameSemaphores[vulkanState.mainWindowData.SemaphoreIndex].ImageAcquiredSemaphore;
//	VkSemaphore render_complete_semaphore = vulkanState.mainWindowData.FrameSemaphores[vulkanState.mainWindowData.SemaphoreIndex].RenderCompleteSemaphore;
//	err = vkAcquireNextImageKHR(vulkanState.device, vulkanState.mainWindowData.Swapchain, UINT64_MAX, image_acquired_semaphore, VK_NULL_HANDLE, &vulkanState.mainWindowData.FrameIndex);
//	if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
//	{
//		vulkanState.swapChainRebuild = true;
//		return;
//	}
//	check_vk_result(err);
//
//	ImGui_ImplVulkanH_Frame* fd = &vulkanState.mainWindowData.Frames[vulkanState.mainWindowData.FrameIndex];
//	{
//		err = vkWaitForFences(vulkanState.device, 1, &fd->Fence, VK_TRUE, UINT64_MAX);	// wait indefinitely instead of periodically checking
//		check_vk_result(err);
//
//		err = vkResetFences(vulkanState.device, 1, &fd->Fence);
//		check_vk_result(err);
//	}
//	{
//		err = vkResetCommandPool(vulkanState.device, fd->CommandPool, 0);
//		check_vk_result(err);
//		VkCommandBufferBeginInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//		err = vkBeginCommandBuffer(fd->CommandBuffer, &info);
//		check_vk_result(err);
//	}
//	{
//		VkRenderPassBeginInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		info.renderPass = vulkanState.mainWindowData.RenderPass;
//		info.framebuffer = fd->Framebuffer;
//		info.renderArea.extent.width = vulkanState.mainWindowData.Width;
//		info.renderArea.extent.height = vulkanState.mainWindowData.Height;
//		info.clearValueCount = 1;
//		info.pClearValues = &vulkanState.mainWindowData.ClearValue;
//		vkCmdBeginRenderPass(fd->CommandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
//	}
//
//	// Record dear imgui primitives into command buffer
//	ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);
//
//	// Submit command buffer
//	vkCmdEndRenderPass(fd->CommandBuffer);
//	{
//		VkPipelineStageFlags wait_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//		VkSubmitInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//		info.waitSemaphoreCount = 1;
//		info.pWaitSemaphores = &image_acquired_semaphore;
//		info.pWaitDstStageMask = &wait_stage;
//		info.commandBufferCount = 1;
//		info.pCommandBuffers = &fd->CommandBuffer;
//		info.signalSemaphoreCount = 1;
//		info.pSignalSemaphores = &render_complete_semaphore;
//
//		err = vkEndCommandBuffer(fd->CommandBuffer);
//		check_vk_result(err);
//		err = vkQueueSubmit(vulkanState.queue, 1, &info, fd->Fence);
//		check_vk_result(err);
//	}
//	FramePresent();
//}
//
//void sf::Renderer::ImGuiBind::OnResize()
//{
//	ImGui_ImplVulkanH_CreateOrResizeWindow(vulkanState.instance, vulkanState.physicalDevice, vulkanState.device, &vulkanState.mainWindowData, vulkanState.queueFamily, vulkanState.allocator, sf::Config::GetWindowSize().x, sf::Config::GetWindowSize().y, vulkanState.minImageCount);
//}
//
//void sf::Renderer::ImGuiBind::Terminate()
//{
//	VkResult err = vkDeviceWaitIdle(vulkanState.device);
//	check_vk_result(err);
//	ImGui_ImplVulkan_Shutdown();
//	ImGui_ImplGlfw_Shutdown();
//	ImGui::DestroyContext();
//
//	ImGui_ImplVulkanH_DestroyWindow(vulkanState.instance, vulkanState.device, &vulkanState.mainWindowData, vulkanState.allocator);
//	vkDestroyDescriptorPool(vulkanState.device, vulkanState.descriptorPool, vulkanState.allocator);
//
//#ifdef IMGUI_VULKAN_DEBUG_REPORT
//	// Remove the debug report callback
//	auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vulkanState.instance, "vkDestroyDebugReportCallbackEXT");
//	vkDestroyDebugReportCallbackEXT(vulkanState.instance, vulkanState.debugReport, vulkanState.allocator);
//#endif // IMGUI_VULKAN_DEBUG_REPORT
//
//	vkDestroyDevice(vulkanState.device, vulkanState.allocator);
//	vkDestroyInstance(vulkanState.instance, vulkanState.allocator);
//}
//
//#endif SF_USE_VULKAN