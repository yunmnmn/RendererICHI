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
#include <RenderWindow.h>

#include <ResourceReference.h>

//// TODO: move this
void* operator new[]([[maybe_unused]] size_t size, [[maybe_unused]] const char* pName, [[maybe_unused]] int flags,
                     [[maybe_unused]] unsigned debugFlags, [[maybe_unused]] const char* file, [[maybe_unused]] int line)
{
   ASSERT(false, "Should never be called");
   return Foundation::Memory::BootstrapAllocator::Allocate(static_cast<uint32_t>(size));
}
void* operator new[]([[maybe_unused]] size_t size, [[maybe_unused]] size_t alignment, [[maybe_unused]] size_t alignmentOffset,
                     [[maybe_unused]] const char* pName, [[maybe_unused]] int flags, [[maybe_unused]] unsigned debugFlags,
                     [[maybe_unused]] const char* file [[maybe_unused]], [[maybe_unused]] int line)
{
   ASSERT(false, "Should never be called");
   return Foundation::Memory::BootstrapAllocator::AllocateAllign(static_cast<uint32_t>(size), static_cast<uint32_t>(alignment));
}

int main()
{
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   // Create a Vulkan instance
   Render::ResourceUniqueRef<Render::VulkanInstance> vulkanInstance;
   {
      // Create the Main RenderWindow descriptor to pass to the Vulkan Instance
      Render::RenderWindowDescriptor mainRenderWindowDescriptor{
          .m_windowResolution = glm::uvec2(1920u, 1080u),
          .m_windowTitle = "TestWindow",
      };

      // Create the VulkanInstance Descriptor
      // NOTE: VulkanInstances implicitly also creates the main RenderWindow with the provided RenderWindow Descriptor
      Render::VulkanInstanceDescriptor vulkanInstanceDescriptor{
          .m_instanceName = "Renderer",
          .m_mainRenderWindow = mainRenderWindowDescriptor,
          .m_version = VK_API_VERSION_1_2,
          .m_debug = true,
          .m_layers = {"VK_LAYER_KHRONOS_validation"},
          // NOTE: These are mandatory Instance Extensions, and will also be explicitly added
          .m_instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"}};
      vulkanInstance = Render::VulkanInstance::CreateInstance(eastl::move(vulkanInstanceDescriptor));
   }

   // Create all physical devices
   vulkanInstance->CreatePhysicalDevices();

   // Select the most suitable PhysicalDevice, and create the logical device
   vulkanInstance->SelectAndCreateLogicalDevice({VK_KHR_SWAPCHAIN_EXTENSION_NAME});

   return 0;
}
