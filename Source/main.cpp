#include <glad/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>
#include <Memory/BaseAllocator.h>
#include <Memory/ClassAllocator.h>

#include <Util/Util.h>
#include <Util/HashName.h>
#include <Logger.h>
#include <IO/FileIO.h>

#include <ResourceReference.h>

#include <VulkanInstance.h>
#include <VulkanInstanceInterface.h>
#include <RenderWindow.h>
#include <VulkanDevice.h>
#include <Buffer.h>
#include <Renderer.h>
#include <CommandBuffer.h>
#include <Fence.h>
#include <GraphicsPipeline.h>
#include <ShaderModule.h>
#include <ShaderStage.h>
#include <DescriptorSet.h>

#include <CommandPoolManager.h>
#include <DescriptorSetLayoutManager.h>
#include <DescriptorPoolManager.h>

struct Vertex
{
   float position[3] = {};
   float color[3] = {};
};

struct Mvp
{
   glm::mat4 projectionMatrix;
   glm::mat4 modelMatrix;
   glm::mat4 viewMatrix;
};

// Creates the commandPoolManager
Render::ResourceRef<Render::CommandPoolManager> CreateCommandPoolManager(Render::ResourceRef<Render::VulkanDevice> p_vulkanDevice)
{
   using namespace Render;
   ResourceRef<CommandPoolManager> commandPoolManager;
   {
      // Create sub descriptors for the various Queues (graphics, compute and transfer)
      Render::vector<CommandPoolSubDescriptor> subDescs;
      // Register the GraphicsQueue to the CommandPoolManager
      subDescs.push_back(CommandPoolSubDescriptor{.m_queueFamilyIndex = p_vulkanDevice->GetGraphicsQueueFamilyIndex(),
                                                  .m_uuid = static_cast<uint32_t>(CommandQueueTypes::Graphics)});
      // Register the ComputeQueue to the CommandPoolManager
      subDescs.push_back(CommandPoolSubDescriptor{.m_queueFamilyIndex = p_vulkanDevice->GetCompuateQueueFamilyIndex(),
                                                  .m_uuid = static_cast<uint32_t>(CommandQueueTypes::Compute)});
      // Register the Transfer to the CommandPoolManager
      subDescs.push_back(CommandPoolSubDescriptor{.m_queueFamilyIndex = p_vulkanDevice->GetTransferQueueFamilyIndex(),
                                                  .m_uuid = static_cast<uint32_t>(CommandQueueTypes::Transfer)});

      // Create the CommandPoolManger descriptor
      {
         CommandPoolManagerDescriptor desc{.m_commandPoolSubDescriptors = eastl::move(subDescs), .m_device = p_vulkanDevice};
         // Create the CommandPoolManger
         commandPoolManager = CommandPoolManager::CreateInstance(eastl::move(desc));

         // Register it to the CommandPoolManager
         CommandPoolManager::Register(commandPoolManager.Get());
      }
   }

   return commandPoolManager;
}

// Create the DescriptorSetLayoutManager
Render::ResourceRef<Render::DescriptorSetLayoutManager>
CreateDescriptorSetLayoutManager(Render::ResourceRef<Render::VulkanDevice> p_vulkanDevice)
{
   using namespace Render;

   ResourceRef<DescriptorSetLayoutManager> descriptorSetLayoutManager;
   {
      DescriptorSetLayoutManagerDescriptor descriptorSetLayoutManagerDescriptor;
      descriptorSetLayoutManagerDescriptor.m_vulkanDevice = p_vulkanDevice;
      // Create the DescriptorSetLayoutManger
      descriptorSetLayoutManager = DescriptorSetLayoutManager::CreateInstance(eastl::move(descriptorSetLayoutManagerDescriptor));

      // Register the DescriptorSetLayoutManger
      DescriptorSetLayoutManager::Register(descriptorSetLayoutManager.Get());
   }
}

// Create the DescriptorPoolManager
Render::ResourceRef<Render::DescriptorPoolManager>
CreateDescriptorPoolManager(Render::ResourceRef<Render::VulkanDevice> p_vulkanDevice)
{
   using namespace Render;

   ResourceRef<DescriptorPoolManager> descriptorPoolManager;
   {
      struct DescriptorPoolManagerDescriptor descriptorPoolManagerDescriptor;
      descriptorPoolManagerDescriptor.m_vulkanDeviceRef = p_vulkanDevice;
      // Create the DescriptorSetLayoutManger
      descriptorPoolManager = DescriptorPoolManager::CreateInstance(eastl::move(descriptorPoolManagerDescriptor));

      // Register the DescriptorSetLayoutManger
      DescriptorPoolManager::Register(descriptorPoolManager.Get());
   }
}

// Create the Vertex and IndexBuffer
eastl::array<Render::ResourceRef<Render::Buffer>, 2u>
CreateVertexAndIndexBuffer(Render::ResourceRef<Render::VulkanDevice> p_vulkanDevice,
                           Render::ResourceRef<Render::CommandPoolManager> p_commandPoolManager)
{
   using namespace Render;
   // Setup vertices
   const Render::vector<Vertex> vertices = {{.position = {1.0f, 1.0f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
                                            {.position = {-1.0f, 1.0f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
                                            {.position = {0.0f, -1.0f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}}};
   const uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);

   // Setup indices
   const Render::vector<uint32_t> indices = {0, 1, 2};
   const uint32_t indicesSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);

   // Create the VertexBuffer
   ResourceRef<Buffer> vertexBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDeviceRef = p_vulkanDevice;
      bufferDescriptor.m_bufferSize = vertexBufferSize;
      bufferDescriptor.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      bufferDescriptor.m_bufferUsageFlags =
          Foundation::Util::SetFlags<BufferUsageFlags>(BufferUsageFlags::TransferDestination, BufferUsageFlags::VertexBuffer);
      vertexBuffer = Buffer::CreateInstance(eastl::move(bufferDescriptor));
   }

   // Create the IndexBuffer
   ResourceRef<Buffer> indexBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDeviceRef = p_vulkanDevice;
      bufferDescriptor.m_bufferSize = indicesSize;
      bufferDescriptor.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      bufferDescriptor.m_bufferUsageFlags =
          Foundation::Util::SetFlags<BufferUsageFlags>(BufferUsageFlags::TransferDestination, BufferUsageFlags::IndexBuffer);
      indexBuffer = Buffer::CreateInstance(eastl::move(bufferDescriptor));
   }

   // Create the staging buffers, and copy the Vertex and Index data from the staging buffer
   {
      // Create the Vertex staging buffer, and map the vertex data
      ResourceRef<Buffer> vertexBufferStaging;
      {
         BufferDescriptor bufferDescriptor;
         bufferDescriptor.m_vulkanDeviceRef = p_vulkanDevice;
         bufferDescriptor.m_bufferSize = vertexBufferSize;
         bufferDescriptor.m_memoryProperties =
             Foundation::Util::SetFlags<MemoryPropertyFlags>(MemoryPropertyFlags::HostVisible, MemoryPropertyFlags::HostCoherent);
         bufferDescriptor.m_bufferUsageFlags = BufferUsageFlags::TransferSource;
         vertexBufferStaging = Buffer::CreateInstance(eastl::move(bufferDescriptor));

         // Map data of the staging buffer, and copy to it
         void* data = nullptr;
         vkMapMemory(p_vulkanDevice->GetLogicalDeviceNative(), vertexBufferStaging->GetDeviceMemoryNative(), 0u,
                     vertexBufferStaging->GetBufferSizeAllocated(), 0u, &data);
         memcpy(data, vertices.data(), vertexBufferSize);
         vkUnmapMemory(p_vulkanDevice->GetLogicalDeviceNative(), vertexBufferStaging->GetDeviceMemoryNative());
      }

      // Create the Index staging buffer, and map the index data
      ResourceRef<Buffer> indexBufferStaging;
      {
         BufferDescriptor bufferDescriptor;
         bufferDescriptor.m_vulkanDeviceRef = p_vulkanDevice;
         bufferDescriptor.m_bufferSize = indicesSize;
         bufferDescriptor.m_memoryProperties =
             Foundation::Util::SetFlags<MemoryPropertyFlags>(MemoryPropertyFlags::HostVisible, MemoryPropertyFlags::HostCoherent);
         bufferDescriptor.m_bufferUsageFlags = BufferUsageFlags::TransferSource;
         indexBufferStaging = Buffer::CreateInstance(eastl::move(bufferDescriptor));

         // Map data of the staging buffer, and copy to it
         void* data = nullptr;
         vkMapMemory(p_vulkanDevice->GetLogicalDeviceNative(), indexBufferStaging->GetDeviceMemoryNative(), 0u,
                     indexBufferStaging->GetBufferSizeAllocated(), 0u, &data);
         memcpy(data, indices.data(), indicesSize);
         vkUnmapMemory(p_vulkanDevice->GetLogicalDeviceNative(), indexBufferStaging->GetDeviceMemoryNative());
      }

      // Copy data from StagingBuffer -> Buffer
      {
         CommandBufferGuard commandBuffer = p_commandPoolManager->GetCommandBuffer(
             static_cast<uint32_t>(CommandQueueTypes::Transfer), Render::CommandBufferPriority::Primary);
         // Set the CommadnBuffer to a begin state
         {
            VkCommandBufferBeginInfo commandBufferBeginInfo = {};
            commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            commandBufferBeginInfo.pNext = nullptr;
            commandBufferBeginInfo.flags = 0u;
            commandBufferBeginInfo.pInheritanceInfo = nullptr;
            VkResult res = vkBeginCommandBuffer(commandBuffer->GetCommandBufferNative(), &commandBufferBeginInfo);
            ASSERT(res == VK_SUCCESS, "Failed to set the CommandBuffer to the \"Begin\" state");
         }

         // Put buffer region copies into command buffer
         VkBufferCopy copyRegion = {};
         copyRegion.srcOffset = 0u;
         copyRegion.dstOffset = 0u;

         // Vertex buffer
         copyRegion.size = vertexBufferSize;
         vkCmdCopyBuffer(commandBuffer->GetCommandBufferNative(), vertexBufferStaging->GetBufferNative(),
                         vertexBuffer->GetBufferNative(), 1u, &copyRegion);
         // Index buffer
         copyRegion.size = indicesSize;
         vkCmdCopyBuffer(commandBuffer->GetCommandBufferNative(), indexBufferStaging->GetBufferNative(),
                         indexBuffer->GetBufferNative(), 1u, &copyRegion);

         // End the commandBuffer, and flush it.
         {
            VkResult res = vkEndCommandBuffer(commandBuffer->GetCommandBufferNative());
            ASSERT(res == VK_SUCCESS, "Failed to end the CommandBuffer");

            // Create fence to ensure that the command buffer has finished executing
            ResourceRef<Fence> stagingFence;
            {
               FenceDescriptor fenceDescriptor;
               fenceDescriptor.m_vulkanDeviceRef = p_vulkanDevice;
               stagingFence = Fence::CreateInstance(eastl::move(fenceDescriptor));
            }

            VkCommandBuffer commandBufferNative = commandBuffer->GetCommandBufferNative();
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.waitSemaphoreCount = 0u;
            submitInfo.pWaitSemaphores = nullptr;
            submitInfo.pWaitDstStageMask = nullptr;
            submitInfo.commandBufferCount = 1u;
            submitInfo.pCommandBuffers = &commandBufferNative;
            submitInfo.signalSemaphoreCount = 0u;
            submitInfo.pSignalSemaphores = nullptr;

            VkFence stagingFenceNative = stagingFence->GetFenceNative();

            // Submit to the queue
            res = vkQueueSubmit(p_vulkanDevice->GetTransferQueueNative(), 1u, &submitInfo, stagingFenceNative);
            ASSERT(res == VK_SUCCESS, "Failed to submit the queue");

            // Wait for the fence to signal that command buffer has finished executing
            const uint64_t FenceWaitTime = 100000000000u;
            res = vkWaitForFences(p_vulkanDevice->GetLogicalDeviceNative(), 1u, &stagingFenceNative, VK_TRUE, FenceWaitTime);
            ASSERT(res == VK_SUCCESS, "Failed to wait for the fence");
         }
      }
   }
   return {vertexBuffer, indexBuffer};
}

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

   // Reference to the selected Vulkan Device
   ResourceRef<VulkanDevice> vulkanDevice = vulkanInstance->GetSelectedVulkanDevice();

   // Create the CommandPoolManager
   ResourceRef<CommandPoolManager> commandPoolManager = CreateCommandPoolManager(vulkanDevice);

   // Create the DescriptorSetLayoutManager
   ResourceRef<DescriptorSetLayoutManager> descriptorSetLayoutManager = CreateDescriptorSetLayoutManager(vulkanDevice);

   // Create the DescriptorPoolManager
   ResourceRef<DescriptorPoolManager> descriptorPoolManager = CreateDescriptorPoolManager(vulkanDevice);

   // Load the Shader binaries
   ResourceRef<ShaderModule> vertexShaderModule;
   ResourceRef<ShaderModule> fragmentShaderModule;
   ResourceRef<ShaderStage> vertexShader;
   ResourceRef<ShaderStage> fragmentShader;
   {
      using namespace Foundation::IO;

      // Get the binaries
      std::vector<uint8_t> vertexShaderBin;
      std::vector<uint8_t> fragmentShaderBin;
      { // Read the VertexShader binaries
         eastl::shared_ptr<FileIOInterface> vertexShaderIO = FileIO::CreateFileIO(FileIODescriptor{
             .m_path = "C:/Users/Yun-Desktop/Desktop/projects/Renderer2/Data/Shaders/triangle.vert.spv",
             .m_fileIOFlags = Foundation::Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});

         // Open the filestream
         vertexShaderIO->Open();

         // Get the binary size, and read the data
         const uint64_t fileSize = vertexShaderIO->GetFileSize();
         vertexShaderBin.resize(fileSize);
         vertexShaderIO->Read(vertexShaderBin.data(), fileSize);
      }

      // Read the FragmentShader binaries
      {
         eastl::shared_ptr<FileIOInterface> fragmentShaderIO = FileIO::CreateFileIO(FileIODescriptor{
             .m_path = "C:/Users/Yun-Desktop/Desktop/projects/Renderer2/Data/Shaders/triangle.frag.spv",
             .m_fileIOFlags = Foundation::Util::SetFlags<FileIOFlags>(FileIOFlags::FileIOIn, FileIOFlags::FileIOBinary)});

         // Open the filestream
         fragmentShaderIO->Open();

         // Get the binary size, and read the data
         const uint64_t fileSize = fragmentShaderIO->GetFileSize();
         fragmentShaderBin.resize(fileSize);
         fragmentShaderIO->Read(fragmentShaderBin.data(), fileSize);
      }

      // Create the ShaderModules
      {
         vertexShaderModule = ShaderModule::CreateInstance(
             ShaderModuleDescriptor{.m_spirvBinary = vertexShaderBin.data(),
                                    .m_binarySizeInBytes = static_cast<uint32_t>(vertexShaderBin.size()),
                                    .m_device = vulkanDevice});

         fragmentShaderModule = ShaderModule::CreateInstance(
             ShaderModuleDescriptor{.m_spirvBinary = fragmentShaderBin.data(),
                                    .m_binarySizeInBytes = static_cast<uint32_t>(vertexShaderBin.size()),
                                    .m_device = vulkanDevice});
      }

      // Create the Shaders
      {
         vertexShader =
             ShaderStage::CreateInstance(ShaderStageDescriptor{.m_shaderModule = vertexShaderModule,
                                                               .m_shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
                                                               .m_entryPoint = "main"});

         fragmentShader =
             ShaderStage::CreateInstance(ShaderStageDescriptor{.m_shaderModule = fragmentShaderModule,
                                                               .m_shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
                                                               .m_entryPoint = "main"});
      }
   }

   // Create the Vertex and Index buffers
   auto buffers = CreateVertexAndIndexBuffer(vulkanDevice, commandPoolManager);
   ResourceRef<Buffer> vertexBuffer = buffers[0];
   ResourceRef<Buffer> indexBuffer = buffers[1];

   // Create the uniform buffers
   ResourceRef<Buffer> uniformBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDeviceRef = vulkanDevice;
      bufferDescriptor.m_bufferSize = sizeof(Mvp);
      bufferDescriptor.m_memoryProperties =
          Foundation::Util::SetFlags<MemoryPropertyFlags>(MemoryPropertyFlags::HostVisible, MemoryPropertyFlags::HostCoherent);
      bufferDescriptor.m_bufferUsageFlags = BufferUsageFlags::Uniform;
      uniformBuffer = Buffer::CreateInstance(eastl::move(bufferDescriptor));
   }

   // Create the DescriptorSetLayout();
   ResourceRef<DescriptorSetLayout> desriptorSetLayoutRef;
   {
      DescriptorSetLayoutDescriptor desc;
      desc.m_vulkanDeviceRef = vulkanDevice;

      // Create the DescriptorSetLayoutBindings
      {
         VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
         descriptorSetLayoutBinding.binding = 0u;
         descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
         descriptorSetLayoutBinding.descriptorCount = 1u;
         descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
         descriptorSetLayoutBinding.pImmutableSamplers = nullptr;

         desc.m_layoutBindings.push_back(eastl::move(descriptorSetLayoutBinding));
      }

      desriptorSetLayoutRef = DescriptorSetLayoutManagerInterface::Get()->CreateOrGetDescriptorSetLayout(eastl::move(desc));
   }

   // Create the DescriptorSet
   ResourceRef<DescriptorSet> descriptorSetRef = descriptorPoolManager->AllocateDescriptorSet(desriptorSetLayoutRef);

   // Create the ShaderModules

   // Create the GraphicsPipeline
   ResourceRef<GraphicsPipeline> graphicsPipelineRef;
   {
   }

   // TODO
   // prepareSynchronizationPrimitives();
   // buildCommandBuffers();

   // TODO:
   // Uniform buffers
   // Shaders

   return 0;
}
