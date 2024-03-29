#include <algorithm>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <TaskScheduler.h>

#include <Memory/DefinedAllocators.h>
#include <Memory/AllocatorClass.h>

#include <Util/Util.h>
#include <Util/HashName.h>
#include <Logger.h>
#include <IO/FileIO.h>

#include <Std/queue.h>
#include <Std/vector.h>

#include <RenderResource.h>

#include <VulkanInstance.h>
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
#include <Image.h>
#include <ImageView.h>
#include <RenderWindow.h>
#include <Surface.h>
#include <Swapchain.h>
#include <VertexInputState.h>
#include <RendererState.h>
#include <TimelineSemaphore.h>
#include <DescriptorSetLayout.h>
#include <BufferView.h>
#include <CommandPool.h>
#include <AsyncUploadQueue.h>
#include <ResourceDeleter.h>
#include <CommandPoolManager.h>
#include <DescriptorPoolManager.h>
#include <RendererState.h>
#include <ResourceTracker.h>
#include <Semaphore.h>

using namespace Foundation;

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

// Create the Vertex and IndexBuffer
Std::array<Render::Ptr<Render::Buffer>, 2u> CreateVertexAndIndexBuffer(Render::Ptr<Render::VulkanDevice> p_vulkanDevice)
{
   using namespace Render;
   // Setup vertices
   const Std::vector<Vertex> vertices = {{.position = {1.0f, 1.0f, 0.0f}, .color = {1.0f, 0.0f, 0.0f}},
                                         {.position = {-1.0f, 1.0f, 0.0f}, .color = {0.0f, 1.0f, 0.0f}},
                                         {.position = {0.0f, -1.0f, 0.0f}, .color = {0.0f, 0.0f, 1.0f}}};
   const uint32_t vertexBufferSize = static_cast<uint32_t>(vertices.size()) * sizeof(Vertex);

   // Setup indices
   const Std::vector<uint32_t> indices = {0u, 1u, 2u};
   const uint32_t indicesSize = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);

   // Create the VertexBuffer
   Ptr<Buffer> vertexBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDevice = p_vulkanDevice;
      bufferDescriptor.m_bufferSize = vertexBufferSize;
      bufferDescriptor.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      bufferDescriptor.m_bufferUsageFlags =
          Foundation::Util::SetFlags<BufferUsageFlags>(BufferUsageFlags::TransferDestination, BufferUsageFlags::VertexBuffer);
      bufferDescriptor.m_initialData = vertices.data();
      bufferDescriptor.m_initialDataSize = vertexBufferSize;
      vertexBuffer = Buffer::CreateInstance(eastl::move(bufferDescriptor));
   }

   // Create the IndexBuffer
   Ptr<Buffer> indexBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDevice = p_vulkanDevice;
      bufferDescriptor.m_bufferSize = indicesSize;
      bufferDescriptor.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      bufferDescriptor.m_bufferUsageFlags =
          Foundation::Util::SetFlags<BufferUsageFlags>(BufferUsageFlags::TransferDestination, BufferUsageFlags::IndexBuffer);
      bufferDescriptor.m_initialData = indices.data();
      bufferDescriptor.m_initialDataSize = indicesSize;
      indexBuffer = Buffer::CreateInstance(eastl::move(bufferDescriptor));
   }

   return {vertexBuffer, indexBuffer};
}

VkFormat GetOptimalDepthFormat(const Render::Ptr<Render::VulkanDevice>& p_vulkanDevice)
{
   // Since all depth formats may be optional, we need to find a suitable depth format to use
   // Start with the highest precision packed format
   Std::vector<VkFormat> depthFormats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
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

Render::Ptr<Render::VulkanDevice> SelectPhysicalDeviceAndCreate(Std::vector<const char*>&& p_deviceExtensions,
                                                                Std::vector<Render::Ptr<Render::VulkanDevice>>& p_vulkanDevices,
                                                                bool p_enableDebugging)
{
   using namespace Render;
   static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);
   uint32_t physicalDeviceIndex = static_cast<uint32_t>(-1);

   // Iterate through all the physical devices, and see if it supports the passed device extensions
   for (uint32_t i = 0u; i < static_cast<uint32_t>(p_vulkanDevices.size()); i++)
   {
      bool isSupported = true;

      Ptr<VulkanDevice>& vulkanDevice = p_vulkanDevices[i];

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
   Ptr<VulkanDevice>& selectedDevice = p_vulkanDevices[physicalDeviceIndex];

   // If Debug is enabled, add the marker extension if a graphics debugger is attached to it
   if (p_enableDebugging)
   {
      if (selectedDevice->IsDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
      {
         p_deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }
   }

   // Select the compatible physical device, and create a logical device
   selectedDevice->CreateLogicalDevice(eastl::move(p_deviceExtensions));

   return selectedDevice;
}

void RenderFunction()
{
   using namespace Render;

   // Initialize glfw
   ASSERT(glfwInit(), "Failed to initialize glfw");

   // Check if glfw is loaded and supported
   ASSERT(glfwVulkanSupported(), "Vulkan isn't available");

   // Create the Main RenderWindow descriptor to pass to the Vulkan Instance
   Ptr<RenderWindow> renderWindow;
   {
      RenderWindowDescriptor descriptor{
          .m_windowResolution = glm::uvec2(1920u, 1080u),
          .m_windowTitle = "Triangle",
      };
      renderWindow = RenderWindow::CreateInstance(descriptor);
   }

   // Create a Vulkan instance
   Ptr<VulkanInstance> vulkanInstance;
   {
      // Create the VulkanInstance Descriptor
      // NOTE: VulkanInstances implicitly also creates the main RenderWindow with the provided RenderWindow Descriptor
      VulkanInstanceDescriptor vulkanInstanceDescriptor{
          .m_instanceName = "Renderer",
          .m_version = VK_API_VERSION_1_3,
          .m_debug = true,
          .m_layers = {"VK_LAYER_KHRONOS_validation"},
          // NOTE: These are mandatory Instance Extensions, and will also be explicitly added
          .m_instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME, "VK_KHR_win32_surface", VK_EXT_DEBUG_UTILS_EXTENSION_NAME}};
      vulkanInstance = VulkanInstance::CreateInstance(eastl::move(vulkanInstanceDescriptor));
   }

   // Create the Surface
   Ptr<Surface> surface;
   {
      SurfaceDescriptor descriptor{.m_vulkanInstance = vulkanInstance, .m_renderWindow = renderWindow};
      surface = Surface::CreateInstance(eastl::move(descriptor));
   }

   // Create the physical devices
   Ptr<VulkanDevice> vulkanDevice;
   {
      Std::vector<Ptr<VulkanDevice>> vulkanDevices;
      const uint32_t physicalDeviceCount = vulkanInstance->GetPhysicalDevicesCount();
      vulkanDevices.reserve(physicalDeviceCount);
      // Create physical device instances
      for (uint32_t i = 0u; i < vulkanInstance->GetPhysicalDevicesCount(); i++)
      {
         vulkanDevices.push_back(VulkanDevice::CreateInstance(
             VulkanDeviceDescriptor{.m_vulkanInstance = vulkanInstance, .m_physicalDeviceIndex = i, .m_surface = surface.get()}));
      }

      // Select the physical device to use
      vulkanDevice = SelectPhysicalDeviceAndCreate(
          {
              VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
              // VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME NOTE: NOt supported yet :(
          },
          vulkanDevices, true);
   }

   // Create the Swapchain
   Ptr<Swapchain> swapchain;
   {
      SwapchainDescriptor descriptor;
      descriptor.m_vulkanDevice = vulkanDevice;
      descriptor.m_surface = surface;
      swapchain = Swapchain::CreateInstance(eastl::move(descriptor));
   }

   // Create and register the AsyncUploadQueue
   Std::unique_ptr<AsyncUploadQueue> asyncUploadQueue;
   {
      AsyncUploadQueueDescriptor asyncUploadQueueDesc{.m_vulkanDevice = vulkanDevice};
      asyncUploadQueue = Std::unique_ptr<AsyncUploadQueue>(new AsyncUploadQueue(eastl::move(asyncUploadQueueDesc)));
      AsyncUploadQueueInterface::Register(asyncUploadQueue.get());
   }

   // Create and register the CommandPoolManager
   Std::unique_ptr<CommandPoolManager> commandPoolManager;
   {
      CommandPoolManagerDescriptor desc{.m_vulkanDevice = vulkanDevice};
      commandPoolManager = Std::unique_ptr<CommandPoolManager>(new CommandPoolManager(eastl::move(desc)));
      CommandPoolManagerInterface::Register(commandPoolManager.get());
   }

   // Create and register the DescriptorPoolManager
   Std::unique_ptr<DescriptorPoolManager> descriptorPoolManager;
   {
      DescriptorPoolManagerDescriptor desc{.m_vulkanDevice = vulkanDevice};
      // Create the DescriptorSetLayoutManger
      descriptorPoolManager = Std::unique_ptr<DescriptorPoolManager>(new DescriptorPoolManager(eastl::move(desc)));
      DescriptorPoolManagerInterface::Register(descriptorPoolManager.get());
   }

   // Load the Shader binaries, create the ShaderModules, and create the ShaderStages
   Ptr<ShaderModule> vertexShaderModule;
   Ptr<ShaderModule> fragmentShaderModule;
   Ptr<ShaderStage> vertexShaderStage;
   Ptr<ShaderStage> fragmentShaderStage;
   {
      using namespace Foundation::IO;

      // Get the binaries
      std::vector<uint8_t> vertexShaderBin;
      std::vector<uint8_t> fragmentShaderBin;
      { // Read the VertexShader binaries
         eastl::shared_ptr<FileIOInterface> vertexShaderIO = FileIO::CreateFileIO(FileIODescriptor{
             .m_path = "Data/Shaders/triangle.vert.spv",
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
             .m_path = "Data/Shaders/triangle.frag.spv",
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
                                    .m_binarySizeInBytes = static_cast<uint32_t>(fragmentShaderBin.size()),
                                    .m_device = vulkanDevice});
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
   auto buffers = CreateVertexAndIndexBuffer(vulkanDevice);
   Ptr<Buffer> vertexBuffer = buffers[0];
   Ptr<Buffer> indexBuffer = buffers[1];

   // Set the uniform data
   Mvp mvp;
   {
      mvp.projectionMatrix = glm::mat4(1.0f);
      mvp.modelMatrix = glm::mat4(1.0f);
      mvp.viewMatrix = glm::mat4(1.0f);
   }

   // Create the uniform buffers
   Ptr<Buffer> uniformBuffer;
   {
      BufferDescriptor bufferDescriptor;
      bufferDescriptor.m_vulkanDevice = vulkanDevice;
      bufferDescriptor.m_bufferSize = sizeof(Mvp);
      bufferDescriptor.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      bufferDescriptor.m_bufferUsageFlags =
          Foundation::Util::SetFlags<BufferUsageFlags>(BufferUsageFlags::TransferDestination, BufferUsageFlags::Uniform);
      bufferDescriptor.m_initialData = &mvp;
      bufferDescriptor.m_initialDataSize = sizeof(Mvp);
      uniformBuffer = Buffer::CreateInstance(eastl::move(bufferDescriptor));
   }

   // Create the DescriptorSetLayout();
   Ptr<DescriptorSetLayout> desriptorSetLayout;
   {
      DescriptorSetLayoutDescriptor descriptorSetLayoutDesc;
      descriptorSetLayoutDesc.m_vulkanDevice = vulkanDevice;
      descriptorSetLayoutDesc.AddResourceLayoutBinding(0u, DescriptorType::UniformBuffer, 1u);

      desriptorSetLayout = DescriptorSetLayout::CreateInstance(eastl::move(descriptorSetLayoutDesc));
   }

   // Create the DescriptorSet
   Ptr<DescriptorSet> descriptorSet;
   {
      DescriptorSetDescriptor desc;
      desc.m_vulkanDevice = vulkanDevice;
      desc.m_descriptorSetLayout = desriptorSetLayout;

      descriptorSet = DescriptorSet::CreateInstance(eastl::move(desc));

      // Update the DescriptorSet
      {
         BufferViewDescriptor bufferViewDesc;
         bufferViewDesc.m_vulkanDevice = vulkanDevice;
         bufferViewDesc.m_buffer = uniformBuffer;
         bufferViewDesc.m_format = VK_FORMAT_UNDEFINED;
         bufferViewDesc.m_offsetFromBaseAddress = 0u;
         bufferViewDesc.m_bufferViewRange = WholeSize;
         bufferViewDesc.m_usage = BufferUsage::Uniform;
         Ptr<BufferView> bufferView = BufferView::CreateInstance(bufferViewDesc);

         descriptorSet->QueueResourceUpdate(0u, 0u, Std::vector<Ptr<BufferView>>{bufferView});
      }
   }

   // Create a DepthBuffer
   Ptr<Image> depthStencilImage;
   {
      VkExtent2D swapchainExtent = swapchain->GetExtend();
      VkExtent3D extent = VkExtent3D{.width = swapchainExtent.width, .height = swapchainExtent.height, .depth = 1u};

      ImageDescriptor imageDesc;
      imageDesc.m_vulkanDevice = vulkanDevice;
      imageDesc.m_imageCreationFlags = {};
      imageDesc.m_imageUsageFlags = ImageUsageFlags::DepthStencilAttachment;
      imageDesc.m_imageType = VkImageType::VK_IMAGE_TYPE_2D;
      imageDesc.m_extend = extent;
      imageDesc.m_format = GetOptimalDepthFormat(vulkanDevice);
      imageDesc.m_mipLevels = 1u;
      imageDesc.m_arrayLayers = 1u;
      imageDesc.m_imageTiling = VK_IMAGE_TILING_OPTIMAL;
      imageDesc.m_memoryProperties = MemoryPropertyFlags::DeviceLocal;
      imageDesc.m_initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
      depthStencilImage = Image::CreateInstance(eastl::move(imageDesc));
   }

   Ptr<ImageView> deptStencilhBufferView;
   {
      ImageViewDescriptor imageViewDesc;
      imageViewDesc.m_vulkanDevcie = vulkanDevice;
      imageViewDesc.m_image = depthStencilImage;
      imageViewDesc.m_viewType = VK_IMAGE_VIEW_TYPE_2D;
      imageViewDesc.m_format = depthStencilImage->GetImageFormatNative();
      imageViewDesc.m_baseMipLevel = 0u;
      imageViewDesc.m_mipLevelCount = 1u;
      imageViewDesc.m_baseArrayLayer = 0u;
      imageViewDesc.m_arrayLayerCount = 1u;
      imageViewDesc.m_aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      deptStencilhBufferView = ImageView::CreateInstance(eastl::move(imageViewDesc));
   }

   // Create VertexInputState
   Ptr<VertexInputState> vertexInputState;
   {
      VertexInputStateDescriptor vertexInputStateDesc;
      vertexInputState = VertexInputState::CreateInstance(eastl::move(vertexInputStateDesc));

      // Set the input binding
      VertexInputBinding& inputBinding = vertexInputState->AddVertexInputBinding(VertexInputRate::VertexInputRateVertex);
      {
         inputBinding.AddVertexInputAttribute(0u, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position));
         inputBinding.AddVertexInputAttribute(1u, VkFormat::VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color));
      }
   }

   // Create the GraphicsPipeline
   Ptr<GraphicsPipeline> graphicsPipeline;
   {
      const glm::vec2 windowResolutionFloat = renderWindow->GetWindowResolution();

      // TODO: Recheck this
      ColorBlendAttachmentState colorBlendAttachmentState = {};
      colorBlendAttachmentState.blendEnable = false;
      colorBlendAttachmentState.srcColorBlendFactor = BlendFactor::FactorZero;
      colorBlendAttachmentState.dstColorBlendFactor = BlendFactor::FactorZero;
      colorBlendAttachmentState.colorBlendOp = BlendOp::Add;
      colorBlendAttachmentState.srcAlphaBlendFactor = BlendFactor::FactorZero;
      colorBlendAttachmentState.dstAlphaBlendFactor = BlendFactor::FactorZero;
      colorBlendAttachmentState.alphaBlendOp = BlendOp::Add;
      colorBlendAttachmentState.colorWriteFlags = ColorComponentFlags::RGBA;

      GraphicsPipelineDescriptor descriptor;
      descriptor.m_vulkanDevice = vulkanDevice;
      descriptor.m_shaderStages = {vertexShaderStage, fragmentShaderStage};
      descriptor.m_descriptorSetLayouts = {desriptorSetLayout};
      descriptor.m_vertexInputState = vertexInputState;
      descriptor.m_polygonMode = PolygonMode::PolygonModeFill;
      descriptor.m_primitiveTopologyClass = PrimitiveTopologyClass::Triangle;

      descriptor.m_colorBlendAttachmentStates.push_back(colorBlendAttachmentState);

      descriptor.m_colorAttachmentFormats.push_back(swapchain->GetFormat());
      descriptor.m_depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
      descriptor.m_stencilFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

      graphicsPipeline = GraphicsPipeline::CreateInstance(eastl::move(descriptor));
   }

   // Create the Fences to check the CommandBuffer execution completion
   Ptr<TimelineSemaphore> submitWaitTimelineSemaphore;
   {
      TimelineSemaphoreDescriptor timelineSemaphoreDesc;
      timelineSemaphoreDesc.m_vulkanDevice = vulkanDevice;
      timelineSemaphoreDesc.m_initailValue = 0ul;

      submitWaitTimelineSemaphore = TimelineSemaphore::CreateInstance(timelineSemaphoreDesc);
   }

   // Get SwapchainIndex helper function
   const uint32_t swapchainImageCount = static_cast<uint32_t>(swapchain->GetSwapchainImageViews().size());
   const auto GetSwapchainIndex = [swapchainImageCount]() -> uint32_t {
      return RenderStateInterface::Get()->GetFrameIndex() % swapchainImageCount;
   };

   struct SubmitCommandBufferContext
   {
      Ptr<CommandBuffer> m_commandBuffer;
      uint64_t m_timelineSemaphoreWaitValue = static_cast<uint32_t>(-1);
   };

   Std::queue<SubmitCommandBufferContext> comandBufferContexts;
   std::mutex comandBufferContextsMutex;

   enki::TaskScheduler taskScheduler;
   taskScheduler.Initialize();
   enki::TaskSet renderThread(
       1u, [&submitWaitTimelineSemaphore, &comandBufferContexts, &comandBufferContextsMutex, &vulkanDevice, swapchain,
            renderWindow]([[maybe_unused]] enki::TaskSetPartition p_range, [[maybe_unused]] uint32_t p_threadNum) //
       {
          // Create the presentation and rendering semaphores
          Ptr<Semaphore> presentCompleteSemaphore;
          Ptr<Semaphore> renderCompleteSemaphore;
          presentCompleteSemaphore = Semaphore::CreateInstance(SemaphoreDescriptor{.m_vulkanDevice = vulkanDevice});
          renderCompleteSemaphore = Semaphore::CreateInstance(SemaphoreDescriptor{.m_vulkanDevice = vulkanDevice});

          uint64_t highestWaitValue = 0ul;

          while (!renderWindow->ShouldClose())
          {
             // TODO: Don't let the CPU keep spinning, let it wait instead
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

                // Get the index of the next Swapchain
                const uint32_t currentSwapchainBuffer = swapchain->AcquireNextImage(presentCompleteSemaphore, nullptr);

                // Submit to the graphics queue passing a wait fence
                {
                   const VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

                   SemaphoreSubmitInfo waitSemaphoreSubmitInfo{.m_semaphore = presentCompleteSemaphore,
                                                               .m_stageMask = waitStageMask};

                   SemaphoreSubmitInfo signalSemaphoreSubmitInfo{.m_semaphore = renderCompleteSemaphore,
                                                                 .m_stageMask = waitStageMask};

                   TimelineSemaphoreSubmitInfo timelineSignalSemaphoreSubmitInfo{
                       .m_timelineSemaphore = submitWaitTimelineSemaphore,
                       .p_waitOrSignalValue = commandBufferContext.m_timelineSemaphoreWaitValue,
                       .m_stageMask = waitStageMask};

                   Std::vector<SemaphoreSubmitInfo> waitSemaphores{waitSemaphoreSubmitInfo};
                   Std::vector<SemaphoreSubmitInfo> signalSemaphores{signalSemaphoreSubmitInfo};
                   Std::vector<TimelineSemaphoreSubmitInfo> timelineSignalSemaphores{timelineSignalSemaphoreSubmitInfo};

                   Std::vector<Ptr<CommandBuffer>> commandBuffers{commandBufferContext.m_commandBuffer};
                   vulkanDevice->QueueSubmit(QueueFamilyType::GraphicsQueue, commandBuffers, waitSemaphores, {}, signalSemaphores,
                                             timelineSignalSemaphores, {});

                   highestWaitValue = commandBufferContext.m_timelineSemaphoreWaitValue;
                }

                // Present
                {
                   Std::vector<Ptr<Semaphore>> waitSemaphores{renderCompleteSemaphore};
                   vulkanDevice->QueuePresent(swapchain, currentSwapchainBuffer, waitSemaphores);
                }
             }
          }

          // Wait till the latest Queue has finished
          Ptr<TimelineSemaphore> semaphore = submitWaitTimelineSemaphore;
          semaphore->WaitForValue(highestWaitValue);

          while (comandBufferContexts.size() > 0)
          {
             comandBufferContexts.pop();
          }
       });

   taskScheduler.AddTaskSetToPipe(&renderThread);

   while (!renderWindow->ShouldClose())
   {
      // Get the current resource index
      const uint32_t swapchainIndex = GetSwapchainIndex();

      // Use the current FrameIndex's fence to check if it has already been signaled
      // NOTE: Don't wait for a signal in the first frame
      const uint64_t frameIndex = RenderStateInterface::Get()->GetFrameIndex();
      const uint64_t waitValue =
          static_cast<uint64_t>(std::max(static_cast<int64_t>(frameIndex) - RendererDefines::MaxQueuedFrames + 1, 0ll));
      Ptr<TimelineSemaphore> semaphore = submitWaitTimelineSemaphore;
      semaphore->WaitForValue(waitValue);

      // From here on, the frame from RendererDefines::MaxQueuedFrames ago is guaranteed to be finished
      ResourceDeleterInterface::Get()->DeleteStaleResources();

      // Create the commandBuffer
      {
         CommandBufferDescriptor commandBufferDesc;
         commandBufferDesc.m_vulkanDevice = vulkanDevice;
         commandBufferDesc.m_queueType = QueueFamilyType::GraphicsQueue;
         Ptr<CommandBuffer> commandBuffer = CommandBuffer::CreateInstance(eastl::move(commandBufferDesc));

         // Transition the Swapchain to the VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL layout
         {
            Ptr<ImageView> swapchainImageView = swapchain->GetSwapchainImageViews()[swapchainIndex];
            commandBuffer->PipelineBarrier()->AddImageBarrier(
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                vulkanDevice->GetGraphicsQueueFamilyIndex(), vulkanDevice->GetGraphicsQueueFamilyIndex(), swapchainImageView);
         }

         // Set the line width
         const float lineWidth = 1.0f;
         commandBuffer->SetLineWidth(lineWidth);

         const float depthBiasConstantFactor = 0.0f;
         const float depthBiasClamp = 0.0f;
         const float depthBiasSlopeFactor = 0.0f;
         commandBuffer->SetDepthBias(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);

         // Bind descriptor sets describing shader binding points
         Std::vector<Ptr<DescriptorSet>> descriptorSets{descriptorSet};
         commandBuffer->BindDescriptorSets(PipelineBindPoint::Graphics, graphicsPipeline, 0u, descriptorSets);

         commandBuffer->BindPipeline(PipelineBindPoint::Graphics, graphicsPipeline);

         Std::array<float, 4> blendFactors{0.0f, 0.0f, 0.0f, 0.0f};
         commandBuffer->SetBlendConstants(eastl::move(blendFactors));

         const bool depthBoundsTestEnable = false;
         commandBuffer->SetDepthBoundsTestEnable(depthBoundsTestEnable);

         const float minDepthBounds = 0.0f;
         const float maxDepthBounds = 0.0f;
         commandBuffer->SetDepthBounds(minDepthBounds, maxDepthBounds);

         const StencilFaceFlags stencilFaceFlags = StencilFaceFlags::FrontAndBack;
         const uint32_t writeMask = 0u;
         commandBuffer->SetStencilWriteMask(stencilFaceFlags, writeMask);

         const uint32_t reference = 0u;
         commandBuffer->SetStencilReference(stencilFaceFlags, reference);

         commandBuffer->SetCullMode(CullMode::CullModeNone);

         commandBuffer->SetFrontFace(FrontFace::FrontFaceCounterClockwise);

         commandBuffer->SetPrimitiveTopology(PrimitiveTopology::TriangleList);

         Std::vector<VkViewport> viewports = {};
         {
            VkViewport viewport = {};
            const glm::vec2 renderWindowRes = renderWindow->GetWindowResolution();
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = renderWindowRes.x;
            viewport.height = renderWindowRes.y;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            viewports.push_back(viewport);
         }
         commandBuffer->SetViewportWithCount(viewports);

         Std::vector<VkRect2D> scissors = {};
         {
            VkRect2D scissor = {};
            const glm::uvec2 renderWindowRes = renderWindow->GetWindowResolution();
            scissor.extent.width = renderWindowRes.x;
            scissor.extent.height = renderWindowRes.y;
            scissor.offset.x = 0u;
            scissor.offset.y = 0u;
            scissors.push_back(scissor);
         }
         commandBuffer->SetScissorWithCount(scissors);

         const bool depthTestEnable = true;
         commandBuffer->SetDepthTestEnable(depthTestEnable);

         const bool depthWriteEnable = true;
         commandBuffer->SetDepthWriteEnable(depthWriteEnable);

         commandBuffer->SetDepthCompareOp(CompareOp::LessOrEqual);

         const bool stencilTestEnable = false;
         commandBuffer->SetStencilTestEnable(stencilTestEnable);

         commandBuffer->SetStencilOp(stencilFaceFlags, StencilOp::Keep, StencilOp::Keep, StencilOp::Keep, CompareOp::Always);

         const bool rasterizerDiscardEnable = false;
         commandBuffer->SetRasterizerDiscardEnable(rasterizerDiscardEnable);

         const bool depthBiasEnable = false;
         commandBuffer->SetDepthBiasEnable(depthBiasEnable);

         const bool primitiveRestartEnable = false;
         commandBuffer->SetPrimitiveRestartEnable(primitiveRestartEnable);

         // Bind triangle vertex buffer (contains position and colors)
         BufferViewDescriptor vertexBufferViewDesc{.m_vulkanDevice = vulkanDevice,
                                                   .m_buffer = vertexBuffer,
                                                   .m_format = VK_FORMAT_UNDEFINED,
                                                   .m_offsetFromBaseAddress = 0u,
                                                   .m_bufferViewRange = WholeSize,
                                                   .m_usage = BufferUsage::VertexBuffer};
         Ptr<BufferView> vertexBufferView = BufferView::CreateInstance(eastl::move(vertexBufferViewDesc));
         BindVertexBuffersCommand::VertexBufferView bindVertexBuffer{.m_vertexBufferView = vertexBufferView,
                                                                     .m_stride = sizeof(Vertex)};
         Std::vector<BindVertexBuffersCommand::VertexBufferView> bindVertexBuffers{bindVertexBuffer};
         commandBuffer->BindVertexBuffers(0u, bindVertexBuffers);

         // Bind triangle index buffer
         BufferViewDescriptor indexBufferViewDesc{.m_vulkanDevice = vulkanDevice,
                                                  .m_buffer = indexBuffer,
                                                  .m_format = VK_FORMAT_UNDEFINED,
                                                  .m_offsetFromBaseAddress = 0u,
                                                  .m_bufferViewRange = WholeSize,
                                                  .m_usage = BufferUsage::IndexBuffer};
         Ptr<BufferView> indexBufferView = BufferView::CreateInstance(eastl::move(indexBufferViewDesc));
         commandBuffer->BindIndexBuffer(indexBufferView, IndexType::Uint32);

         // Transition the DepthStencil buffer to the VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL layout
         {
            commandBuffer->PipelineBarrier()->AddImageBarrier(
                VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, vulkanDevice->GetGraphicsQueueFamilyIndex(),
                vulkanDevice->GetGraphicsQueueFamilyIndex(), deptStencilhBufferView);
         }

         {
            Ptr<ImageView> swapchainImageView = swapchain->GetSwapchainImageViews()[swapchainIndex];
            RenderingAttachmentInfo colorAttachmentInfo;
            colorAttachmentInfo.m_imageView = swapchainImageView;
            colorAttachmentInfo.m_imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            colorAttachmentInfo.m_resolveMode = VK_RESOLVE_MODE_NONE;
            colorAttachmentInfo.m_resolveImageView = nullptr;
            colorAttachmentInfo.m_resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachmentInfo.m_loadOp = AttachmentLoadOp::Clear;
            colorAttachmentInfo.m_storeOp = AttachmentStoreOp::Store;
            colorAttachmentInfo.m_clearValue = {.color = {.float32 = {1.0f, 0.0f, 0.0f, 0.0f}}};

            RenderingAttachmentInfo depthStencilAttachment;
            depthStencilAttachment.m_imageView = deptStencilhBufferView;
            depthStencilAttachment.m_imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
            depthStencilAttachment.m_resolveMode = VK_RESOLVE_MODE_NONE;
            depthStencilAttachment.m_resolveImageView = nullptr;
            depthStencilAttachment.m_resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthStencilAttachment.m_loadOp = AttachmentLoadOp::Clear;
            depthStencilAttachment.m_storeOp = AttachmentStoreOp::Store;
            depthStencilAttachment.m_clearValue = {.depthStencil = {.depth = 0.0f, .stencil = 0}};

            VkRect2D renderArea = {};
            renderArea.offset = {.x = 0u, .y = 0u};
            renderArea.extent = {.width = static_cast<uint32_t>(swapchain->GetExtend().width),
                                 .height = static_cast<uint32_t>(swapchain->GetExtend().height)};

            Std::vector<RenderingAttachmentInfo> colorAttachmentInfos{colorAttachmentInfo};
            commandBuffer->BeginRendering(renderArea, colorAttachmentInfos, depthStencilAttachment, depthStencilAttachment);
         }

         // Draw indexed triangle
         const uint32_t indexCount = 3u;
         commandBuffer->DrawIndexed(indexCount, 1u, 0u, 0u, 1u);

         commandBuffer->EndRendering();

         // Transition the Swapchain from VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR layout
         {
            Ptr<ImageView> swapchainImageView = swapchain->GetSwapchainImageViews()[swapchainIndex];
            commandBuffer->PipelineBarrier()->AddImageBarrier(
                VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT, VK_ACCESS_2_NONE, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, vulkanDevice->GetGraphicsQueueFamilyIndex(),
                vulkanDevice->GetGraphicsQueueFamilyIndex(), swapchainImageView);
         }

         commandBuffer->Compile();

         // Add a CommandBufferContext
         {
            std::lock_guard<std::mutex> lock(comandBufferContextsMutex);

            const uint64_t submitWaitValue = RenderStateInterface::Get()->GetFrameIndex() + 1u;
            comandBufferContexts.push(
                SubmitCommandBufferContext{.m_commandBuffer = commandBuffer, .m_timelineSemaphoreWaitValue = submitWaitValue});
         }
      }

      // Increment the FrameIndex
      RenderStateInterface::Get()->IncrementFrameIndex();

      glfwPollEvents();
   }

   taskScheduler.WaitforAll();

   CommandPoolManagerInterface::Unregister();
   DescriptorPoolManagerInterface::Unregister();
   AsyncUploadQueueInterface::Unregister();
}

int main()
{
   using namespace Render;

   // Create and register the RendererState
   Std::unique_ptr<RenderState> renderState(new RenderState(RenderStateDescriptor{}));
   RenderStateInterface::Register(renderState.get());

   // Create and register the ResourceTracker
   Std::unique_ptr<ResourceTracker> resourceTracker(new ResourceTracker());
   ResourceTrackerInterface::Register(resourceTracker.get());

   // Create and register the ResourceDeleter
   Std::unique_ptr<ResourceDeleter> resourceDeleter(new ResourceDeleter());
   ResourceDeleterInterface::Register(resourceDeleter.get());

   RenderFunction();

   resourceDeleter = nullptr;
   ResourceDeleterInterface::Unregister();

   resourceTracker = nullptr;
   ResourceTrackerInterface::Unregister();

   renderState = nullptr;
   RenderStateInterface::Unregister();

   return 0;
}
