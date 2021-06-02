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
#include <VulkanDevice.h>
#include <Buffer.h>
#include <Renderer.h>

#include <ResourceReference.h>

#include <CommandPoolManager.h>

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

struct Vertex
{
   float position[3] = {};
   float color[3] = {};
};

int main()
{
   using namespace Render;
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   // Create a Vulkan instance
   Render::ResourceRef<Render::VulkanInstance> vulkanInstance;
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

   // Refernece to the selected Vulkan Device
   ResourceRef<VulkanDevice> vulkanDevice = vulkanInstance->GetSelectedVulkanDevice();

   // Create the CommandPoolManager
   ResourceRef<CommandPoolManager> commandPoolManager;
   {
      // Create sub descriptors for the various Queues (graphics, compute and transfer)
      Render::vector<CommandPoolSubDescriptor> subDescs;
      // Register the GraphicsQueue to the CommandPoolManager
      subDescs.push_back(CommandPoolSubDescriptor{.m_queueFamilyIndex = vulkanDevice->GetGraphicsQueueFamilyIndex(),
                                                  .m_uuid = static_cast<uint32_t>(CommandQueueTypes::Graphics)});
      // Register the ComputeQueue to the CommandPoolManager
      subDescs.push_back(CommandPoolSubDescriptor{.m_queueFamilyIndex = vulkanDevice->GetCompuateQueueFamilyIndex(),
                                                  .m_uuid = static_cast<uint32_t>(CommandQueueTypes::Compute)});
      // Register the Transfer to the CommandPoolManager
      subDescs.push_back(CommandPoolSubDescriptor{.m_queueFamilyIndex = vulkanDevice->GetTransferQueueFamilyIndex(),
                                                  .m_uuid = static_cast<uint32_t>(CommandQueueTypes::Transfer)});

      // Create the CommandPoolManger descriptor
      {
         CommandPoolManagerDescriptor desc{.m_commandPoolSubDescriptors = eastl::move(subDescs), .m_device = vulkanDevice};
         // Create the CommandPoolManger
         commandPoolManager = CommandPoolManager::CreateInstance(eastl::move(desc));

         // Register it to the CommandPoolManager
         CommandPoolManager::Register(commandPoolManager.Get());
      }
   }

   // prepareSynchronizationPrimitives();

   ResourceRef<Buffer> vertexBufferResource;
   {
      // Setup vertices
      const Render::vector<Vertex> vertices = {{.position = {1.0f, 1.0f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
                                               {.position = {-1.0f, 1.0f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
                                               {.position = {0.0f, -1.0f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}}};
      const uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);

      // Create the Vertex Buffer, and upload the data
      {
         BufferDescriptor bufferDescriptor;
         bufferDescriptor.m_vulkanDeviceRef = vulkanDevice;
         bufferDescriptor.m_bufferSize = vertexBufferSize;
         bufferDescriptor.m_bufferUsageFlags =
             RendererHelper::SetFlags<BufferUsageFlags>(BufferUsageFlags::TransferDestination, BufferUsageFlags::VertexBuffer);
         vertexBufferResource = Buffer::CreateInstance(eastl::move(bufferDescriptor));
      }
   }

   // prepareUniformBuffers();
   // setupDescriptorSetLayout();
   // preparePipelines();
   // setupDescriptorPool();
   // setupDescriptorSet();
   // buildCommandBuffers();

   // TODO:
   // Uniform buffers
   // Shaders
   // vertices

   return 0;
}
