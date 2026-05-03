#include <GHI/RenderGraph.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/CommandBuffer.h>
#include <GHI/Device.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/RenderCommands.h>

using namespace Render;
using GHI::Ptr;

namespace
{

void Expect(bool p_condition, const char* p_message)
{
   if (!p_condition)
   {
      throw std::runtime_error(p_message);
   }
}

class TestDevice final : public GHI::Device
{
 public:
   TestDevice() : GHI::Device(GHI::DeviceDescriptor{})
   {
   }

   GHI::QueueSubmitResult QueueSubmitInternal([[maybe_unused]] GHI::QueueFamilyType p_queueType,
                                              [[maybe_unused]] const std::vector<Ptr<GHI::CommandBuffer>>& p_commandBuffers,
                                              [[maybe_unused]] const std::vector<GHI::FenceSubmitInfo>& p_waitFor,
                                              [[maybe_unused]] const std::vector<GHI::FenceSubmitInfo>& p_signalAfter) final
   {
      return {};
   }

   void WaitFencesInternal([[maybe_unused]] std::vector<GHI::FenceSubmitInfo> p_waitFor) final
   {
   }

 private:
   void ReleaseInternal() final
   {
   }
};

class TestCommandBuffer final : public GHI::CommandBuffer
{
 public:
   explicit TestCommandBuffer(Ptr<GHI::Device> p_device)
       : GHI::CommandBuffer(std::move(p_device), GHI::CommandBufferDescriptor{})
   {
   }

   void CompileInternal() final
   {
   }

 private:
   void ReleaseInternal() final
   {
   }
};

class TestImage final : public GHI::Image
{
 public:
   explicit TestImage(Ptr<GHI::Device> p_device)
       : GHI::Image(std::move(p_device),
                    GHI::ImageDescriptor{.m_imageUsageFlags = GHI::ImageUsageFlags::ColorAttachment,
                                         .m_imageType = GHI::ImageType::Image2D,
                                         .m_extend = glm::uvec3(16u, 16u, 1u),
                                         .m_format = GHI::ResourceFormat::B8G8R8A8Srgb,
                                         .m_imageTiling = GHI::ImageTiling::TilingOptimal,
                                         .m_memoryProperties = GHI::MemoryPropertyFlags::DeviceLocal,
                                         .m_initialLayout = GHI::ImageLayout::Undefined})
   {
   }

 private:
   void ReleaseInternal() final
   {
   }
};

class TestImageView final : public GHI::ImageView
{
 public:
   TestImageView(Ptr<GHI::Device> p_device, Ptr<GHI::Image> p_image)
       : GHI::ImageView(std::move(p_device),
                        GHI::ImageViewDescriptor{.m_image = std::move(p_image),
                                                 .m_extend = glm::uvec3(16u, 16u, 1u),
                                                 .m_viewType = GHI::ImageViewType::View2D,
                                                 .m_format = GHI::ResourceFormat::B8G8R8A8Srgb,
                                                 .m_aspectMask = GHI::ImageAspectFlags::Color})
   {
   }

 private:
   void ReleaseInternal() final
   {
   }
};

class TestBuffer final : public GHI::Buffer
{
 public:
   explicit TestBuffer(Ptr<GHI::Device> p_device)
       : GHI::Buffer(std::move(p_device),
                     GHI::BufferDescriptor{.m_requestBufferSize = 256u,
                                           .m_bufferUsageFlags = GHI::BufferUsageFlags::VertexBuffer |
                                                                 GHI::BufferUsageFlags::TransferDestination,
                                           .m_queueFamilyAccess = GHI::QueueTypeFlags::GraphicsQueue,
                                           .m_memoryProperties = GHI::MemoryPropertyFlags::DeviceLocal})
   {
   }

   Ptr<GHI::Fence> UploadDataInternal([[maybe_unused]] const void* p_data,
                                      [[maybe_unused]] uint64_t p_dataSize) final
   {
      return nullptr;
   }

   void UploadDataImmediateInternal([[maybe_unused]] const void* p_data,
                                    [[maybe_unused]] uint64_t p_dataSize) final
   {
   }

   void* MapInternal([[maybe_unused]] uint64_t p_offset, [[maybe_unused]] uint64_t p_size = GHI::WholeSize) final
   {
      return nullptr;
   }

   void UnmapInternal() final
   {
   }

 private:
   void ReleaseInternal() final
   {
   }
};

class TestBufferView final : public GHI::BufferView
{
 public:
   TestBufferView(Ptr<GHI::Device> p_device, Ptr<GHI::Buffer> p_buffer)
       : GHI::BufferView(std::move(p_device),
                         GHI::BufferViewDescriptor{.m_buffer = std::move(p_buffer),
                                                   .m_bufferViewRange = 256u,
                                                   .m_usage = GHI::BufferUsage::VertexBuffer})
   {
   }

   void InitInternal() final
   {
   }

   void ShutdownInternal() final
   {
   }

 private:
   void ReleaseInternal() final
   {
   }
};

Ptr<GHI::ImageView> CreateImageView(const Ptr<GHI::Device>& p_device)
{
   Ptr<GHI::Image> image = std::make_shared<TestImage>(p_device);
   return std::make_shared<TestImageView>(p_device, std::move(image));
}

Ptr<GHI::BufferView> CreateBufferView(const Ptr<GHI::Device>& p_device)
{
   Ptr<GHI::Buffer> buffer = std::make_shared<TestBuffer>(p_device);
   return std::make_shared<TestBufferView>(p_device, std::move(buffer));
}

GHI::ImageDescriptor CreateColorImageDescriptor()
{
   return GHI::ImageDescriptor{.m_imageUsageFlags = GHI::ImageUsageFlags::ColorAttachment,
                               .m_imageType = GHI::ImageType::Image2D,
                               .m_extend = glm::uvec3(16u, 16u, 1u),
                               .m_format = GHI::ResourceFormat::B8G8R8A8Srgb,
                               .m_imageTiling = GHI::ImageTiling::TilingOptimal,
                               .m_memoryProperties = GHI::MemoryPropertyFlags::DeviceLocal,
                               .m_initialLayout = GHI::ImageLayout::Undefined};
}

GHI::BufferDescriptor CreateStorageBufferDescriptor()
{
   return GHI::BufferDescriptor{.m_requestBufferSize = 256u,
                                .m_bufferUsageFlags = GHI::BufferUsageFlags::Storage,
                                .m_queueFamilyAccess = GHI::QueueTypeFlags::GraphicsQueue,
                                .m_memoryProperties = GHI::MemoryPropertyFlags::DeviceLocal};
}

const GHI::PipelineBarrierCommand& GetBarrier(std::span<const GHI::RenderCommand> p_commands, size_t p_index)
{
   Expect(p_index < p_commands.size(), "Expected render command is missing");

   const GHI::PipelineBarrierCommand* barrier = std::get_if<GHI::PipelineBarrierCommand>(&p_commands[p_index]);
   Expect(barrier != nullptr, "Expected a PipelineBarrierCommand");
   return *barrier;
}

void InstallTestBarrierEmitter(GHI::RenderGraph& p_graph)
{
   p_graph.SetBarrierEmitter([](GHI::CommandBuffer& p_commandBuffer,
                                const GHI::RenderGraphBarrierInfo& p_barrierInfo) {
      const GHI::ResourceUsageInfo oldInfo =
          GHI::ResourceUsageToInfo(p_barrierInfo.m_oldUsage, p_barrierInfo.m_oldShaderStages);
      const GHI::ResourceUsageInfo newInfo =
          GHI::ResourceUsageToInfo(p_barrierInfo.m_newUsage, p_barrierInfo.m_newShaderStages);

      constexpr uint32_t IgnoredQueueFamily = static_cast<uint32_t>(-1);
      GHI::PipelineBarrierCommand* barrier = p_commandBuffer.PipelineBarrier();
      if (p_barrierInfo.m_resourceType == GHI::RenderGraphResourceType::Image)
      {
         Expect(p_barrierInfo.m_imageView != nullptr, "Test RenderGraph image barrier has no ImageView");
         barrier->AddImageBarrier(oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages,
                                  newInfo.m_access, oldInfo.m_imageLayout, newInfo.m_imageLayout,
                                  IgnoredQueueFamily, IgnoredQueueFamily, p_barrierInfo.m_imageView);
         return;
      }

      Expect(p_barrierInfo.m_resourceType == GHI::RenderGraphResourceType::Buffer,
             "Test RenderGraph barrier has unsupported resource type");
      Expect(p_barrierInfo.m_bufferView != nullptr, "Test RenderGraph buffer barrier has no BufferView");
      barrier->AddBufferBarrier(oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages,
                                newInfo.m_access, IgnoredQueueFamily, IgnoredQueueFamily,
                                p_barrierInfo.m_bufferView);
   });
}

void TestImageBarrierSequenceAndExecutionOrder()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> imageView = CreateImageView(device);

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   const GHI::RenderGraphResourceHandle color =
       graph.ImportImageView("color", imageView, GHI::ResourceUsage::Undefined);

   std::vector<std::string> executionLog;
   graph.AddPass("sample color")
       .Read(color, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          executionLog.push_back("sample");
       });

   graph.AddPass("write color")
       .Write(color, GHI::ResourceUsage::ColorAttachmentWrite)
       .Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          executionLog.push_back("write");
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect((executionLog == std::vector<std::string>{"write", "sample"}),
          "RenderGraph executed image passes in the wrong order");

   const std::span<const GHI::RenderCommand> commands = commandBuffer.GetRenderCommands();
   Expect(commands.size() == 2u, "RenderGraph should have emitted two image barriers");

   const GHI::PipelineBarrierCommand& firstBarrier = GetBarrier(commands, 0u);
   const std::vector<GHI::PipelineImageBarrier>& firstImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(firstBarrier);
   Expect(firstImageBarriers.size() == 1u, "First barrier should contain one image barrier");
   Expect(firstImageBarriers[0].m_imageView == imageView, "First image barrier references the wrong ImageView");
   Expect(firstImageBarriers[0].m_oldLayout == GHI::ImageLayout::Undefined,
          "First image barrier should transition from Undefined");
   Expect(firstImageBarriers[0].m_newLayout == GHI::ImageLayout::ColorAttachment,
          "First image barrier should transition to ColorAttachment");
   Expect(firstImageBarriers[0].m_srcAccessMask == GHI::AccessFlags::None,
          "First image barrier should have no source access");
   Expect(firstImageBarriers[0].m_dstAccessMask == GHI::AccessFlags::ColorAttachmentWrite,
          "First image barrier should write as a color attachment");

   const GHI::PipelineBarrierCommand& secondBarrier = GetBarrier(commands, 1u);
   const std::vector<GHI::PipelineImageBarrier>& secondImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(secondBarrier);
   Expect(secondImageBarriers.size() == 1u, "Second barrier should contain one image barrier");
   Expect(secondImageBarriers[0].m_oldLayout == GHI::ImageLayout::ColorAttachment,
          "Second image barrier should transition from ColorAttachment");
   Expect(secondImageBarriers[0].m_newLayout == GHI::ImageLayout::ShaderRead,
          "Second image barrier should transition to ShaderRead");
   Expect(secondImageBarriers[0].m_srcAccessMask == GHI::AccessFlags::ColorAttachmentWrite,
          "Second image barrier should wait on color attachment writes");
   Expect(secondImageBarriers[0].m_dstAccessMask == GHI::AccessFlags::ShaderRead,
          "Second image barrier should make shader reads visible");
}

void TestPassCanProduceMultipleOutputs()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> albedoView;
   Ptr<GHI::ImageView> normalView;
   Ptr<GHI::ImageView> depthView;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   uint32_t materializedImageIndex = 0u;
   graph.SetImageMaterializer([device, &albedoView, &normalView, &depthView,
                               &materializedImageIndex]([[maybe_unused]] const GHI::ImageDescriptor& p_desc) {
      Ptr<GHI::ImageView> imageView = CreateImageView(device);
      if (materializedImageIndex == 0u)
      {
         albedoView = imageView;
      }
      else if (materializedImageIndex == 1u)
      {
         normalView = imageView;
      }
      else if (materializedImageIndex == 2u)
      {
         depthView = imageView;
      }
      ++materializedImageIndex;
      return imageView;
   });

   std::vector<std::string> executionLog;
   auto [albedo, normal, depth] =
       graph.AddPass("gbuffer")
           .WriteImage("gbuffer albedo", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite)
           .WriteImage("gbuffer normal", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite)
           .WriteImage("gbuffer depth", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite)
       .Execute([&executionLog](GHI::RenderGraphContext& p_context) {
          Expect(p_context.GetOutputCount() == 3u, "GBuffer execute should expose three outputs");
          executionLog.push_back("gbuffer");
       });

   graph.AddPass("lighting")
       .Read(normal, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Read(albedo, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Read(depth, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          executionLog.push_back("lighting");
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect((executionLog == std::vector<std::string>{"gbuffer", "lighting"}),
          "RenderGraph did not order a multi-output producer before its consumer");

   const std::span<const GHI::RenderCommand> commands = commandBuffer.GetRenderCommands();
   Expect(commands.size() == 6u, "Multi-output producer/consumer should emit six image barriers");

   const std::vector<GHI::PipelineImageBarrier>& firstImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 0u));
   const std::vector<GHI::PipelineImageBarrier>& secondImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 1u));
   const std::vector<GHI::PipelineImageBarrier>& thirdImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 2u));
   const std::vector<GHI::PipelineImageBarrier>& fourthImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 3u));
   const std::vector<GHI::PipelineImageBarrier>& fifthImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 4u));
   const std::vector<GHI::PipelineImageBarrier>& sixthImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 5u));

   Expect(firstImageBarriers[0].m_imageView == albedoView,
          "First multi-output barrier should transition albedo for the producer");
   Expect(secondImageBarriers[0].m_imageView == normalView,
          "Second multi-output barrier should transition normal for the producer");
   Expect(thirdImageBarriers[0].m_imageView == depthView,
          "Third multi-output barrier should transition depth for the producer");
   Expect(fourthImageBarriers[0].m_imageView == normalView,
          "Fourth multi-output barrier should transition normal for the consumer");
   Expect(fifthImageBarriers[0].m_imageView == albedoView,
          "Fifth multi-output barrier should transition albedo for the consumer");
   Expect(sixthImageBarriers[0].m_imageView == depthView,
          "Sixth multi-output barrier should transition depth for the consumer");
}

void TestZeroOutputSideEffectPassExecutes()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);

   uint32_t callbackCount = 0u;
   graph.AddPass("debug marker")
       .SideEffect()
       .Execute([&callbackCount]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          ++callbackCount;
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect(callbackCount == 1u, "RenderGraph did not execute a zero-output side-effect pass");
   Expect(commandBuffer.GetRenderCommands().empty(),
          "Zero-output side-effect pass should not emit barriers by itself");
}

void TestReadWriteBuildersCreateInputAndOutput()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> imageView;
   Ptr<GHI::BufferView> bufferView;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageMaterializer([device, &imageView]([[maybe_unused]] const GHI::ImageDescriptor& p_desc) {
      imageView = CreateImageView(device);
      return imageView;
   });
   graph.SetBufferMaterializer([device, &bufferView]([[maybe_unused]] const GHI::BufferDescriptor& p_desc) {
      bufferView = CreateBufferView(device);
      return bufferView;
   });
   std::vector<std::string> executionLog;

   auto [storageImage] =
       graph.AddPass("readwrite image")
           .ReadWriteImage("storage image", CreateColorImageDescriptor(),
                           GHI::ResourceUsage::StorageReadWrite, GHI::ShaderStageFlag::Compute)
       .Prepare([&imageView](GHI::RenderGraphPrepareContext& p_context) {
          const GHI::RenderGraphResourceHandle storageImage = p_context.GetOutput(0u);

          Expect(p_context.GetInputCount() == 1u, "ReadWrite image should be exposed as one input");
          Expect(p_context.GetOutputCount() == 1u, "ReadWrite image should be exposed as one output");
          Expect(p_context.GetInput(0u).m_index == storageImage.m_index,
                 "ReadWrite image input handle is wrong");
          Expect(p_context.GetOutput(0u).m_index == storageImage.m_index,
                 "ReadWrite image output handle is wrong");
          Expect(p_context.GetImageDescriptor(storageImage) != nullptr,
                 "ReadWrite image should expose its ImageDescriptor");
          Expect(p_context.GetImageView(storageImage) == imageView,
                 "ReadWrite image should be materialized before pass prepare");
       })
       .Execute([&executionLog](GHI::RenderGraphContext& p_context) {
          Expect(p_context.GetInputCount() == 1u, "ReadWrite image execute should expose one input");
          Expect(p_context.GetOutputCount() == 1u, "ReadWrite image execute should expose one output");
          executionLog.push_back("readwrite image");
       });

   auto [storageBuffer] =
       graph.AddPass("readwrite buffer")
           .ReadWriteBuffer("storage buffer", CreateStorageBufferDescriptor(),
                            GHI::ResourceUsage::StorageReadWrite, GHI::ShaderStageFlag::Compute)
       .Prepare([&bufferView](GHI::RenderGraphPrepareContext& p_context) {
          const GHI::RenderGraphResourceHandle storageBuffer = p_context.GetOutput(0u);

          Expect(p_context.GetInputCount() == 1u, "ReadWrite buffer should be exposed as one input");
          Expect(p_context.GetOutputCount() == 1u, "ReadWrite buffer should be exposed as one output");
          Expect(p_context.GetInput(0u).m_index == storageBuffer.m_index,
                 "ReadWrite buffer input handle is wrong");
          Expect(p_context.GetOutput(0u).m_index == storageBuffer.m_index,
                 "ReadWrite buffer output handle is wrong");
          Expect(p_context.GetBufferDescriptor(storageBuffer) != nullptr,
                 "ReadWrite buffer should expose its BufferDescriptor");
          Expect(p_context.GetBufferView(storageBuffer) == bufferView,
                 "ReadWrite buffer should be materialized before pass prepare");
       })
       .Execute([&executionLog](GHI::RenderGraphContext& p_context) {
          Expect(p_context.GetInputCount() == 1u, "ReadWrite buffer execute should expose one input");
          Expect(p_context.GetOutputCount() == 1u, "ReadWrite buffer execute should expose one output");
          executionLog.push_back("readwrite buffer");
       });

   graph.AddPass("sample readwrite image")
       .Read(storageImage, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          executionLog.push_back("sample image");
       });

   graph.AddPass("consume readwrite buffer")
       .Read(storageBuffer, GHI::ResourceUsage::VertexRead)
       .Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          executionLog.push_back("consume buffer");
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect((executionLog == std::vector<std::string>{"readwrite image", "readwrite buffer",
                                                    "sample image", "consume buffer"}),
          "RenderGraph did not preserve read-write producer dependencies");

   const std::span<const GHI::RenderCommand> commands = commandBuffer.GetRenderCommands();
   Expect(commands.size() == 4u, "ReadWrite producer/consumer should emit four barriers");

   const std::vector<GHI::PipelineImageBarrier>& firstImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 0u));
   const std::vector<GHI::PipelineBufferBarrier>& secondBufferBarriers =
       GHI::RenderCommandAccess::GetBufferBarriers(GetBarrier(commands, 1u));
   const std::vector<GHI::PipelineImageBarrier>& thirdImageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 2u));
   const std::vector<GHI::PipelineBufferBarrier>& fourthBufferBarriers =
       GHI::RenderCommandAccess::GetBufferBarriers(GetBarrier(commands, 3u));

   Expect(firstImageBarriers.size() == 1u, "ReadWrite image producer should emit one image barrier");
   Expect(firstImageBarriers[0].m_imageView == imageView,
          "ReadWrite image producer barrier references the wrong ImageView");
   Expect(firstImageBarriers[0].m_oldLayout == GHI::ImageLayout::Undefined,
          "ReadWrite image producer should transition from Undefined");
   Expect(firstImageBarriers[0].m_newLayout == GHI::ImageLayout::General,
          "ReadWrite image producer should transition to General");

   Expect(secondBufferBarriers.size() == 1u, "ReadWrite buffer producer should emit one buffer barrier");
   Expect(secondBufferBarriers[0].m_bufferView == bufferView,
          "ReadWrite buffer producer barrier references the wrong BufferView");
   Expect(secondBufferBarriers[0].m_srcAccessMask == GHI::AccessFlags::None,
          "ReadWrite buffer producer should have no source access for graph-created storage");
   Expect(secondBufferBarriers[0].m_dstAccessMask == (GHI::AccessFlags::ShaderRead |
                                                     GHI::AccessFlags::ShaderWrite),
          "ReadWrite buffer producer should transition to shader read/write");

   Expect(thirdImageBarriers.size() == 1u, "ReadWrite image consumer should emit one image barrier");
   Expect(thirdImageBarriers[0].m_oldLayout == GHI::ImageLayout::General,
          "ReadWrite image consumer should transition from General");
   Expect(thirdImageBarriers[0].m_newLayout == GHI::ImageLayout::ShaderRead,
          "ReadWrite image consumer should transition to ShaderRead");

   Expect(fourthBufferBarriers.size() == 1u, "ReadWrite buffer consumer should emit one buffer barrier");
   Expect(fourthBufferBarriers[0].m_srcAccessMask == (GHI::AccessFlags::ShaderRead |
                                                      GHI::AccessFlags::ShaderWrite),
          "ReadWrite buffer consumer should wait on shader read/write");
   Expect(fourthBufferBarriers[0].m_dstAccessMask == GHI::AccessFlags::VertexAttributeRead,
          "ReadWrite buffer consumer should transition to vertex reads");
}

void TestPrepareMaterializesDescriptorResourceAndDetectsTransient()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> materializedView;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageMaterializer([device, &materializedView]([[maybe_unused]] const GHI::ImageDescriptor& p_desc) {
      materializedView = CreateImageView(device);
      return materializedView;
   });
   std::vector<std::string> events;

   auto [color] =
       graph.AddPass("produce prepared color")
           .WriteImage("prepared color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite)
       .Prepare([&materializedView, &events](GHI::RenderGraphPrepareContext& p_context) {
          const GHI::RenderGraphResourceHandle color = p_context.GetOutput(0u);

          Expect(p_context.GetPassName() == "produce prepared color",
                 "Prepare context reported the wrong producer pass");
          Expect(p_context.GetInputCount() == 0u, "Producer prepare should not have inputs");
          Expect(p_context.GetOutputCount() == 1u, "Producer prepare should expose one output");
          Expect(p_context.GetOutput(0u).m_index == color.m_index,
                 "Producer prepare exposed the wrong output");
          Expect(p_context.GetImageDescriptor(color) != nullptr,
                 "Descriptor-backed output should expose its ImageDescriptor during prepare");
          Expect(p_context.GetImageView(color) == materializedView,
                 "Descriptor-backed output should be materialized before pass prepare");

          events.push_back("prepare producer");
       })
       .Execute([&materializedView, &events](GHI::RenderGraphContext& p_context) {
          const GHI::RenderGraphResourceHandle color = p_context.GetOutput(0u);

          Expect(p_context.GetImageView(color) == materializedView,
                 "Execute context did not see the prepared ImageView");
          events.push_back("execute producer");
       });

   graph.AddPass("consume prepared color")
       .Read(color, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Prepare([&](GHI::RenderGraphPrepareContext& p_context) {
          Expect(p_context.GetPassName() == "consume prepared color",
                 "Prepare context reported the wrong consumer pass");
          Expect(p_context.GetInputCount() == 1u, "Consumer prepare should expose one input");
          Expect(p_context.GetInput(0u).m_index == color.m_index,
                 "Consumer prepare exposed the wrong input");
          Expect(p_context.GetImageView(color) == materializedView,
                 "Consumer prepare should see resources materialized by earlier prepares");
          events.push_back("prepare consumer");
       })
       .Execute([&events]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          events.push_back("execute consumer");
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect((events == std::vector<std::string>{"prepare producer", "prepare consumer",
                                              "execute producer", "execute consumer"}),
          "RenderGraph did not run prepare before execute in solved pass order");
   Expect(graph.WasResourceCreatedInPrepare(color),
          "Prepared descriptor-backed resource should be marked as created in prepare");
   Expect(graph.CanResourceBeTransient(color),
          "Prepared descriptor-backed resource should be marked as transient-capable");
   Expect(graph.GetResourceFirstUseOrder(color) == 0u,
          "Prepared resource should first be used by the producer pass");
   Expect(graph.GetResourceLastUseOrder(color) == 1u,
          "Prepared resource should last be used by the consumer pass");
}

void TestPrepareCreatesPassLocalTransientResources()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> scratchImageView;
   Ptr<GHI::BufferView> scratchBufferView;
   GHI::RenderGraphResourceHandle scratchImage;
   GHI::RenderGraphResourceHandle scratchBuffer;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageMaterializer([device, &scratchImageView]([[maybe_unused]] const GHI::ImageDescriptor& p_desc) {
      scratchImageView = CreateImageView(device);
      return scratchImageView;
   });
   graph.SetBufferMaterializer([device, &scratchBufferView]([[maybe_unused]] const GHI::BufferDescriptor& p_desc) {
      scratchBufferView = CreateBufferView(device);
      return scratchBufferView;
   });

   std::vector<std::string> events;
   graph.AddPass("scratch resources")
       .SideEffect()
       .Prepare([&](GHI::RenderGraphPrepareContext& p_context) {
          Expect(p_context.GetInputCount() == 0u, "Pass-local transient prepare should not expose inputs");
          Expect(p_context.GetOutputCount() == 0u, "Pass-local transient prepare should not expose outputs");
          Expect(p_context.GetTransientCount() == 0u,
                 "Pass-local transient list should start empty before resources are created");

          scratchImage = p_context.CreateTransientImage("scratch image", CreateColorImageDescriptor(),
                                                       GHI::ResourceUsage::StorageReadWrite,
                                                       GHI::ShaderStageFlag::Compute);
          scratchBuffer = p_context.CreateTransientBuffer("scratch buffer", CreateStorageBufferDescriptor(),
                                                         GHI::ResourceUsage::StorageReadWrite,
                                                         GHI::ShaderStageFlag::Compute);

          Expect(p_context.GetTransientCount() == 2u,
                 "Prepare-created pass-local resources should be exposed as transients");
          Expect(p_context.GetTransient(0u).m_index == scratchImage.m_index,
                 "First pass-local transient handle is wrong");
          Expect(p_context.GetTransient(1u).m_index == scratchBuffer.m_index,
                 "Second pass-local transient handle is wrong");
          Expect(p_context.GetImageDescriptor(scratchImage) != nullptr,
                 "Pass-local image should expose its descriptor");
          Expect(p_context.GetBufferDescriptor(scratchBuffer) != nullptr,
                 "Pass-local buffer should expose its descriptor");
          Expect(p_context.GetImageView(scratchImage) == scratchImageView,
                 "Pass-local image should be materialized during prepare");
          Expect(p_context.GetBufferView(scratchBuffer) == scratchBufferView,
                 "Pass-local buffer should be materialized during prepare");
          events.push_back("prepare scratch");
       })
       .Execute([&](GHI::RenderGraphContext& p_context) {
          Expect(p_context.GetInputCount() == 0u, "Pass-local transient execute should not expose inputs");
          Expect(p_context.GetOutputCount() == 0u, "Pass-local transient execute should not expose outputs");
          Expect(p_context.GetTransientCount() == 2u,
                 "Pass-local transient execute should expose prepare-created resources");
          Expect(p_context.GetTransient(0u).m_index == scratchImage.m_index,
                 "Execute first pass-local transient handle is wrong");
          Expect(p_context.GetTransient(1u).m_index == scratchBuffer.m_index,
                 "Execute second pass-local transient handle is wrong");
          Expect(p_context.GetImageView(scratchImage) == scratchImageView,
                 "Execute context did not see the pass-local ImageView");
          Expect(p_context.GetBufferView(scratchBuffer) == scratchBufferView,
                 "Execute context did not see the pass-local BufferView");
          events.push_back("execute scratch");
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect((events == std::vector<std::string>{"prepare scratch", "execute scratch"}),
          "Pass-local transient prepare/execute order is wrong");
   Expect(graph.CanResourceBeTransient(scratchImage),
          "Pass-local image should be marked transient-capable");
   Expect(graph.CanResourceBeTransient(scratchBuffer),
          "Pass-local buffer should be marked transient-capable");
   Expect(graph.GetResourceFirstUseOrder(scratchImage) == 0u,
          "Pass-local image should first be used by its owning pass");
   Expect(graph.GetResourceLastUseOrder(scratchImage) == 0u,
          "Pass-local image should last be used by its owning pass");
   Expect(graph.GetResourceFirstUseOrder(scratchBuffer) == 0u,
          "Pass-local buffer should first be used by its owning pass");
   Expect(graph.GetResourceLastUseOrder(scratchBuffer) == 0u,
          "Pass-local buffer should last be used by its owning pass");

   const std::span<const GHI::RenderCommand> commands = commandBuffer.GetRenderCommands();
   Expect(commands.size() == 2u, "Pass-local transient resources should emit two barriers");

   const std::vector<GHI::PipelineImageBarrier>& imageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commands, 0u));
   const std::vector<GHI::PipelineBufferBarrier>& bufferBarriers =
       GHI::RenderCommandAccess::GetBufferBarriers(GetBarrier(commands, 1u));
   Expect(imageBarriers.size() == 1u, "Pass-local image should emit one image barrier");
   Expect(imageBarriers[0].m_imageView == scratchImageView,
          "Pass-local image barrier references the wrong ImageView");
   Expect(bufferBarriers.size() == 1u, "Pass-local buffer should emit one buffer barrier");
   Expect(bufferBarriers[0].m_bufferView == scratchBufferView,
          "Pass-local buffer barrier references the wrong BufferView");
}

void TestBufferBarrier()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::BufferView> bufferView = CreateBufferView(device);

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   const GHI::RenderGraphResourceHandle vertexBuffer =
       graph.ImportBufferView("vertex buffer", bufferView, GHI::ResourceUsage::HostWrite);

   graph.AddPass("vertex read")
       .Read(vertexBuffer, GHI::ResourceUsage::VertexRead)
       .Execute([]([[maybe_unused]] GHI::RenderGraphContext& p_context) {});

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   const std::span<const GHI::RenderCommand> commands = commandBuffer.GetRenderCommands();
   Expect(commands.size() == 1u, "RenderGraph should have emitted one buffer barrier");

   const GHI::PipelineBarrierCommand& barrier = GetBarrier(commands, 0u);
   const std::vector<GHI::PipelineBufferBarrier>& bufferBarriers =
       GHI::RenderCommandAccess::GetBufferBarriers(barrier);
   Expect(bufferBarriers.size() == 1u, "Barrier should contain one buffer barrier");
   Expect(bufferBarriers[0].m_bufferView == bufferView, "Buffer barrier references the wrong BufferView");
   Expect(bufferBarriers[0].m_srcAccessMask == GHI::AccessFlags::HostWrite,
          "Buffer barrier should start from host writes");
   Expect(bufferBarriers[0].m_dstAccessMask == GHI::AccessFlags::VertexAttributeRead,
          "Buffer barrier should transition to vertex reads");
   Expect(bufferBarriers[0].m_dstStageMask == GHI::PipelineStageFlags::VertexInput,
          "Buffer barrier should target vertex input");
}

void TestContextResourceLookupAndNoBarrierForUnchangedState()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> imageView = CreateImageView(device);

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   const GHI::RenderGraphResourceHandle sampled =
       graph.ImportImageView("sampled", imageView, GHI::ResourceUsage::SampledRead,
                             GHI::ShaderStageFlag::Fragment);

   uint32_t callbackCount = 0u;
   graph.AddPass("sample a")
       .Read(sampled, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([&](GHI::RenderGraphContext& p_context) {
          Expect(p_context.GetImageView(sampled) == imageView, "RenderGraphContext returned the wrong ImageView");
          ++callbackCount;
       });

   graph.AddPass("sample b")
       .Read(sampled, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([&]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          ++callbackCount;
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect(callbackCount == 2u, "RenderGraph did not execute both read-only passes");
   Expect(commandBuffer.GetRenderCommands().empty(),
          "RenderGraph should not emit barriers when the resource state is unchanged");
}

} // namespace

void RunRenderGraphTests()
{
   TestImageBarrierSequenceAndExecutionOrder();
   TestPassCanProduceMultipleOutputs();
   TestZeroOutputSideEffectPassExecutes();
   TestReadWriteBuildersCreateInputAndOutput();
   TestPrepareMaterializesDescriptorResourceAndDetectsTransient();
   TestPrepareCreatesPassLocalTransientResources();
   TestBufferBarrier();
   TestContextResourceLookupAndNoBarrierForUnchangedState();

   std::cout << "RenderGraph tests passed\n";
}
