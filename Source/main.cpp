#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>
#include <Memory/BaseAllocator.h>

#include <Util/HashName.h>
#include <Memory/ClassAllocator.h>

#include <Logger.h>

#include <VulkanInstance.h>
#include <VulkanInstanceInterface.h>

#include <ResourceReference.h>

// TODO: move this
void* operator new[]([[maybe_unused]] size_t size, [[maybe_unused]] const char* pName, [[maybe_unused]] int flags,
                     [[maybe_unused]] unsigned debugFlags, [[maybe_unused]] const char* file, [[maybe_unused]] int line)
{
   return Foundation::Memory::BootstrapAllocator::Allocate(static_cast<uint32_t>(size));
}
void* operator new[]([[maybe_unused]] size_t size, [[maybe_unused]] size_t alignment, [[maybe_unused]] size_t alignmentOffset,
                     [[maybe_unused]] const char* pName, [[maybe_unused]] int flags, [[maybe_unused]] unsigned debugFlags,
                     [[maybe_unused]] const char* file [[maybe_unused]], [[maybe_unused]] int line)
{
   return Foundation::Memory::BootstrapAllocator::AllocateAllign(static_cast<uint32_t>(size), static_cast<uint32_t>(alignment));
}

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
   Render::VulkanInstanceDescriptor descriptor{
       .m_instanceName = "Renderer",
       .m_version = VK_API_VERSION_1_2,
       .m_layers = {"VK_LAYER_KHRONOS_validation"},
       .m_extensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME}};
   Render::ResourceUniqueRef<Render::VulkanInstance> vulkanInstance =
       Render::VulkanInstance::CreateInstance(eastl::move(descriptor));

   // TODO: make this optional
   vulkanInstance->EnableDebugging();

   // Create the physical devices
   vulkanInstance->CreatePhysicalDevices();

   // Select and create the logical device
   vulkanInstance->SelectAndCreateLogicalDevice({VK_KHR_SWAPCHAIN_EXTENSION_NAME, /*VK_EXT_DEBUG_MARKER_EXTENSION_NAME*/});

   return 0;
}
