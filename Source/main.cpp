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
#include <ShaderResourceSet.h>
#include <RenderPass.h>
#include <Framebuffer.h>
#include <Image.h>
#include <ImageView.h>
#include <RenderWindow.h>
#include <Surface.h>
#include <Swapchain.h>

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

   return descriptorSetLayoutManager;
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

   return descriptorPoolManager;
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

Render::ResourceRef<Render::VulkanDevice>
SelectPhysicalDeviceAndCreate(Render::vector<const char*>&& p_deviceExtensions,
                              Render::vector<Render::ResourceRef<Render::VulkanDevice>>& p_vulkanDeviceRefs, bool p_enableDebugging)
{
   using namespace Render;
   static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);
   uint32_t physicalDeviceIndex = static_cast<uint32_t>(-1);

   // Iterate through all the physical devices, and see if it supports the passed device extensions
   for (uint32_t i = 0u; i < static_cast<uint32_t>(p_vulkanDeviceRefs.size()); i++)
   {
      bool isSupported = true;

      ResourceRef<VulkanDevice>& vulkanDevice = p_vulkanDeviceRefs[i];

      // Check if all the extensions are supported
      for (const char* deviceExtension : p_deviceExtensions)
      {
         if (!vulkanDevice->IsDeviceExtensionSupported(deviceExtension))
         {
            isSupported = false;
            break;
         }
      }

      // Check if the there is a QueueFamily that supports Graphics, Compute and Transfer
      {
         const uint32_t queueFamilyIndex =
             vulkanDevice->SupportQueueFamilyFlags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
         if (queueFamilyIndex == static_cast<uint32_t>(-1))
         {
            isSupported = false;
         }
      }

      // Check if Presenting is supported
      {
         // Check if presenting is supported in the physical device
         if (vulkanDevice->SupportPresenting() == InvalidIndex)
         {
            isSupported = false;
         }
      }

      // Check if the swapchain is supported on the device
      {
         if (!vulkanDevice->SupportSwapchain())
         {
            isSupported = false;
         }
      }

      // TODO: only support discrete GPUs for now
      // Check if it's a discrete GPU
      {
         if (!vulkanDevice->IsDiscreteGpu())
         {
            isSupported = false;
         }
      }

      // If all device extensions, queues, presenting, swapchain, discrete GPU, pick that device
      if (isSupported)
      {
         physicalDeviceIndex = i;
         break;
      }
   }

   ASSERT(physicalDeviceIndex != InvalidIndex,
          "There is no PhysicalDevice that is compatible with the required device extensions and/or supports Presenting");

   // Get a reference of the selected device
   ResourceRef<VulkanDevice>& selectedDevice = p_vulkanDeviceRefs[physicalDeviceIndex];

   // If Debug is enabled, add the marker extension if a graphics debugger is attached to it
   if (p_enableDebugging)
   {
      if (selectedDevice->IsDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
      {
         p_deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }
   }

   // TODO: make a static function(bound to the unit) instead of a lambda
   // Load the physical device specific function pointers
   const auto extensionLoader = [](const char* extension) -> GLADapiproc {
      VulkanInstanceInterface* vulkanInterface = VulkanInstanceInterface::Get();
      if (vulkanInterface)
      {
         return glfwGetInstanceProcAddress(vulkanInterface->GetInstanceNative(), extension);
      }
      else
      {
         return glfwGetInstanceProcAddress(VK_NULL_HANDLE, extension);
      }
   };
   gladLoadVulkan(selectedDevice->GetPhysicalDeviceNative(), extensionLoader);

   // Select the compatible physical device, and create a logical device
   selectedDevice->CreateLogicalDevice(eastl::move(p_deviceExtensions));

   return selectedDevice;
}

int main()
{
   using namespace Render;
   // Register the Memory Manager
   Foundation::Memory::MemoryManager memoryManager;
   Foundation::Memory::MemoryManagerInterface::Register(&memoryManager);

   // Create the Main RenderWindow descriptor to pass to the Vulkan Instance
   ResourceRef<RenderWindow> renderWindowRef;
   {
      RenderWindowDescriptor descriptor{
          .m_windowResolution = glm::uvec2(1920u, 1080u),
          .m_windowTitle = "TestWindow",
      };
      renderWindowRef = RenderWindow::CreateInstance(descriptor);
   }

   // Create a Vulkan instance
   ResourceRef<VulkanInstance> vulkanInstance;
   {
      // Create the VulkanInstance Descriptor
      // NOTE: VulkanInstances implicitly also creates the main RenderWindow with the provided RenderWindow Descriptor
      VulkanInstanceDescriptor vulkanInstanceDescriptor{
          .m_instanceName = "Renderer",
          .m_version = VK_API_VERSION_1_2,
          .m_debug = true,
          .m_layers = {"VK_LAYER_KHRONOS_validation"},
          // NOTE: These are mandatory Instance Extensions, and will also be explicitly added
          .m_instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface"}};
      vulkanInstance = VulkanInstance::CreateInstance(eastl::move(vulkanInstanceDescriptor));
   }

   // Create the Surface
   ResourceRef<Surface> surfaceRef;
   {
      SurfaceDescriptor descriptor{.m_vulkanInstanceRef = vulkanInstance, .m_renderWindowRef = renderWindowRef};
      surfaceRef = Surface::CreateInstance(eastl::move(descriptor));
   }

   // Create the physical devices
   ResourceRef<VulkanDevice> vulkanDeviceRef;
   {
      Render::vector<ResourceRef<VulkanDevice>> vulkanDeviceRefs;
      const uint32_t physicalDeviceCount = vulkanInstance->GetPhysicalDevicesCount();
      vulkanDeviceRefs.reserve(physicalDeviceCount);
      // Create physical device instances
      for (uint32_t i = 0u; i < vulkanInstance->GetPhysicalDevicesCount(); i++)
      {
         vulkanDeviceRefs.push_back(VulkanDevice::CreateInstance(VulkanDeviceDescriptor{
             .m_vulkanInstanceRef = vulkanInstance, .m_physicalDeviceIndex = i, .m_surface = surfaceRef.Get()}));
      }

      // Select the physical device to use
      vulkanDeviceRef = SelectPhysicalDeviceAndCreate({VK_KHR_SWAPCHAIN_EXTENSION_NAME}, vulkanDeviceRefs, true);
   }

   // Create the Swapchain
   ResourceRef<Swapchain> swapchainRef;
   {
      SwapchainDescriptor descriptor;
      descriptor.m_vulkanDeviceRef = vulkanDeviceRef;
      descriptor.m_surfaceRef = surfaceRef;
   }

   // Create the CommandPoolManager
   ResourceRef<CommandPoolManager> commandPoolManager = CreateCommandPoolManager(vulkanDeviceRef);

   // Create the DescriptorSetLayoutManager
   ResourceRef<DescriptorSetLayoutManager> descriptorSetLayoutManager = CreateDescriptorSetLayoutManager(vulkanDeviceRef);

   // Create the DescriptorPoolManager
   ResourceRef<DescriptorPoolManager> descriptorPoolManager = CreateDescriptorPoolManager(vulkanDeviceRef);

   // Load the Shader binaries
   ResourceRef<ShaderModule> vertexShaderModule;
   ResourceRef<ShaderModule> fragmentShaderModule;
   ResourceRef<ShaderStage> vertexShaderStage;
   ResourceRef<ShaderStage> fragmentShaderStage;
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
                                    .m_device = vulkanDeviceRef});

         fragmentShaderModule = ShaderModule::CreateInstance(
             ShaderModuleDescriptor{.m_spirvBinary = fragmentShaderBin.data(),
                                    .m_binarySizeInBytes = static_cast<uint32_t>(fragmentShaderBin.size()),
                                    .m_device = vulkanDeviceRef});
      }

      // Create the Shaders
      {
         vertexShaderStage =
             ShaderStage::CreateInstance(ShaderStageDescriptor{.m_shaderModule = vertexShaderModule,
                                                               .m_shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT,
                                                               .m_entryPoint = "main"});

         fragmentShaderStage =
             ShaderStage::CreateInstance(ShaderStageDescriptor{.m_shaderModule = fragmentShaderModule,
                                                               .m_shaderStage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT,
                                                               .m_entryPoint = "main"});
      }
   }

   // Create the Vertex and Index buffers
   auto buffers = CreateVertexAndIndexBuffer(vulkanDeviceRef, commandPoolManager);
   ResourceRef<Buffer> vertexBuffer = buffers[0];
   ResourceRef<Buffer> indexBuffer = buffers[1];

   // Create the uniform buffers
   ResourceRef<Buffer> uniformBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDeviceRef = vulkanDeviceRef;
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
      desc.m_vulkanDeviceRef = vulkanDeviceRef;

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

   // Create a DepthBuffer
   ResourceRef<Image> depthBufferRef;
   ResourceRef<ImageView> depthBufferViewRef;
   {
      ImageDescriptor desc;
      desc.m_vulkanDeviceRef = vulkanDevice;
      desc.m_imageType = VkImageType::VK_IMAGE_TYPE_2D;
      desc.m_extend =
   }

   // Create the RenderPass
   ResourceRef<RenderPass> renderPassRef;
   {
      RenderPassDescriptor descriptor;
      descriptor.m_colorAttachments = {RenderPassDescriptor::RenderPassAttachmentDescriptor{
          .m_loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, .m_storeOp = VK_ATTACHMENT_STORE_OP_STORE, m_attachment = }};
      descriptor.m_depthAttachment = ;
      descriptor.m_vulkanDeviceRef = vulkanDeviceRef;
   }

   // Create a Framebuffer for each Swapchain
   Render::vector<ResourceRef<Framebuffer>> framebufferRefs;
   {
   }

   // Create the GraphicsPipeline
   ResourceRef<GraphicsPipeline> graphicsPipelineRef;
   {
      GraphicsPipelineDescriptor descriptor;
      descriptor.m_shaderStages = {vertexShaderStage, fragmentShaderStage};
      descriptor.m_descriptorSetLayouts = {desriptorSetLayoutRef};
      descriptor.m_vulkanDeviceRef = vulkanDeviceRef;
      descriptor.m_renderPass = renderPassRef;
   }

   // TODO
   // prepareSynchronizationPrimitives();
   // buildCommandBuffers();

   // TODO:
   // Uniform buffers

   return 0;
}
