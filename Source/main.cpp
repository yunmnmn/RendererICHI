#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>

#include <Util/HashName.h>
#include <Memory/ClassAllocator.h>

#include <Logger.h>

#include <VulkanInstance.h>
#include <VulkanInstanceInterface.h>
#include <GLFW/glfw3.h>

#include <glad/vulkan.h>

int main()
{
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   if (!glfwInit())
   {
      ASSERT(false, "Failed to initialize glfw");
   }

   // Create a Vulkan instance
   Render::VulkanInstance::Descriptor descriptor{
       .m_instanceName = "Renderer",
       .m_version = VK_API_VERSION_1_2,
       .m_layers = {"VK_LAYER_KHRONOS_validation"},
       .m_extensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME}};
   eastl::unique_ptr<Render::VulkanInstance> vulkanInstance = Render::VulkanInstance::CreateInstance(eastl::move(descriptor));

   // TODO: make this optional
   vulkanInstance->EnableDebugging();

   // Create the physical devices
   vulkanInstance->CreatePhysicalDevices();

   // Select and create the logical device
   vulkanInstance->SelectAndCreateLogicalDevice({VK_KHR_SWAPCHAIN_EXTENSION_NAME, /*VK_EXT_DEBUG_MARKER_EXTENSION_NAME*/});

   return 0;
}
