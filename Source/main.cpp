#include <glad/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <TaskScheduler.h>

#include <Memory/MemoryManager.h>
#include <Memory/MemoryManagerInterface.h>
#include <Memory/BaseAllocator.h>
#include <Memory/ClassAllocator.h>

#include <Util/Util.h>
#include <Util/HashName.h>
#include <Logger.h>
#include <IO/FileIO.h>

#include <std/queue.h>
#include <std/vector.h>

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
#include <Framebuffer.h>
#include <GraphicsPipeline.h>
#include <VertexInputState.h>
#include <RendererState.h>
#include <TimelineSemaphore.h>

#include <CommandPoolManager.h>
#include <DescriptorSetLayoutManager.h>
#include <DescriptorPoolManager.h>
#include <RendererStateInterface.h>

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
         CommandPoolManagerDescriptor desc{.m_commandPoolSubDescriptors = eastl::move(subDescs),
                                           .m_vulkanDeviceRef = p_vulkanDevice};
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

VkFormat GetOptimalDepthFormat(const Render::ResourceRef<Render::VulkanDevice>& p_vulkanDevice)
{
   // Since all depth formats may be optional, we need to find a suitable depth format to use
   // Start with the highest precision packed format
   Render::vector<VkFormat> depthFormats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
                                            VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

   for (auto& format : depthFormats)
   {
      VkFormatProperties formatProps;
      vkGetPhysicalDeviceFormatProperties(p_vulkanDevice->GetPhysicalDeviceNative(), format, &formatProps);
      // Format must support depth stencil attachment for optimal tiling
      if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
      {
         return format;
      }
   }

   return {};
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

   // Initialize glfw
   ASSERT(glfwInit(), "Failed to initialize glfw");

   // Check if glfw is loaded and supported
   ASSERT(glfwVulkanSupported(), "Vulkan isn't available");

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
      swapchainRef = Swapchain::CreateInstance(eastl::move(descriptor));
   }

   // Create the SwapchainImage resources
   Render::vector<ResourceRef<Image>> swapchainImageRefs;
   {
      const uint32_t swapchainImageCount = swapchainRef->GetSwapchainImageCount();
      swapchainImageRefs.reserve(swapchainImageCount);
      for (uint32_t i = 0u; i < swapchainRef->GetSwapchainImageCount(); i++)
      {
         ImageDescriptor2 descriptor{.m_vulkanDeviceRef = vulkanDeviceRef, .m_swapchainRef = swapchainRef, .m_swapchainIndex = i};
         swapchainImageRefs.push_back(Image::CreateInstance(eastl::move(descriptor)));
      }
   }

   // Create the SwapchainImageView resources
   Render::vector<ResourceRef<ImageView>> swapchainImageViewRefs;
   {
      swapchainImageViewRefs.reserve(swapchainImageRefs.size());
      for (const ResourceRef<Image>& swapchainImageRef : swapchainImageRefs)
      {
         ImageViewSwapchainDescriptor descriptor{.m_vulkanDevcieRef = vulkanDeviceRef, .m_image = swapchainImageRef};
         swapchainImageViewRefs.push_back(ImageView::CreateInstance(eastl::move(descriptor)));
      }
   }

   // Create the CommandPoolManager
   ResourceRef<CommandPoolManager> commandPoolManager = CreateCommandPoolManager(vulkanDeviceRef);

   // Create the DescriptorSetLayoutManager
   ResourceRef<DescriptorSetLayoutManager> descriptorSetLayoutManager = CreateDescriptorSetLayoutManager(vulkanDeviceRef);

   // Create the DescriptorPoolManager
   ResourceRef<DescriptorPoolManager> descriptorPoolManager = CreateDescriptorPoolManager(vulkanDeviceRef);

   // Create the RendererState
   ResourceRef<RenderState> renderStateRef = RenderState::CreateInstance(RenderStateDescriptor{});
   RenderStateInterface::Register(renderStateRef.Get());

   // Load the Shader binaries, create the ShaderModules, and create the ShaderStages
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

      // Write to uniform buffer
      {
         Mvp mvp;
         mvp.projectionMatrix = glm::mat4(1.0f);
         mvp.modelMatrix = glm::mat4(1.0f);
         mvp.viewMatrix = glm::mat4(1.0f);

         // Map uniform buffer and update it
         uint8_t* pData;
         vkMapMemory(vulkanDeviceRef->GetLogicalDeviceNative(), uniformBuffer->GetDeviceMemoryNative(), 0, sizeof(Mvp), 0,
                     (void**)&pData);
         memcpy(pData, &mvp, sizeof(Mvp));
         // Unmap after data has been copied
         // Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
         vkUnmapMemory(vulkanDeviceRef->GetLogicalDeviceNative(), uniformBuffer->GetDeviceMemoryNative());
      }
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
   ResourceRef<DescriptorSet> descriptorSetRef;
   {
      descriptorSetRef = descriptorSetRef = descriptorPoolManager->AllocateDescriptorSet(desriptorSetLayoutRef);

      // Write the descriptor
      // TODO: make better
      {
         // Update the descriptor set determining the shader binding points
         // For every binding point used in a shader there needs to be one
         // descriptor set matching that binding point
         VkDescriptorBufferInfo bufferDescriptor = {};
         bufferDescriptor.buffer = uniformBuffer->GetBufferNative();
         bufferDescriptor.offset = 0;
         bufferDescriptor.range = sizeof(Mvp);

         VkWriteDescriptorSet writeDescriptorSet = {};
         // Binding 0 : Uniform buffer
         writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
         writeDescriptorSet.dstSet = descriptorSetRef->GetDescriptorSetNative();
         writeDescriptorSet.descriptorCount = 1;
         writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
         writeDescriptorSet.pBufferInfo = &bufferDescriptor;
         // Binds this uniform buffer to binding point 0
         writeDescriptorSet.dstBinding = 0;

         vkUpdateDescriptorSets(vulkanDeviceRef->GetLogicalDeviceNative(), 1, &writeDescriptorSet, 0, nullptr);
      }
   }

   // Create a DepthBuffer
   ResourceRef<Image> depthStencilImage;
   {
      VkExtent2D swapchainExtent = swapchainRef->GetExtend();
      VkExtent3D extent = VkExtent3D{.width = swapchainExtent.width, .height = swapchainExtent.height, .depth = 1u};

      ImageDescriptor desc;
      desc.m_vulkanDeviceRef = vulkanDeviceRef;
      desc.m_imageCreationFlags = {};
      desc.m_imageUsageFlags = ImageUsageFlags::DepthStencilAttachment;
      desc.m_imageType = VkImageType::VK_IMAGE_TYPE_2D;
      desc.m_extend = extent;
      desc.m_format = GetOptimalDepthFormat(vulkanDeviceRef);
      desc.m_mipLevels = 1u;
      desc.m_arrayLayers = 1u;
      desc.m_imageTiling = VK_IMAGE_TILING_OPTIMAL;
      desc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      desc.m_initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
      depthStencilImage = Image::CreateInstance(eastl::move(desc));
   }

   ResourceRef<ImageView> depthBufferViewRef;
   {
      ImageViewDescriptor desc;
      desc.m_vulkanDevcieRef = vulkanDeviceRef;
      desc.m_image = depthStencilImage;
      desc.m_viewType = VK_IMAGE_VIEW_TYPE_2D;
      desc.m_format = depthStencilImage->GetImageFormatNative();
      desc.m_baseMipLevel = 0u;
      desc.m_mipLevelCount = 1u;
      desc.m_baseArrayLayer = 0u;
      desc.m_arrayLayerCount = 1u;
      desc.m_aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      depthBufferViewRef = ImageView::CreateInstance(eastl::move(desc));
   }

   //// Create the RenderPass
   ResourceRef<RenderPass> renderPassRef;
   {
      RenderPassDescriptor descriptor;
      descriptor.m_colorAttachments = {
          RenderPassDescriptor::RenderPassAttachmentDescriptor{.m_loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                               .m_storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                               .m_format = swapchainRef->GetFormat()}};
      descriptor.m_depthAttachment =
          RenderPassDescriptor::RenderPassAttachmentDescriptor{.m_loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                                                               .m_storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                                                               .m_format = depthStencilImage->GetImageFormatNative()};
      descriptor.m_vulkanDeviceRef = vulkanDeviceRef;
      renderPassRef = RenderPass::CreateInstance(eastl::move(descriptor));
   }

   // Create a Framebuffer for each Swapchain
   Render::vector<ResourceRef<Framebuffer>> framebufferRefs;
   {
      framebufferRefs.reserve(swapchainImageViewRefs.size());
      for (ResourceRef<ImageView>& swapchainImageViewRef : swapchainImageViewRefs)
      {
         FrameBufferDescriptor desc;
         desc.m_vulkanDeviceRef = vulkanDeviceRef;
         desc.m_renderPassRef = renderPassRef;
         desc.m_attachmentRefs = {swapchainImageViewRef, depthBufferViewRef};
         desc.m_frameBufferCreateFlags = {};
         framebufferRefs.push_back(Framebuffer::CreateInstance(eastl::move(desc)));
      }
   }

   // Create VertexInputState
   Render::ResourceRef<VertexInputState> vertexInputStateRef;
   {
      VertexInputStateDescriptor desc;
      vertexInputStateRef = VertexInputState::CreateInstance(eastl::move(desc));

      // Set the input binding
      VertexInputBinding& inputBinding =
          vertexInputStateRef->AddVertexInputBinding(VertexInputRate::VertexInputRateVertex, sizeof(Vertex));
      {
         inputBinding.AddVertexInputAttribute(0u, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
         inputBinding.AddVertexInputAttribute(1u, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
      }
   }

   // Create the GraphicsPipeline
   ResourceRef<GraphicsPipeline> graphicsPipelineRef;
   {
      GraphicsPipelineDescriptor descriptor;
      descriptor.m_shaderStages = {vertexShaderStage, fragmentShaderStage};
      descriptor.m_descriptorSetLayouts = {desriptorSetLayoutRef};
      descriptor.m_vulkanDeviceRef = vulkanDeviceRef;
      descriptor.m_renderPass = renderPassRef;
      descriptor.m_vertexInputState = vertexInputStateRef;
      descriptor.m_primitiveTopology = PrimitiveTopology::TriangleList;
      descriptor.m_rasterizationState = RasterizationState();
      descriptor.m_scissor = Scissor{.m_offset = glm::ivec2(0, 0), .m_extend = glm::uvec2(1920u, 1080u)};
      descriptor.m_viewport = {
          .m_x = 0.0f, .m_y = 0.0, .m_width = 1920.0f, .m_height = 1080.0f, .m_minDepth = 0.0f, .m_maxDepth = 1.0f};
      graphicsPipelineRef = GraphicsPipeline::CreateInstance(eastl::move(descriptor));
   }

   // Create the Fences to check the CommandBuffer execution completion
   ResourceRef<TimelineSemaphore> submitWaitTimelineSemaphore;
   {
      TimelineSemaphoreDescriptor desc;
      desc.m_vulkanDeviceRef = vulkanDeviceRef;
      // Set it already to a signaled state by setting the initial value to RendererDefines::MaxQueuedFrames -1
      desc.m_initailValue = RendererDefines::MaxQueuedFrames - 1u;

      submitWaitTimelineSemaphore = TimelineSemaphore::CreateInstance(desc);
   }

   // Get SwapchainIndex helper function
   const uint32_t swapchainImageCount = static_cast<uint32_t>(swapchainImageViewRefs.size());
   const auto GetSwapchainIndex = [swapchainImageCount]() -> uint32_t {
      return RenderStateInterface::Get()->GetFrameIndex() % swapchainImageCount;
   };

   struct SubmitCommandBufferContext
   {
      ResourceRef<CommandBuffer> m_commandBufferRef;
      uint64_t m_timelineSemaphoreWaitValue = static_cast<uint32_t>(-1);
   };

   Render::queue<SubmitCommandBufferContext> comandBufferContexts;
   std::mutex comandBufferContextsMutex;

   enki::TaskScheduler taskScheduler;
   taskScheduler.Initialize();
   enki::TaskSet renderThread(1u, [&submitWaitTimelineSemaphore, &comandBufferContexts, &comandBufferContextsMutex,
                                   &vulkanDeviceRef, &swapchainRef]([[maybe_unused]] enki::TaskSetPartition p_range,
                                                                    [[maybe_unused]] uint32_t p_threadNum) {
      // Create the presentation and rendering semaphores
      VkSemaphore presentCompleteSemaphore;
      VkSemaphore renderCompleteSemaphore;
      {
         // Semaphores (Used for correct command ordering)
         VkSemaphoreCreateInfo semaphoreCreateInfo = {};
         semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
         semaphoreCreateInfo.pNext = nullptr;
         semaphoreCreateInfo.flags = 0;

         // Semaphore used to ensures that image presentation is complete before starting to submit again
         VkResult res =
             vkCreateSemaphore(vulkanDeviceRef->GetLogicalDeviceNative(), &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
         ASSERT(res == VK_SUCCESS, "Failed to create the semaphore");

         // Semaphore used to ensures that all commands submitted have been finished before submitting the image to the queue
         res =
             vkCreateSemaphore(vulkanDeviceRef->GetLogicalDeviceNative(), &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
         ASSERT(res == VK_SUCCESS, "Failed to create the semaphore");
      }

      while (true)
      {
         uint32_t currentSwapchainBuffer = static_cast<uint32_t>(-1);

         {
            SubmitCommandBufferContext commandBufferContext;
            // Get the oldest submitted CommandBufferContext
            {
               // Lock the queue first
               std::lock_guard<std::mutex> lock(comandBufferContextsMutex);

               // Early out if it's empty
               if (comandBufferContexts.empty())
               {
                  continue;
               }

               // Get the oldest element in the queue
               commandBufferContext = eastl::move(comandBufferContexts.front());
               comandBufferContexts.pop();
            }

            VkCommandBuffer commandBufferNative = commandBufferContext.m_commandBufferRef->GetCommandBufferNative();

            // Get the index of the next Swapchain
            VkResult res = vkAcquireNextImageKHR(vulkanDeviceRef->GetLogicalDeviceNative(), swapchainRef->GetSwapchainNative(),
                                                 UINT64_MAX, presentCompleteSemaphore, VK_NULL_HANDLE, &currentSwapchainBuffer);
            ASSERT(res == VK_SUCCESS, "Failed to acquire the next image from the swapchain");

            // Get next image in the swap chain (back/front buffer)
            swapchainRef->GetSwapchainNative();

            // Pipeline stage at which the queue submission will wait (via pWaitSemaphores)
            VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            // Timeline semaphore
            uint64_t timelineSignalValue[] = {commandBufferContext.m_timelineSemaphoreWaitValue,
                                              commandBufferContext.m_timelineSemaphoreWaitValue};
            uint64_t ignoredWaitValue = static_cast<uint64_t>(-1);
            VkTimelineSemaphoreSubmitInfo timelineSemaphoreSubmitInfo = {};
            timelineSemaphoreSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
            timelineSemaphoreSubmitInfo.pNext = nullptr;
            timelineSemaphoreSubmitInfo.waitSemaphoreValueCount = 1u;
            timelineSemaphoreSubmitInfo.pWaitSemaphoreValues = &ignoredWaitValue;
            timelineSemaphoreSubmitInfo.signalSemaphoreValueCount = 2u;
            timelineSemaphoreSubmitInfo.pSignalSemaphoreValues = timelineSignalValue;

            // The submit info structure specifies a command buffer queue submission batch
            const uint32_t signalSemaphoreCount = 2u;
            VkSemaphore signalSemaphores[signalSemaphoreCount] = {submitWaitTimelineSemaphore->GetTimelineSemaphoreNative(),
                                                                  renderCompleteSemaphore};

            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext = &timelineSemaphoreSubmitInfo;
            submitInfo.pWaitDstStageMask = &waitStageMask;
            submitInfo.pWaitSemaphores = &presentCompleteSemaphore;
            submitInfo.waitSemaphoreCount = 1u;
            submitInfo.pSignalSemaphores = signalSemaphores;
            submitInfo.signalSemaphoreCount = signalSemaphoreCount;
            submitInfo.pCommandBuffers = &commandBufferNative;
            submitInfo.commandBufferCount = 1u;

            // Submit to the graphics queue passing a wait fence
            res = vkQueueSubmit(vulkanDeviceRef->GetGraphicsQueueNative(), 1u, &submitInfo, VK_NULL_HANDLE);
            ASSERT(res == VK_SUCCESS, "Failed to submit the queue");
         }

         VkSwapchainKHR swapchainNative = swapchainRef->GetSwapchainNative();
         VkPresentInfoKHR presentInfo = {};
         presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
         presentInfo.pNext = NULL;
         presentInfo.swapchainCount = 1u;
         presentInfo.pSwapchains = &swapchainNative;
         presentInfo.pImageIndices = &currentSwapchainBuffer;
         presentInfo.pWaitSemaphores = &renderCompleteSemaphore;
         presentInfo.waitSemaphoreCount = 1u;
         VkResult res = vkQueuePresentKHR(vulkanDeviceRef->GetGraphicsQueueNative(), &presentInfo);

         ASSERT(res == VK_SUCCESS, "Failed to present the queue");
      }
   });

   taskScheduler.AddTaskSetToPipe(&renderThread);

   VkCommandBufferBeginInfo cmdBufInfo = {};
   cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   cmdBufInfo.pNext = nullptr;

   VkClearValue clearValues[2];
   clearValues[0].color = {{0.0f, 0.0f, 0.2f, 1.0f}};
   clearValues[1].depthStencil = {1.0f, 0u};

   VkRenderPassBeginInfo renderPassBeginInfo = {};
   renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   renderPassBeginInfo.pNext = nullptr;
   renderPassBeginInfo.renderPass = renderPassRef->GetRenderPassNative();
   renderPassBeginInfo.renderArea.offset.x = 0;
   renderPassBeginInfo.renderArea.offset.y = 0;
   renderPassBeginInfo.renderArea.extent.width = 1920;
   renderPassBeginInfo.renderArea.extent.height = 1080;
   renderPassBeginInfo.clearValueCount = 2;
   renderPassBeginInfo.pClearValues = clearValues;

   while (true)
   {
      // Get the current resource index
      const uint32_t resourceIndex = RenderStateInterface::Get()->GetResourceIndex();
      const uint32_t swapchainIndex = GetSwapchainIndex();

      // Use the current FrameIndex's fence to check if it has already been signaled
      // NOTE: Don't wait for a signal in the first frame
      uint64_t waitValue = RenderStateInterface::Get()->GetFrameIndex();
      ResourceRef<TimelineSemaphore> semaphore = submitWaitTimelineSemaphore;
      VkSemaphore semaphoreNative = semaphore->GetTimelineSemaphoreNative();
      VkSemaphoreWaitInfo waitInfo;
      waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
      waitInfo.pNext = NULL;
      waitInfo.flags = 0;
      waitInfo.semaphoreCount = 1u;
      waitInfo.pSemaphores = &semaphoreNative;
      waitInfo.pValues = &waitValue;
      VkResult res = vkWaitSemaphores(vulkanDeviceRef->GetLogicalDeviceNative(), &waitInfo, UINT64_MAX);
      ASSERT(res == VK_SUCCESS, "Failed to wait for the TimelineSemaphore");

      // Update the COmmandPoolManager
      CommandPoolManagerInterface::Get()->Update();

      // Create the commandBuffer
      {
         CommandBufferGuard commandBuffer = commandPoolManager->GetCommandBuffer(static_cast<uint32_t>(CommandQueueTypes::Graphics),
                                                                                 CommandBufferPriority::Primary);

         // Set target frame buffer
         renderPassBeginInfo.framebuffer = framebufferRefs[swapchainIndex]->GetFrameBufferNative();

         res = vkBeginCommandBuffer(commandBuffer.Get()->GetCommandBufferNative(), &cmdBufInfo);
         ASSERT(res == VK_SUCCESS, "Failed to begin the commandbuffer");

         // Transition the Swapchain to the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL layout
         {
            VkImageMemoryBarrier imageMemoryBarrier = {};
            {
               VkImageSubresourceRange subResourceRange = {};
               subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
               subResourceRange.baseMipLevel = 0u;
               subResourceRange.levelCount = swapchainImageRefs[swapchainIndex]->GetMipLevels();
               subResourceRange.baseArrayLayer = 0u;
               subResourceRange.layerCount = swapchainImageRefs[swapchainIndex]->GetArrayLayers();

               imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
               imageMemoryBarrier.pNext = nullptr;
               imageMemoryBarrier.srcAccessMask = 0u;
               imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
               imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
               imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
               imageMemoryBarrier.srcQueueFamilyIndex = vulkanDeviceRef->GetGraphicsQueueFamilyIndex();
               imageMemoryBarrier.dstQueueFamilyIndex = vulkanDeviceRef->GetGraphicsQueueFamilyIndex();
               imageMemoryBarrier.image = swapchainImageRefs[swapchainIndex]->GetImageNative();
               imageMemoryBarrier.subresourceRange = subResourceRange;
            }
            vkCmdPipelineBarrier(commandBuffer.Get()->GetCommandBufferNative(),
                                 VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, {}, 0u, nullptr, 0u,
                                 nullptr, 1u, &imageMemoryBarrier);
         }

         // Transition the DepthStencil buffer to the VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL layout
         {
            VkImageMemoryBarrier imageMemoryBarrier = {};
            {
               VkImageSubresourceRange subResourceRange = {};
               subResourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
               subResourceRange.baseMipLevel = 0u;
               subResourceRange.levelCount = depthStencilImage->GetMipLevels();
               subResourceRange.baseArrayLayer = 0u;
               subResourceRange.layerCount = depthStencilImage->GetArrayLayers();

               imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
               imageMemoryBarrier.pNext = nullptr;
               imageMemoryBarrier.srcAccessMask = 0u;
               imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
               imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
               imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
               imageMemoryBarrier.srcQueueFamilyIndex = vulkanDeviceRef->GetGraphicsQueueFamilyIndex();
               imageMemoryBarrier.dstQueueFamilyIndex = vulkanDeviceRef->GetGraphicsQueueFamilyIndex();
               imageMemoryBarrier.image = depthStencilImage->GetImageNative();
               imageMemoryBarrier.subresourceRange = subResourceRange;
            }
            vkCmdPipelineBarrier(commandBuffer.Get()->GetCommandBufferNative(),
                                 VkPipelineStageFlagBits::VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VkPipelineStageFlagBits::VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, {}, 0u, nullptr, 0u, nullptr,
                                 1u, &imageMemoryBarrier);
         }

         vkCmdBeginRenderPass(commandBuffer.Get()->GetCommandBufferNative(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

         // Update dynamic viewport state
         VkViewport viewport = {};
         viewport.width = 1920.0f;
         viewport.height = 1080.0f;
         viewport.minDepth = (float)0.0f;
         viewport.maxDepth = (float)1.0f;
         vkCmdSetViewport(commandBuffer.Get()->GetCommandBufferNative(), 0, 1, &viewport);

         // Update dynamic scissor state
         VkRect2D scissor = {};
         scissor.extent.width = 1920;
         scissor.extent.height = 1080;
         scissor.offset.x = 0;
         scissor.offset.y = 0;
         vkCmdSetScissor(commandBuffer.Get()->GetCommandBufferNative(), 0, 1, &scissor);

         // Bind descriptor sets describing shader binding points
         VkDescriptorSet descriptorSet = descriptorSetRef->GetDescriptorSetNative();
         vkCmdBindDescriptorSets(commandBuffer.Get()->GetCommandBufferNative(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                 graphicsPipelineRef->GetGraphicsPipelineLayoutNative(), 0, 1, &descriptorSet, 0, nullptr);

         vkCmdBindPipeline(commandBuffer.Get()->GetCommandBufferNative(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                           graphicsPipelineRef->GetGraphicsPipelineNative());

         // Bind triangle vertex buffer (contains position and colors)
         VkDeviceSize offsets[1] = {0};
         VkBuffer vertexBufferNative = vertexBuffer->GetBufferNative();
         vkCmdBindVertexBuffers(commandBuffer.Get()->GetCommandBufferNative(), 0, 1, &vertexBufferNative, offsets);

         // Bind triangle index buffer
         VkBuffer indexBufferNative = indexBuffer->GetBufferNative();
         vkCmdBindIndexBuffer(commandBuffer.Get()->GetCommandBufferNative(), indexBufferNative, 0, VK_INDEX_TYPE_UINT32);

         // Draw indexed triangle
         const uint32_t indexCount = 3u;
         vkCmdDrawIndexed(commandBuffer.Get()->GetCommandBufferNative(), indexCount, 1, 0, 0, 1);

         vkCmdEndRenderPass(commandBuffer.Get()->GetCommandBufferNative());

         // Transition the Swapchain from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout
         {
            VkImageMemoryBarrier imageMemoryBarrier = {};
            {
               VkImageSubresourceRange subResourceRange = {};
               subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
               subResourceRange.baseMipLevel = 0u;
               subResourceRange.levelCount = swapchainImageRefs[swapchainIndex]->GetMipLevels();
               subResourceRange.baseArrayLayer = 0u;
               subResourceRange.layerCount = swapchainImageRefs[swapchainIndex]->GetArrayLayers();

               imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
               imageMemoryBarrier.pNext = nullptr;
               imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
               imageMemoryBarrier.dstAccessMask = {};
               imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
               imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
               imageMemoryBarrier.srcQueueFamilyIndex = vulkanDeviceRef->GetGraphicsQueueFamilyIndex();
               imageMemoryBarrier.dstQueueFamilyIndex = vulkanDeviceRef->GetGraphicsQueueFamilyIndex();
               imageMemoryBarrier.image = swapchainImageRefs[swapchainIndex]->GetImageNative();
               imageMemoryBarrier.subresourceRange = subResourceRange;
            }
            vkCmdPipelineBarrier(commandBuffer.Get()->GetCommandBufferNative(),
                                 VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                 VkPipelineStageFlagBits::VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, {}, 0u, nullptr, 0u, nullptr, 1u,
                                 &imageMemoryBarrier);
         }

         res = vkEndCommandBuffer(commandBuffer.Get()->GetCommandBufferNative());
         ASSERT(res == VK_SUCCESS, "Failed to end the commandbuffer");

         // Add a CommandBufferContext
         {
            std::lock_guard<std::mutex> lock(comandBufferContextsMutex);

            const uint64_t submitWaitValue = RenderStateInterface::Get()->GetFrameIndex() + RendererDefines::MaxQueuedFrames;
            comandBufferContexts.push(SubmitCommandBufferContext{.m_commandBufferRef = commandBuffer.GetRef(),
                                                                 .m_timelineSemaphoreWaitValue = submitWaitValue});
         }
      }

      // Increment the FrameIndex
      RenderStateInterface::Get()->IncrementFrameIndex();

      glfwPollEvents();
   }

   return 0;
}
