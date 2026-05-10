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
#include <GHI/SubCommandRecorder.h>

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

class TestSubCommandRecorder final : public GHI::SubCommandRecorder
{
 public:
   TestSubCommandRecorder() = default;
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

   TestImageView(Ptr<GHI::Device> p_device, GHI::ImageViewDescriptor&& p_desc)
       : GHI::ImageView(std::move(p_device), std::move(p_desc))
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

   TestBufferView(Ptr<GHI::Device> p_device, GHI::BufferViewDescriptor&& p_desc)
       : GHI::BufferView(std::move(p_device), std::move(p_desc))
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
   GHI::RenderGraphPass& writePass = graph.AddPass("write color");
   auto [writtenColor] =
       writePass.Write(color, GHI::ResourceUsage::ColorAttachmentWrite);
   Expect(writtenColor.m_index != color.m_index, "Write should return a new logical resource version");
   Expect(graph.GetImageView(writtenColor) == imageView,
          "Written logical resource version should reference the same physical ImageView");
   writePass.Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
      executionLog.push_back("write");
   });

   graph.AddPass("sample color")
       .Read(writtenColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([&executionLog]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          executionLog.push_back("sample");
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
                               &materializedImageIndex]([[maybe_unused]] const GHI::ImageDescriptor& p_desc,
                                                        [[maybe_unused]] bool p_canBeTransient) {
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

void TestZeroOutputNeverCullPassExecutes()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);

   uint32_t callbackCount = 0u;
   graph.AddPass("debug marker")
       .NeverCull()
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
   graph.SetImageMaterializer([device, &imageView]([[maybe_unused]] const GHI::ImageDescriptor& p_desc,
                                                   [[maybe_unused]] bool p_canBeTransient) {
      imageView = CreateImageView(device);
      return imageView;
   });
   graph.SetBufferMaterializer([device, &bufferView]([[maybe_unused]] const GHI::BufferDescriptor& p_desc,
                                                     [[maybe_unused]] bool p_canBeTransient) {
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
          Expect(p_context.GetInput(0u).m_index != storageImage.m_index,
                 "ReadWrite image input should be the previous logical version");
          Expect(p_context.GetOutput(0u).m_index == storageImage.m_index,
                 "ReadWrite image output handle is wrong");
          Expect(p_context.GetImageDescriptor(p_context.GetInput(0u)) != nullptr,
                 "ReadWrite image input should expose the shared ImageDescriptor");
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
          Expect(p_context.GetInput(0u).m_index != storageBuffer.m_index,
                 "ReadWrite buffer input should be the previous logical version");
          Expect(p_context.GetOutput(0u).m_index == storageBuffer.m_index,
                 "ReadWrite buffer output handle is wrong");
          Expect(p_context.GetBufferDescriptor(p_context.GetInput(0u)) != nullptr,
                 "ReadWrite buffer input should expose the shared BufferDescriptor");
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

void TestNamedPassResourceLookupKeepsStructuredOutputs()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> sourceView = CreateImageView(device);
   Ptr<GHI::ImageView> blurredView;
   Ptr<GHI::BufferView> scratchView;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageMaterializer([device, &blurredView]([[maybe_unused]] const GHI::ImageDescriptor& p_desc,
                                                     [[maybe_unused]] bool p_canBeTransient) {
      blurredView = CreateImageView(device);
      return blurredView;
   });
   graph.SetBufferMaterializer([device, &scratchView]([[maybe_unused]] const GHI::BufferDescriptor& p_desc,
                                                      [[maybe_unused]] bool p_canBeTransient) {
      scratchView = CreateBufferView(device);
      return scratchView;
   });

   const GHI::RenderGraphResourceHandle source =
       graph.ImportImageView("source image", sourceView, GHI::ResourceUsage::SampledRead,
                             GHI::ShaderStageFlag::Fragment);

   std::vector<std::string> events;
   auto blur =
       graph.AddPass("named blur")
           .Read("source", source, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
           .WriteImage("blurred", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite)
           .ReadWriteBuffer("scratch buffer", CreateStorageBufferDescriptor(),
                            GHI::ResourceUsage::StorageReadWrite, GHI::ShaderStageFlag::Compute)
       .Prepare([source, &blurredView, &scratchView, &events](GHI::RenderGraphPrepareContext& p_context) {
          Expect(p_context.GetInputCount() == 2u,
                 "Named pass should expose the read input and read-write input");
          Expect(p_context.GetOutputCount() == 2u,
                 "Named pass should expose the write output and read-write output");
          Expect(p_context.Input("source").m_index == source.m_index,
                 "Prepare named input lookup returned the wrong source");
          Expect(p_context.Output("blurred").m_index == p_context.GetOutput(0u).m_index,
                 "Prepare named output lookup returned the wrong image");
          Expect(p_context.Input("scratch buffer").m_index == p_context.GetInput(1u).m_index,
                 "Prepare named read-write input lookup returned the wrong previous version");
          Expect(p_context.Output("scratch buffer").m_index == p_context.GetOutput(1u).m_index,
                 "Prepare named read-write output lookup returned the wrong produced version");
          Expect(p_context.Input("scratch buffer").m_index != p_context.Output("scratch buffer").m_index,
                 "Named ReadWrite should distinguish input and output versions");
          Expect(p_context.GetImageView(p_context.Output("blurred")) == blurredView,
                 "Named image output should resolve to the materialized ImageView");
          Expect(p_context.GetBufferView(p_context.Output("scratch buffer")) == scratchView,
                 "Named buffer output should resolve to the materialized BufferView");
          events.push_back("prepare blur");
       })
       .Execute([source, &blurredView, &scratchView, &events](GHI::RenderGraphContext& p_context) {
          Expect(p_context.Input("source").m_index == source.m_index,
                 "Execute named input lookup returned the wrong source");
          Expect(p_context.Output("blurred").m_index == p_context.GetOutput(0u).m_index,
                 "Execute named output lookup returned the wrong image");
          Expect(p_context.Input("scratch buffer").m_index == p_context.GetInput(1u).m_index,
                 "Execute named read-write input lookup returned the wrong previous version");
          Expect(p_context.Output("scratch buffer").m_index == p_context.GetOutput(1u).m_index,
                 "Execute named read-write output lookup returned the wrong produced version");
          Expect(p_context.GetImageView(p_context.Output("blurred")) == blurredView,
                 "Execute named image output should resolve to the materialized ImageView");
          Expect(p_context.GetBufferView(p_context.Output("scratch buffer")) == scratchView,
                 "Execute named buffer output should resolve to the materialized BufferView");
          events.push_back("execute blur");
       });

   auto [blurred, scratchBuffer] = blur;
   Expect(blur.Input("source").m_index == source.m_index,
          "Output list should preserve named inputs for pass-local lookup");
   Expect(blur.Output("blurred").m_index == blurred.m_index,
          "Output list should map the image name to the structured output");
   Expect(blur.Output("scratch buffer").m_index == scratchBuffer.m_index,
          "Output list should map the buffer name to the structured output");

   graph.AddPass("consume named output")
       .Read(blurred, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Read(scratchBuffer, GHI::ResourceUsage::VertexRead)
       .Execute([&events]([[maybe_unused]] GHI::RenderGraphContext& p_context) {
          events.push_back("consume blur");
       });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect((events == std::vector<std::string>{"prepare blur", "execute blur", "consume blur"}),
          "Named pass resource lookup did not preserve solved execution order");
}

void TestPrepareMaterializesDescriptorResourceAndDetectsTransient()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> materializedView;
   bool materializerSawTransient = false;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageMaterializer([device, &materializedView, &materializerSawTransient](
                                  [[maybe_unused]] const GHI::ImageDescriptor& p_desc, bool p_canBeTransient) {
      materializerSawTransient = p_canBeTransient;
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
   Expect(materializerSawTransient,
          "Descriptor-backed image materializer should know transient eligibility before creation");
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
   bool scratchImageMaterializerSawTransient = false;
   bool scratchBufferMaterializerSawTransient = false;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageMaterializer([device, &scratchImageView, &scratchImageMaterializerSawTransient](
                                  [[maybe_unused]] const GHI::ImageDescriptor& p_desc, bool p_canBeTransient) {
      scratchImageMaterializerSawTransient = p_canBeTransient;
      scratchImageView = CreateImageView(device);
      return scratchImageView;
   });
   graph.SetBufferMaterializer([device, &scratchBufferView, &scratchBufferMaterializerSawTransient](
                                  [[maybe_unused]] const GHI::BufferDescriptor& p_desc, bool p_canBeTransient) {
      scratchBufferMaterializerSawTransient = p_canBeTransient;
      scratchBufferView = CreateBufferView(device);
      return scratchBufferView;
   });

   std::vector<std::string> events;
   graph.AddPass("scratch resources")
       .NeverCull()
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
   Expect(scratchImageMaterializerSawTransient,
          "Pass-local image materializer should know transient eligibility before creation");
   Expect(scratchBufferMaterializerSawTransient,
          "Pass-local buffer materializer should know transient eligibility before creation");
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

void TestTransientAliasGroupsUseSolvedLifetimes()
{
   GHI::RenderGraph graph;

   auto [firstColor] =
       graph.AddPass("first color write")
           .WriteImage("first color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.AddPass("first color read")
       .Read(firstColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [secondColor] =
       graph.AddPass("second color write")
           .WriteImage("second color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.AddPass("second color read")
       .Read(secondColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [overlappingColor] =
       graph.AddPass("overlapping color write")
           .WriteImage("overlapping color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.AddPass("overlapping color read")
       .Read(secondColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Read(overlappingColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   graph.Compile();

   Expect(graph.CanResourceBeTransient(firstColor),
          "First graph-created color should be transient-capable");
   Expect(graph.CanResourceBeTransient(secondColor),
          "Second graph-created color should be transient-capable");
   Expect(graph.CanResourceLifetimesAlias(firstColor, secondColor),
          "Non-overlapping transient lifetimes should be alias candidates");
   Expect(!graph.CanResourceLifetimesAlias(secondColor, overlappingColor),
          "Overlapping transient lifetimes should not alias");

   const std::vector<GHI::RenderGraphTransientResourceInfo>& transientResources =
       graph.GetTransientResources();
   Expect(transientResources.size() == 3u,
          "RenderGraph should expose all transient-capable storage resources");
   Expect(transientResources[0].m_handle.m_index == firstColor.m_index,
          "Transient resources should be sorted by first use");
   Expect(transientResources[0].m_allocationSize > 0u,
          "Transient resources should expose an allocation size");
   Expect(transientResources[1].m_handle.m_index == secondColor.m_index,
          "Second transient resource order is wrong");
   Expect(transientResources[2].m_handle.m_index == overlappingColor.m_index,
          "Third transient resource order is wrong");

   const std::vector<GHI::RenderGraphTransientAliasGroup>& aliasGroups =
       graph.GetTransientAliasGroups();
   Expect(aliasGroups.size() == 2u,
          "RenderGraph should pack lifetime-compatible resources into two alias groups");

   bool firstAndSecondShareGroup = false;
   bool secondAndOverlappingShareGroup = false;
   for (const GHI::RenderGraphTransientAliasGroup& group : aliasGroups)
   {
      bool hasFirstColor = false;
      bool hasSecondColor = false;
      bool hasOverlappingColor = false;
      for (const GHI::RenderGraphResourceHandle resource : group.m_resources)
      {
         hasFirstColor = hasFirstColor || resource.m_index == firstColor.m_index;
         hasSecondColor = hasSecondColor || resource.m_index == secondColor.m_index;
         hasOverlappingColor = hasOverlappingColor || resource.m_index == overlappingColor.m_index;
      }

      firstAndSecondShareGroup = firstAndSecondShareGroup || (hasFirstColor && hasSecondColor);
      secondAndOverlappingShareGroup = secondAndOverlappingShareGroup || (hasSecondColor && hasOverlappingColor);
   }

   Expect(firstAndSecondShareGroup,
          "RenderGraph should place the closest non-overlapping resources in one alias group");
   Expect(!secondAndOverlappingShareGroup,
          "Overlapping resources should not be placed in the same alias group");
   Expect(graph.GetResourceFirstUseOrder(firstColor) == 0u,
          "First color should first be used by the first solved pass");
   Expect(graph.GetResourceLastUseOrder(firstColor) == 1u,
          "First color should last be used by the second solved pass");
   Expect(graph.GetResourceFirstUseOrder(secondColor) == 2u,
          "Second color should first be used after first color is dead");
   Expect(graph.GetResourceLastUseOrder(secondColor) == 5u,
          "Second color should include its later overlapping read");
}

void TestTransientAliasGroupsRespectCompatibility()
{
   GHI::RenderGraph defaultGraph;

   auto [color] =
       defaultGraph.AddPass("color write")
           .WriteImage("color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   defaultGraph.AddPass("color read")
       .Read(color, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [buffer] =
       defaultGraph.AddPass("buffer write")
           .WriteBuffer("buffer", CreateStorageBufferDescriptor(),
                        GHI::ResourceUsage::StorageWrite, GHI::ShaderStageFlag::Compute);
   defaultGraph.AddPass("buffer read")
       .Read(buffer, GHI::ResourceUsage::StorageRead, GHI::ShaderStageFlag::Compute);

   defaultGraph.Compile();

   Expect(defaultGraph.CanResourceLifetimesAlias(color, buffer),
          "Image and buffer lifetimes should be non-overlapping");
   Expect(!defaultGraph.CanResourcesShareTransientAllocation(color, buffer),
          "Default transient compatibility should keep different resource types separate");
   Expect(defaultGraph.GetTransientAliasGroups().size() == 2u,
          "Default compatibility should place image and buffer resources in separate alias groups");

   GHI::RenderGraph customGraph;
   auto [firstColor] =
       customGraph.AddPass("first color write")
           .WriteImage("first color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   customGraph.AddPass("first color read")
       .Read(firstColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [secondColor] =
       customGraph.AddPass("second color write")
           .WriteImage("second color", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   customGraph.AddPass("second color read")
       .Read(secondColor, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   bool checkedFirstSecondPair = false;
   customGraph.SetTransientCompatibilityChecker(
       [firstColor, secondColor, &checkedFirstSecondPair](GHI::RenderGraphResourceHandle p_first,
                                                          GHI::RenderGraphResourceHandle p_second) {
          const bool isFirstSecondPair =
              (p_first.m_index == firstColor.m_index && p_second.m_index == secondColor.m_index) ||
              (p_first.m_index == secondColor.m_index && p_second.m_index == firstColor.m_index);
          checkedFirstSecondPair = checkedFirstSecondPair || isFirstSecondPair;
          return !isFirstSecondPair;
       });

   customGraph.Compile();

   Expect(customGraph.CanResourceLifetimesAlias(firstColor, secondColor),
          "Custom compatibility test resources should be lifetime-compatible");
   Expect(!customGraph.CanResourcesShareTransientAllocation(firstColor, secondColor),
          "Custom compatibility checker should reject the lifetime-compatible pair");
   Expect(checkedFirstSecondPair,
          "RenderGraph did not consult the transient compatibility checker");
   Expect(customGraph.GetTransientAliasGroups().size() == 2u,
          "Rejected resources should be placed in separate alias groups");
}

void TestTransientAliasGroupsUseBestFitSize()
{
   GHI::RenderGraph graph;

   auto [smallEarly] =
       graph.AddPass("small early write")
           .WriteImage("small early", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   auto [largeEarly] =
       graph.AddPass("large early write")
           .WriteImage("large early", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.AddPass("small late read")
       .Read(smallEarly, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [largeLate] =
       graph.AddPass("large late write")
           .WriteImage("large late", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.SetTransientAllocationSizeResolver(
       [smallEarly, largeEarly, largeLate](GHI::RenderGraphResourceHandle p_handle) -> uint64_t {
          if (p_handle.m_index == smallEarly.m_index)
          {
             return 10u;
          }
          if (p_handle.m_index == largeEarly.m_index)
          {
             return 100u;
          }
          if (p_handle.m_index == largeLate.m_index)
          {
             return 90u;
          }
          return 1u;
       });

   graph.Compile();

   const std::vector<GHI::RenderGraphTransientAliasGroup>& aliasGroups =
       graph.GetTransientAliasGroups();
   Expect(aliasGroups.size() == 2u,
          "Best-fit transient scheduling should use two alias groups for the overlapping setup");

   bool largeResourcesShareGroup = false;
   bool smallAndLargeLateShareGroup = false;
   for (const GHI::RenderGraphTransientAliasGroup& group : aliasGroups)
   {
      bool hasSmallEarly = false;
      bool hasLargeEarly = false;
      bool hasLargeLate = false;
      for (const GHI::RenderGraphResourceHandle resource : group.m_resources)
      {
         hasSmallEarly = hasSmallEarly || resource.m_index == smallEarly.m_index;
         hasLargeEarly = hasLargeEarly || resource.m_index == largeEarly.m_index;
         hasLargeLate = hasLargeLate || resource.m_index == largeLate.m_index;
      }

      largeResourcesShareGroup = largeResourcesShareGroup || (hasLargeEarly && hasLargeLate);
      smallAndLargeLateShareGroup = smallAndLargeLateShareGroup || (hasSmallEarly && hasLargeLate);
      if (hasLargeEarly && hasLargeLate)
      {
         Expect(group.m_allocationSize == 100u,
                "Best-fit group should keep the larger existing allocation size");
         Expect(group.m_lastUseOrder == graph.GetResourceLastUseOrder(largeLate),
                "Best-fit group should extend to the appended resource lifetime");
      }
   }

   Expect(largeResourcesShareGroup,
          "Best-fit scheduler should choose the closest-size compatible alias slot");
   Expect(!smallAndLargeLateShareGroup,
          "Best-fit scheduler should not choose a smaller slot with higher added cost");
}

void TestTransientAliasGroupsUseBackendCompatibilityBeforeSize()
{
   GHI::RenderGraph graph;

   auto [compatibleEarly] =
       graph.AddPass("compatible early write")
           .WriteImage("compatible early", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   auto [incompatibleEarly] =
       graph.AddPass("incompatible early write")
           .WriteImage("incompatible early", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.AddPass("compatible late read")
       .Read(compatibleEarly, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [late] =
       graph.AddPass("late write")
           .WriteImage("late", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   graph.SetTransientAllocationSizeResolver(
       [compatibleEarly, incompatibleEarly, late](GHI::RenderGraphResourceHandle p_handle) -> uint64_t {
          if (p_handle.m_index == compatibleEarly.m_index)
          {
             return 128u;
          }
          if (p_handle.m_index == incompatibleEarly.m_index)
          {
             return 90u;
          }
          if (p_handle.m_index == late.m_index)
          {
             return 85u;
          }
          return 1u;
       });
   graph.SetTransientCompatibilityChecker(
       [incompatibleEarly, late](GHI::RenderGraphResourceHandle p_first,
                                 GHI::RenderGraphResourceHandle p_second) {
          const bool isRejectedPair =
              (p_first.m_index == incompatibleEarly.m_index && p_second.m_index == late.m_index) ||
              (p_first.m_index == late.m_index && p_second.m_index == incompatibleEarly.m_index);
          return !isRejectedPair;
       });

   graph.Compile();

   const std::vector<GHI::RenderGraphTransientAliasGroup>& aliasGroups =
       graph.GetTransientAliasGroups();
   Expect(aliasGroups.size() == 2u,
          "Backend compatibility should keep the rejected resource in a separate alias group");

   bool compatibleAndLateShareGroup = false;
   bool incompatibleAndLateShareGroup = false;
   for (const GHI::RenderGraphTransientAliasGroup& group : aliasGroups)
   {
      bool hasCompatibleEarly = false;
      bool hasIncompatibleEarly = false;
      bool hasLate = false;
      for (const GHI::RenderGraphResourceHandle resource : group.m_resources)
      {
         hasCompatibleEarly = hasCompatibleEarly || resource.m_index == compatibleEarly.m_index;
         hasIncompatibleEarly = hasIncompatibleEarly || resource.m_index == incompatibleEarly.m_index;
         hasLate = hasLate || resource.m_index == late.m_index;
      }

      compatibleAndLateShareGroup = compatibleAndLateShareGroup || (hasCompatibleEarly && hasLate);
      incompatibleAndLateShareGroup = incompatibleAndLateShareGroup || (hasIncompatibleEarly && hasLate);
      if (hasCompatibleEarly && hasLate)
      {
         Expect(group.m_allocationSize == 128u,
                "Compatible fallback group should keep the larger allocation size");
      }
   }

   Expect(compatibleAndLateShareGroup,
          "Scheduler should choose the compatible alias slot even when it is a worse size fit");
   Expect(!incompatibleAndLateShareGroup,
          "Scheduler should reject the closest-size alias slot when backend compatibility fails");
}

void TestTransientAliasGroupsRespectFullGroupCompatibility()
{
   GHI::RenderGraph graph;

   auto [first] =
       graph.AddPass("first write")
           .WriteImage("first", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   auto [second] =
       graph.AddPass("second write")
           .WriteImage("second", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   auto [third] =
       graph.AddPass("third write")
           .WriteImage("third", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);

   bool checkedFullGroup = false;
   graph.SetTransientAliasGroupCompatibilityChecker(
       [first, second, third, &checkedFullGroup](
           const std::vector<GHI::RenderGraphResourceHandle>& p_groupResources,
           GHI::RenderGraphResourceHandle p_candidate) {
          bool hasFirst = false;
          bool hasSecond = false;
          for (const GHI::RenderGraphResourceHandle resource : p_groupResources)
          {
             hasFirst = hasFirst || resource.m_index == first.m_index;
             hasSecond = hasSecond || resource.m_index == second.m_index;
          }

          const bool rejectsFullGroup = hasFirst && hasSecond && p_candidate.m_index == third.m_index;
          checkedFullGroup = checkedFullGroup || rejectsFullGroup;
          return !rejectsFullGroup;
       });

   graph.Compile();

   const std::vector<GHI::RenderGraphTransientAliasGroup>& aliasGroups =
       graph.GetTransientAliasGroups();
   Expect(checkedFullGroup,
          "Scheduler should ask whether the candidate can join the whole alias group");
   Expect(aliasGroups.size() == 2u,
          "Full-group compatibility should reject the third resource from the first alias group");

   bool firstAndSecondShareGroup = false;
   bool allThreeShareGroup = false;
   bool thirdIsStandalone = false;
   for (const GHI::RenderGraphTransientAliasGroup& group : aliasGroups)
   {
      bool hasFirst = false;
      bool hasSecond = false;
      bool hasThird = false;
      for (const GHI::RenderGraphResourceHandle resource : group.m_resources)
      {
         hasFirst = hasFirst || resource.m_index == first.m_index;
         hasSecond = hasSecond || resource.m_index == second.m_index;
         hasThird = hasThird || resource.m_index == third.m_index;
      }

      firstAndSecondShareGroup = firstAndSecondShareGroup || (hasFirst && hasSecond);
      allThreeShareGroup = allThreeShareGroup || (hasFirst && hasSecond && hasThird);
      thirdIsStandalone = thirdIsStandalone || (hasThird && group.m_resources.size() == 1u);
   }

   Expect(firstAndSecondShareGroup,
          "First two resources should still share the compatible alias group");
   Expect(!allThreeShareGroup,
          "Rejected full group should not contain all three resources");
   Expect(thirdIsStandalone,
          "Rejected candidate should be placed into its own alias group");
}

void TestTransientMaterializerConsumesAliasGroups()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::Image> firstImage;
   Ptr<GHI::Image> secondImage;

   GHI::RenderGraph graph;
   InstallTestBarrierEmitter(graph);
   graph.SetImageViewMaterializer([](Ptr<GHI::Image> p_image, const GHI::ImageDescriptor& p_desc) {
      Ptr<GHI::Device> device = p_image->GetDevice();
      return std::make_shared<TestImageView>(
          std::move(device), GHI::CreateDefaultRenderGraphImageViewDescriptor(std::move(p_image), p_desc));
   });

   auto [first] =
       graph.AddPass("first write")
           .WriteImage("first", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   graph.AddPass("first read")
       .Read(first, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   auto [second] =
       graph.AddPass("second write")
           .WriteImage("second", CreateColorImageDescriptor(),
                       GHI::ResourceUsage::ColorAttachmentWrite);
   graph.AddPass("second read")
       .Read(second, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment);

   bool sawAliasGroup = false;
   graph.SetTransientMaterializer([&](std::span<const GHI::RenderGraphTransientAliasGroupRequest> p_groups,
                                      GHI::RenderGraphTransientResourceWriter& p_writer) {
      for (const GHI::RenderGraphTransientAliasGroupRequest& group : p_groups)
      {
         bool hasFirst = false;
         bool hasSecond = false;
         for (const GHI::RenderGraphTransientResourceRequest& resource : group.m_resources)
         {
            hasFirst = hasFirst || resource.m_handle.m_index == first.m_index;
            hasSecond = hasSecond || resource.m_handle.m_index == second.m_index;
         }

         if (!hasFirst || !hasSecond)
         {
            continue;
         }

         sawAliasGroup = true;
         Expect(group.m_resources.size() == 2u,
                "Transient materializer should see the solved alias group");

         for (const GHI::RenderGraphTransientResourceRequest& resource : group.m_resources)
         {
            Expect(resource.m_type == GHI::RenderGraphResourceType::Image,
                   "Transient materializer request should describe an image");
            Expect(resource.m_imageDesc != nullptr,
                   "Transient materializer should expose the image descriptor");

            if (resource.m_handle.m_index == first.m_index)
            {
               firstImage = std::make_shared<TestImage>(device);
               p_writer.SetImage(resource.m_handle, firstImage);
            }
            else if (resource.m_handle.m_index == second.m_index)
            {
               secondImage = std::make_shared<TestImage>(device);
               p_writer.SetImage(resource.m_handle, secondImage);
            }
         }
      }
   });

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect(sawAliasGroup, "Transient materializer did not receive the alias group");
   Expect(graph.GetImageView(first) != nullptr,
          "Transient materializer did not assign the first ImageView");
   Expect(graph.GetImageView(second) != nullptr,
          "Transient materializer did not assign the second ImageView");
   Expect(graph.GetImageView(first)->GetImage().get() == firstImage.get(),
          "Transient writer did not wrap the first Image");
   Expect(graph.GetImageView(second)->GetImage().get() == secondImage.get(),
          "Transient writer did not wrap the second Image");
}

void TestBarrierInfoCarriesQueueFamilyOwnership()
{
   Ptr<GHI::Device> device = std::make_shared<TestDevice>();
   Ptr<GHI::ImageView> imageView = CreateImageView(device);

   GHI::RenderGraph graph;
   graph.SetQueueFamilyResolver([](GHI::QueueFamilyType p_queueType) {
      switch (p_queueType)
      {
      case GHI::QueueFamilyType::GraphicsQueue:
         return GHI::QueueFamilyInfo{.m_queueType = p_queueType,
                                     .m_supportedQueues = GHI::QueueTypeFlags::GraphicsQueue,
                                     .m_familyIndex = 0u,
                                     .m_queueIndex = 0u};
      case GHI::QueueFamilyType::ComputeQueue:
         return GHI::QueueFamilyInfo{.m_queueType = p_queueType,
                                     .m_supportedQueues = GHI::QueueTypeFlags::ComputeQueue,
                                     .m_familyIndex = 1u,
                                     .m_queueIndex = 0u};
      default:
         return GHI::QueueFamilyInfo{};
      }
   });

   bool sawOwnershipTransfer = false;
   graph.SetBarrierEmitter([&sawOwnershipTransfer](GHI::CommandBuffer& p_commandBuffer,
                                                   const GHI::RenderGraphBarrierInfo& p_barrierInfo) {
      Expect(p_barrierInfo.m_oldQueue == GHI::QueueFamilyType::ComputeQueue,
             "Barrier should report the imported resource's old queue");
      Expect(p_barrierInfo.m_newQueue == GHI::QueueFamilyType::GraphicsQueue,
             "Barrier should report the consuming pass queue");
      Expect(p_barrierInfo.RequiresQueueFamilyOwnershipTransfer(),
             "Barrier should report a queue-family ownership transfer");
      sawOwnershipTransfer = true;

      const GHI::ResourceUsageInfo oldInfo =
          GHI::ResourceUsageToInfo(p_barrierInfo.m_oldUsage, p_barrierInfo.m_oldShaderStages);
      const GHI::ResourceUsageInfo newInfo =
          GHI::ResourceUsageToInfo(p_barrierInfo.m_newUsage, p_barrierInfo.m_newShaderStages);
      p_commandBuffer.PipelineBarrier()->AddImageBarrier(
          oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages, newInfo.m_access,
          oldInfo.m_imageLayout, newInfo.m_imageLayout, p_barrierInfo.m_oldQueueFamily.m_familyIndex,
          p_barrierInfo.m_newQueueFamily.m_familyIndex, p_barrierInfo.m_imageView);
   });

   const GHI::RenderGraphResourceHandle computeOutput =
       graph.ImportImageView("compute output", imageView, GHI::ResourceUsage::StorageWrite,
                             GHI::ShaderStageFlag::Compute, GHI::QueueFamilyType::ComputeQueue);

   graph.AddPass("graphics sample")
       .Queue(GHI::QueueFamilyType::GraphicsQueue)
       .Read(computeOutput, GHI::ResourceUsage::SampledRead, GHI::ShaderStageFlag::Fragment)
       .Execute([]([[maybe_unused]] GHI::RenderGraphContext& p_context) {});

   TestCommandBuffer commandBuffer(device);
   graph.Execute(commandBuffer);

   Expect(sawOwnershipTransfer, "RenderGraph did not emit queue-family ownership information");
   const std::vector<GHI::PipelineImageBarrier>& imageBarriers =
       GHI::RenderCommandAccess::GetImageBarriers(GetBarrier(commandBuffer.GetRenderCommands(), 0u));
   Expect(imageBarriers[0].m_srcQueueFamilyIndex == 1u,
          "Queue-family source index was not forwarded to the barrier");
   Expect(imageBarriers[0].m_dstQueueFamilyIndex == 0u,
          "Queue-family destination index was not forwarded to the barrier");
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

void TestMeshShaderStageUsageInfo()
{
   const GHI::ResourceUsageInfo usageInfo =
       GHI::ResourceUsageToInfo(GHI::ResourceUsage::StorageRead, GHI::ShaderStageFlag::Mesh);

   Expect(any(usageInfo.m_pipelineStages, GHI::PipelineStageFlags::MeshShader),
          "Mesh shader resource usage should target the mesh shader pipeline stage");
   Expect(!any(usageInfo.m_pipelineStages, GHI::PipelineStageFlags::VertexShader),
          "Mesh shader resource usage should not target the vertex shader pipeline stage");
   Expect(usageInfo.m_access == GHI::AccessFlags::ShaderRead,
          "Mesh shader storage reads should use shader-read access");
}

void TestTaskShaderStageUsageInfo()
{
   const GHI::ResourceUsageInfo usageInfo =
       GHI::ResourceUsageToInfo(GHI::ResourceUsage::StorageRead, GHI::ShaderStageFlag::Task);

   Expect(any(usageInfo.m_pipelineStages, GHI::PipelineStageFlags::TaskShader),
          "Task shader resource usage should target the task shader pipeline stage");
   Expect(!any(usageInfo.m_pipelineStages, GHI::PipelineStageFlags::MeshShader),
          "Task shader-only resource usage should not target the mesh shader pipeline stage");
   Expect(usageInfo.m_access == GHI::AccessFlags::ShaderRead,
          "Task shader storage reads should use shader-read access");

   const GHI::PipelineStageFlags combinedStages =
       GHI::ShaderStagesToPipelineStages(GHI::ShaderStageFlag::Task | GHI::ShaderStageFlag::Mesh);
   Expect(any(combinedStages, GHI::PipelineStageFlags::TaskShader),
          "Combined task+mesh shader usage should include the task shader pipeline stage");
   Expect(any(combinedStages, GHI::PipelineStageFlags::MeshShader),
          "Combined task+mesh shader usage should include the mesh shader pipeline stage");
}

void TestDrawMeshTasksCommandRecordsGroupCounts()
{
   TestSubCommandRecorder recorder;
   recorder.DrawMeshTasks(2u, 3u, 4u);

   const std::span<const GHI::RenderCommand> commands = recorder.GetRenderCommands();
   Expect(commands.size() == 1u, "DrawMeshTasks should record one render command");
   Expect(std::holds_alternative<GHI::DrawMeshTasksCommand>(commands[0]),
          "DrawMeshTasks should record a DrawMeshTasksCommand");

   const GHI::DrawMeshTasksCommand& command = std::get<GHI::DrawMeshTasksCommand>(commands[0]);
   Expect(GHI::RenderCommandAccess::GetGroupCountX(command) == 2u, "DrawMeshTasks groupCountX was not recorded");
   Expect(GHI::RenderCommandAccess::GetGroupCountY(command) == 3u, "DrawMeshTasks groupCountY was not recorded");
   Expect(GHI::RenderCommandAccess::GetGroupCountZ(command) == 4u, "DrawMeshTasks groupCountZ was not recorded");
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
   TestZeroOutputNeverCullPassExecutes();
   TestReadWriteBuildersCreateInputAndOutput();
   TestNamedPassResourceLookupKeepsStructuredOutputs();
   TestPrepareMaterializesDescriptorResourceAndDetectsTransient();
   TestPrepareCreatesPassLocalTransientResources();
   TestTransientAliasGroupsUseSolvedLifetimes();
   TestTransientAliasGroupsRespectCompatibility();
   TestTransientAliasGroupsUseBestFitSize();
   TestTransientAliasGroupsUseBackendCompatibilityBeforeSize();
   TestTransientAliasGroupsRespectFullGroupCompatibility();
   TestTransientMaterializerConsumesAliasGroups();
   TestBarrierInfoCarriesQueueFamilyOwnership();
   TestBufferBarrier();
   TestMeshShaderStageUsageInfo();
   TestTaskShaderStageUsageInfo();
   TestDrawMeshTasksCommandRecordsGroupCounts();
   TestContextResourceLookupAndNoBarrierForUnchangedState();

   std::cout << "RenderGraph tests passed\n";
}
