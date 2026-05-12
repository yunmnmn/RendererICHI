#include <GHI/RenderGraph.h>

#include <algorithm>
#include <array>
#include <future>
#include <utility>

#include <Util/Assert.h>

#include <GHI/BufferView.h>
#include <GHI/CommandBuffer.h>
#include <GHI/ImageView.h>
#include <GHI/Query.h>
#include <GHI/QueryPool.h>
#include <GHI/ResourceFactory.h>

namespace Render
{

namespace GHI
{

// Implementation overview:
// 1. Pass builders collect logical resource accesses and primary-stream metadata.
// 2. Compile builds a stable topological order and derives storage lifetimes.
// 3. Prepare materializes graph-owned resources and lets passes create local transients.
// 4. Execute records pass-local subcommands first, then serially assembles the primary command stream.
// The Vulkan/D3D12 split is preserved by keeping backend-specific barriers/materialization behind hooks.

namespace
{

constexpr uint32_t InvalidPass = static_cast<uint32_t>(-1);

class RecordedSubCommandBuffer final : public SubCommandBuffer
{
 public:
   RecordedSubCommandBuffer() : SubCommandBuffer(SubCommandBufferDescriptor{})
   {
   }

 private:
   void ReleaseInternal() final
   {
      // Test fallback: there is no native secondary command buffer to release.
   }
};

// Access classification is kept close to the pass builder so the public API can fail fast when a pass declares
// a read/write shape that does not match the ResourceUsage vocabulary.
bool IsReadOnlyUsage(ResourceUsage p_usage)
{
   return ResourceUsageReads(p_usage) && !ResourceUsageWrites(p_usage);
}

bool IsWriteOnlyUsage(ResourceUsage p_usage)
{
   return ResourceUsageWrites(p_usage) && !ResourceUsageReads(p_usage);
}

bool IsReadWriteUsage(ResourceUsage p_usage)
{
   return ResourceUsageReads(p_usage) && ResourceUsageWrites(p_usage);
}

bool IsColorAttachmentUsage(ResourceUsage p_usage)
{
   return p_usage == ResourceUsage::ColorAttachmentRead || p_usage == ResourceUsage::ColorAttachmentWrite ||
          p_usage == ResourceUsage::ColorAttachmentReadWrite;
}

bool IsDepthStencilAttachmentUsage(ResourceUsage p_usage)
{
   return p_usage == ResourceUsage::DepthStencilRead || p_usage == ResourceUsage::DepthStencilWrite ||
          p_usage == ResourceUsage::DepthStencilReadWrite;
}

QueryReadbackData MakeTimestampReadbackData(const QueryReadbackData& p_begin, const QueryReadbackData& p_end)
{
   // Public timestamp readback exposes a single result object even though the graph stores begin/end queries
   // independently. Keeping both values preserves the raw tick data for caller-side duration conversion.
   QueryReadbackData data;
   data.m_values.reserve(p_begin.m_values.size() + p_end.m_values.size());
   data.m_values.insert(data.m_values.end(), p_begin.m_values.begin(), p_begin.m_values.end());
   data.m_values.insert(data.m_values.end(), p_end.m_values.begin(), p_end.m_values.end());
   return data;
}

uint64_t GetResourceFormatByteSize(ResourceFormat p_format)
{
   // This fallback estimate is intentionally simple; real backends should provide exact memory requirements.
   switch (p_format)
   {
   case ResourceFormat::R4G4UnormPack8:
   case ResourceFormat::R8Unorm:
   case ResourceFormat::R8Snorm:
   case ResourceFormat::R8Scaled:
   case ResourceFormat::R8SScaled:
   case ResourceFormat::R8Uint:
   case ResourceFormat::R8Sint:
   case ResourceFormat::R8Srgb:
   case ResourceFormat::S8Uint:
      return 1u;
   case ResourceFormat::R4G4B4A4UnormPack16:
   case ResourceFormat::B4G4R4A4UnormPack16:
   case ResourceFormat::R5G6B5UnormPack16:
   case ResourceFormat::B5G6R5UnormPack16:
   case ResourceFormat::R5G5B5A1UnormPack16:
   case ResourceFormat::B5G5R5A1UnormPack16:
   case ResourceFormat::A1R5G5B5UnormPack16:
   case ResourceFormat::R8G8Unorm:
   case ResourceFormat::R8G8Snorm:
   case ResourceFormat::R8G8Uscaled:
   case ResourceFormat::R8G8Sscaled:
   case ResourceFormat::R8G8Uint:
   case ResourceFormat::D16Unorm:
   case ResourceFormat::D16UnormS8Uint:
      return 2u;
   case ResourceFormat::R8G8B8A8Unorm:
   case ResourceFormat::B8G8R8A8Srgb:
   case ResourceFormat::R32G32Sfloat:
   case ResourceFormat::X8D24UnormPack32:
   case ResourceFormat::D32Sfloat:
   case ResourceFormat::D24UnormS8Uint:
      return 4u;
   case ResourceFormat::R32G32B32Sfloat:
      return 12u;
   case ResourceFormat::D32SfloatS8Uint:
      return 8u;
   default:
      return 1u;
   }
}

ImageAspectFlags GetDefaultAspectMask(ResourceFormat p_format)
{
   // Full graph-created image views default to the obvious aspect for the image format.
   switch (p_format)
   {
   case ResourceFormat::D16Unorm:
   case ResourceFormat::X8D24UnormPack32:
   case ResourceFormat::D32Sfloat:
      return ImageAspectFlags::Depth;
   case ResourceFormat::S8Uint:
      return ImageAspectFlags::Stencil;
   case ResourceFormat::D16UnormS8Uint:
   case ResourceFormat::D24UnormS8Uint:
   case ResourceFormat::D32SfloatS8Uint:
      return ImageAspectFlags::Depth | ImageAspectFlags::Stencil;
   default:
      return ImageAspectFlags::Color;
   }
}

ImageViewType GetDefaultImageViewType(const ImageDescriptor& p_desc)
{
   // The graph creates full-resource views, so the descriptor's dimensionality maps directly to the view type.
   switch (p_desc.m_imageType)
   {
   case ImageType::Image1D:
      return p_desc.m_arrayLayers > 1u ? ImageViewType::View1DArray : ImageViewType::View1D;
   case ImageType::Image2D:
      return p_desc.m_arrayLayers > 1u ? ImageViewType::View2DArray : ImageViewType::View2D;
   case ImageType::Image3D:
      return ImageViewType::View3D;
   default:
      ASSERT(false, "Unsupported RenderGraph image type for default ImageView");
      return ImageViewType::View2D;
   }
}

BufferUsage GetDefaultBufferViewUsage(BufferUsageFlags p_usageFlags)
{
   // Pick the most specific full-view usage from the buffer creation flags. The order favors descriptor-style
   // usages first because vertex/index/transfer buffers usually do not require typed API buffer views.
   if (any(p_usageFlags, BufferUsageFlags::Storage))
   {
      return BufferUsage::Storage;
   }
   if (any(p_usageFlags, BufferUsageFlags::Uniform))
   {
      return BufferUsage::Uniform;
   }
   if (any(p_usageFlags, BufferUsageFlags::StorageTexel))
   {
      return BufferUsage::StorageTexel;
   }
   if (any(p_usageFlags, BufferUsageFlags::UniformTexel))
   {
      return BufferUsage::UniformTexel;
   }
   if (any(p_usageFlags, BufferUsageFlags::VertexBuffer))
   {
      return BufferUsage::VertexBuffer;
   }
   if (any(p_usageFlags, BufferUsageFlags::IndexBuffer))
   {
      return BufferUsage::IndexBuffer;
   }
   if (any(p_usageFlags, BufferUsageFlags::IndirectBuffer))
   {
      return BufferUsage::IndirectBuffer;
   }
   if (any(p_usageFlags, BufferUsageFlags::TransferDestination))
   {
      return BufferUsage::TransferDestination;
   }
   if (any(p_usageFlags, BufferUsageFlags::TransferSource))
   {
      return BufferUsage::TransferSource;
   }

   ASSERT(false, "Unsupported RenderGraph buffer usage for default BufferView");
   return BufferUsage::Invalid;
}

} // namespace

ImageViewDescriptor CreateDefaultRenderGraphImageViewDescriptor(Ptr<Image> p_image, const ImageDescriptor& p_desc)
{
   // RenderGraph-owned resources expose a full default view. Custom subresource views should be created outside
   // the graph and imported, or added as a later explicit graph feature.
   return ImageViewDescriptor{.m_image = std::move(p_image),
                              .m_extend = p_desc.m_extend,
                              .m_viewType = GetDefaultImageViewType(p_desc),
                              .m_format = p_desc.m_format,
                              .m_baseMipLevel = 0u,
                              .m_mipLevelCount = p_desc.m_mipLevels,
                              .m_baseArrayLayer = 0u,
                              .m_arrayLayerCount = p_desc.m_arrayLayers,
                              .m_aspectMask = GetDefaultAspectMask(p_desc.m_format)};
}

BufferViewDescriptor CreateDefaultRenderGraphBufferViewDescriptor(Ptr<Buffer> p_buffer, const BufferDescriptor& p_desc)
{
   // Buffers follow the same convention as images: one full default view per graph-created resource.
   return BufferViewDescriptor{.m_buffer = std::move(p_buffer),
                               .m_format = ResourceFormat::Invalid,
                               .m_offsetFromBaseAddress = 0u,
                               .m_bufferViewRange = p_desc.m_requestBufferSize,
                               .m_usage = GetDefaultBufferViewUsage(p_desc.m_bufferUsageFlags)};
}

// ----------- RenderGraphQuery -----------

RenderGraphQuery::RenderGraphQuery(RenderGraphPass& p_pass, Ptr<QueryResultState> p_state)
    : m_pass(&p_pass), m_state(std::move(p_state))
{
   ASSERT(m_state != nullptr, "RenderGraphQuery needs a valid QueryResultState");
}

bool RenderGraphQuery::IsValid() const
{
   return m_state != nullptr;
}

Ptr<GHI::Query> RenderGraphQuery::GetQuery() const
{
   ASSERT(m_state != nullptr, "RenderGraphQuery is invalid");
   return m_state->GetQuery();
}

Ptr<Buffer> RenderGraphQuery::GetReadbackBuffer() const
{
   ASSERT(m_state != nullptr, "RenderGraphQuery is invalid");
   return m_state->GetReadbackBuffer();
}

std::optional<QueryReadbackData> RenderGraphQuery::Readback() const
{
   ASSERT(m_state != nullptr, "RenderGraphQuery is invalid");
   // Non-waiting readback lets callers poll a previous frame without stalling the device/CPU timeline.
   return m_state->Readback();
}

QueryReadbackData RenderGraphQuery::ReadbackWait() const
{
   ASSERT(m_state != nullptr, "RenderGraphQuery is invalid");
   // The wait variant is explicit because it can synchronize with GPU work.
   return m_state->ReadbackWait();
}

RenderGraphQuery RenderGraphQuery::Execute(RenderGraphExecuteCallback p_execute)
{
   ASSERT(m_pass != nullptr, "RenderGraphQuery is not attached to a pass");
   m_pass->Execute(std::move(p_execute));
   return *this;
}

RenderGraphQuery RenderGraphQuery::Prepare(RenderGraphPrepareCallback p_prepare)
{
   ASSERT(m_pass != nullptr, "RenderGraphQuery is not attached to a pass");
   m_pass->Prepare(std::move(p_prepare));
   return *this;
}

RenderGraphQuery RenderGraphQuery::NeverCull()
{
   ASSERT(m_pass != nullptr, "RenderGraphQuery is not attached to a pass");
   m_pass->NeverCull();
   return *this;
}

// ----------- RenderGraphTimestampQuery -----------

RenderGraphTimestampQuery::RenderGraphTimestampQuery(RenderGraphPass& p_pass, Ptr<QueryResultState> p_beginState,
                                                     Ptr<QueryResultState> p_endState)
    : m_pass(&p_pass), m_beginState(std::move(p_beginState)), m_endState(std::move(p_endState))
{
   ASSERT(m_beginState != nullptr, "RenderGraphTimestampQuery needs a valid begin QueryResultState");
   ASSERT(m_endState != nullptr, "RenderGraphTimestampQuery needs a valid end QueryResultState");
}

bool RenderGraphTimestampQuery::IsValid() const
{
   return m_beginState != nullptr && m_endState != nullptr;
}

Ptr<GHI::Query> RenderGraphTimestampQuery::GetBeginQuery() const
{
   ASSERT(m_beginState != nullptr, "RenderGraphTimestampQuery is invalid");
   return m_beginState->GetQuery();
}

Ptr<GHI::Query> RenderGraphTimestampQuery::GetEndQuery() const
{
   ASSERT(m_endState != nullptr, "RenderGraphTimestampQuery is invalid");
   return m_endState->GetQuery();
}

Ptr<Buffer> RenderGraphTimestampQuery::GetBeginReadbackBuffer() const
{
   ASSERT(m_beginState != nullptr, "RenderGraphTimestampQuery is invalid");
   return m_beginState->GetReadbackBuffer();
}

Ptr<Buffer> RenderGraphTimestampQuery::GetEndReadbackBuffer() const
{
   ASSERT(m_endState != nullptr, "RenderGraphTimestampQuery is invalid");
   return m_endState->GetReadbackBuffer();
}

std::optional<QueryReadbackData> RenderGraphTimestampQuery::Readback() const
{
   ASSERT(m_beginState != nullptr, "RenderGraphTimestampQuery is invalid");
   ASSERT(m_endState != nullptr, "RenderGraphTimestampQuery is invalid");

   const std::optional<QueryReadbackData> beginData = m_beginState->Readback();
   const std::optional<QueryReadbackData> endData = m_endState->Readback();
   if (!beginData.has_value() || !endData.has_value())
   {
      // Timestamp pairs are only useful when both endpoints have resolved.
      return {};
   }

   return MakeTimestampReadbackData(beginData.value(), endData.value());
}

QueryReadbackData RenderGraphTimestampQuery::ReadbackWait() const
{
   ASSERT(m_beginState != nullptr, "RenderGraphTimestampQuery is invalid");
   ASSERT(m_endState != nullptr, "RenderGraphTimestampQuery is invalid");

   const QueryReadbackData beginData = m_beginState->ReadbackWait();
   const QueryReadbackData endData = m_endState->ReadbackWait();
   return MakeTimestampReadbackData(beginData, endData);
}

RenderGraphTimestampQuery RenderGraphTimestampQuery::Execute(RenderGraphExecuteCallback p_execute)
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   m_pass->Execute(std::move(p_execute));
   return *this;
}

RenderGraphTimestampQuery RenderGraphTimestampQuery::Prepare(RenderGraphPrepareCallback p_prepare)
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   m_pass->Prepare(std::move(p_prepare));
   return *this;
}

RenderGraphTimestampQuery RenderGraphTimestampQuery::NeverCull()
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   m_pass->NeverCull();
   return *this;
}

RenderGraphQuery RenderGraphTimestampQuery::WriteQuery(QueryDescriptor p_desc)
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   return m_pass->WriteQuery(std::move(p_desc));
}

RenderGraphQuery RenderGraphTimestampQuery::WriteQuery(Ptr<QueryPool> p_queryPool, QueryControlFlags p_controlFlags)
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   return m_pass->WriteQuery(std::move(p_queryPool), p_controlFlags);
}

RenderGraphQuery RenderGraphTimestampQuery::WriteQuery(Ptr<GHI::Query> p_query)
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   return m_pass->WriteQuery(std::move(p_query));
}

RenderGraphQuery RenderGraphTimestampQuery::WriteQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex,
                                                       QueryControlFlags p_controlFlags)
{
   ASSERT(m_pass != nullptr, "RenderGraphTimestampQuery is not attached to a pass");
   return m_pass->WriteQuery(std::move(p_queryPool), p_queryIndex, p_controlFlags);
}

// ----------- RenderGraphContext -----------

// Execute callbacks receive this read-only view of the solved pass. It exposes the pass-local recorder and
// the pass-declared handles, but not graph mutation.
RenderGraphContext::RenderGraphContext(RenderGraph& p_graph, RenderGraphPass& p_pass, SubCommandRecorder& p_recorder)
    : m_graph(&p_graph), m_pass(&p_pass), m_recorder(&p_recorder)
{
}

SubCommandRecorder& RenderGraphContext::GetRecorder() const
{
   return *m_recorder;
}

std::string_view RenderGraphContext::GetPassName() const
{
   return m_pass->GetName();
}

size_t RenderGraphContext::GetInputCount() const
{
   return m_pass->GetInputs().size();
}

size_t RenderGraphContext::GetOutputCount() const
{
   return m_pass->GetOutputs().size();
}

size_t RenderGraphContext::GetTransientCount() const
{
   return m_pass->GetTransients().size();
}

RenderGraphResourceHandle RenderGraphContext::GetInput(size_t p_index) const
{
   ASSERT(p_index < m_pass->GetInputs().size(), "RenderGraph execute input index is out of range");
   return m_pass->GetInputs()[p_index].m_handle;
}

RenderGraphResourceHandle RenderGraphContext::GetOutput(size_t p_index) const
{
   ASSERT(p_index < m_pass->GetOutputs().size(), "RenderGraph execute output index is out of range");
   return m_pass->GetOutputs()[p_index].m_handle;
}

RenderGraphResourceHandle RenderGraphContext::GetTransient(size_t p_index) const
{
   ASSERT(p_index < m_pass->GetTransients().size(), "RenderGraph execute transient index is out of range");
   return m_pass->GetTransients()[p_index];
}

RenderGraphResourceHandle RenderGraphContext::Input(std::string_view p_name) const
{
   return m_pass->FindNamedInput(p_name);
}

RenderGraphResourceHandle RenderGraphContext::Output(std::string_view p_name) const
{
   return m_pass->FindNamedOutput(p_name);
}

Ptr<ImageView> RenderGraphContext::GetImageView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(m_pass->HasDeclaredResource(p_handle), "RenderGraph execute context can only resolve resources declared by this pass");
   return m_graph->GetImageView(p_handle);
}

Ptr<BufferView> RenderGraphContext::GetBufferView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(m_pass->HasDeclaredResource(p_handle), "RenderGraph execute context can only resolve resources declared by this pass");
   return m_graph->GetBufferView(p_handle);
}

// ----------- RenderGraphPass -----------

// Pass builders accumulate declared accesses only. The actual order, barriers, and allocation strategy are
// derived later by Compile/Prepare so call order only matters as a stable tie-breaker.
RenderGraphPass::RenderGraphPass(RenderGraph& p_graph, uint32_t p_passIndex, std::string_view p_name)
    : m_graph(&p_graph), m_passIndex(p_passIndex), m_name(p_name)
{
}

RenderGraphPass& RenderGraphPass::Read(RenderGraphResourceHandle p_handle, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(IsReadOnlyUsage(p_usage), "RenderGraphPass::Read requires a read-only ResourceUsage");
   ASSERT(p_handle.IsValid(), "RenderGraph pass reads an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass reads with an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   // A read consumes an existing logical version; it never creates a new DAG output.
   const ResourceAccess access{.m_handle = p_handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_inputs.push_back(access);
   m_resourceAccesses.push_back(access);
   return *this;
}

RenderGraphPass& RenderGraphPass::Read(std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                       ShaderStageFlag p_shaderStages)
{
   Read(p_handle, p_usage, p_shaderStages);
   AddNamedInput(p_name, p_handle);
   return *this;
}

RenderGraphOutputList<1> RenderGraphPass::Write(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                                ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(IsWriteOnlyUsage(p_usage), "RenderGraphPass::Write requires a write-only ResourceUsage");
   ASSERT(p_handle.IsValid(), "RenderGraph pass writes an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass writes with an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   // Writes create a new logical version so later passes depend on this producer rather than the old handle.
   const RenderGraphResourceHandle output = m_graph->CreateResourceVersion(p_handle, m_passIndex);

   const ResourceAccess access{.m_handle = output, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_outputs.push_back(access);
   m_resourceAccesses.push_back(access);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{output});
}

RenderGraphOutputList<1> RenderGraphPass::Write(std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                                ShaderStageFlag p_shaderStages)
{
   RenderGraphOutputList<1> output = Write(p_handle, p_usage, p_shaderStages);
   AddNamedOutput(p_name, output.Get<0>());
   return output;
}

RenderGraphOutputList<1> RenderGraphPass::ReadWrite(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                                    ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(IsReadWriteUsage(p_usage), "RenderGraphPass::ReadWrite requires a read-write ResourceUsage");
   ASSERT(p_handle.IsValid(), "RenderGraph pass read-writes an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass read-writes with an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   // ReadWrite is modeled as consuming the previous version and producing a fresh version that shares storage.
   const RenderGraphResourceHandle output = m_graph->CreateResourceVersion(p_handle, m_passIndex);

   const ResourceAccess inputAccess{.m_handle = p_handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   const ResourceAccess outputAccess{.m_handle = output, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_inputs.push_back(inputAccess);
   m_outputs.push_back(outputAccess);
   m_resourceAccesses.push_back(inputAccess);
   m_resourceAccesses.push_back(outputAccess);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{output});
}

RenderGraphOutputList<1> RenderGraphPass::ReadWrite(std::string_view p_name, RenderGraphResourceHandle p_handle,
                                                    ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   RenderGraphOutputList<1> output = ReadWrite(p_handle, p_usage, p_shaderStages);
   AddNamedInput(p_name, p_handle);
   AddNamedOutput(p_name, output.Get<0>());
   return output;
}

RenderGraphPass& RenderGraphPass::Use(RenderGraphResourceHandle p_handle, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(p_handle.IsValid(), "RenderGraph pass uses an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass uses an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   // Use is the generic form for cases where the caller already chose an exact ResourceUsage.
   if (ResourceUsageReads(p_usage))
   {
      const ResourceAccess inputAccess{.m_handle = p_handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
      m_inputs.push_back(inputAccess);
      m_resourceAccesses.push_back(inputAccess);
   }

   if (ResourceUsageWrites(p_usage))
   {
      const RenderGraphResourceHandle output = m_graph->CreateResourceVersion(p_handle, m_passIndex);
      const ResourceAccess outputAccess{.m_handle = output, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
      m_outputs.push_back(outputAccess);
      m_resourceAccesses.push_back(outputAccess);
   }

   return *this;
}

RenderGraphPass& RenderGraphPass::Queue(QueueFamilyType p_queue)
{
   ASSERT(p_queue != QueueFamilyType::Invalid, "RenderGraph pass can't use an invalid queue");

   m_graph->m_compiled = false;
   m_graph->m_prepared = false;
   m_queue = p_queue;
   return *this;
}

RenderGraphPass& RenderGraphPass::Prepare(RenderGraphPrepareCallback p_prepare)
{
   m_graph->m_prepared = false;
   m_prepare = std::move(p_prepare);
   return *this;
}

RenderGraphPass& RenderGraphPass::Execute(RenderGraphExecuteCallback p_execute)
{
   m_graph->m_prepared = false;
   m_execute = std::move(p_execute);
   return *this;
}

RenderGraphPass& RenderGraphPass::ClearAttachment(RenderGraphResourceHandle p_handle, ClearColorValue p_clearValue)
{
   ASSERT(HasDeclaredResource(p_handle), "RenderGraphPass::ClearAttachment needs a resource declared by this pass");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraphPass::ClearAttachment handle is out of range");
   ASSERT(m_graph->m_resources[m_graph->GetStorageResourceIndex(p_handle)].m_type == RenderGraphResourceType::Image,
          "RenderGraphPass::ClearAttachment needs an image attachment");

   m_graph->m_prepared = false;
   const uint32_t storageResourceIndex = m_graph->GetStorageResourceIndex(p_handle);
   for (AttachmentClearInfo& clearInfo : m_attachmentClears)
   {
      if (m_graph->GetStorageResourceIndex(clearInfo.m_handle) == storageResourceIndex)
      {
         // Prepare-time clears can override declaration-time clears. Match by storage so logical versions
         // of the same attachment do not accumulate conflicting clear metadata.
         clearInfo = AttachmentClearInfo{.m_handle = p_handle, .m_clearValue = p_clearValue};
         return *this;
      }
   }

   m_attachmentClears.push_back(AttachmentClearInfo{.m_handle = p_handle, .m_clearValue = p_clearValue});
   return *this;
}

RenderGraphPass& RenderGraphPass::NeverCull()
{
   m_graph->m_prepared = false;
   m_neverCull = true;
   return *this;
}

RenderGraphQuery RenderGraphPass::WriteQuery(QueryDescriptor p_desc)
{
   ASSERT(p_desc.m_queryPool != nullptr, "RenderGraphPass::WriteQuery descriptor needs a QueryPool");

   // Descriptor overloads allocate a query index from the pool immediately, then store the resulting query
   // object as the pass-owned primary-stream query.
   Ptr<QueryPool> queryPool = p_desc.m_queryPool;
   const uint32_t queryIndex = queryPool->AllocateQueryIndex();
   Ptr<GHI::Query> query = std::make_shared<GHI::Query>(queryPool->GetDevice(), std::move(p_desc), queryPool, queryIndex);
   return WriteQuery(std::move(query));
}

RenderGraphQuery RenderGraphPass::WriteQuery(Ptr<QueryPool> p_queryPool, QueryControlFlags p_controlFlags)
{
   return WriteQuery(QueryDescriptor{.m_queryPool = std::move(p_queryPool), .m_controlFlags = p_controlFlags});
}

RenderGraphQuery RenderGraphPass::WriteQuery(Ptr<GHI::Query> p_query)
{
   ASSERT(p_query != nullptr, "RenderGraphPass::WriteQuery needs a valid Query");
   ASSERT(p_query->GetType() != QueryType::Timestamp,
          "RenderGraphPass::WriteQuery is for begin/end query types; use WriteTimestamps for timestamp pools");

   m_graph->m_prepared = false;
   // The result state is shared by the returned promise and the later primary-stream ResolveQueryData command.
   Ptr<QueryResultState> resultState = std::make_shared<QueryResultState>(p_query);
   m_query = PassQueryInfo{.m_query = std::move(p_query), .m_resultState = resultState};
   return RenderGraphQuery(*this, std::move(resultState));
}

RenderGraphQuery RenderGraphPass::WriteQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex, QueryControlFlags p_controlFlags)
{
   ASSERT(p_queryPool != nullptr, "RenderGraphPass::WriteQuery needs a valid QueryPool");
   ASSERT(p_queryPool->GetType() != QueryType::Timestamp,
          "RenderGraphPass::WriteQuery is for begin/end query types; use WriteTimestamps for timestamp pools");
   ASSERT(p_queryIndex < p_queryPool->GetQueryCount(), "RenderGraphPass::WriteQuery query index is out of range");

   QueryDescriptor queryDesc{.m_queryPool = p_queryPool, .m_controlFlags = p_controlFlags};
   Ptr<GHI::Query> query =
       std::make_shared<GHI::Query>(p_queryPool->GetDevice(), std::move(queryDesc), std::move(p_queryPool), p_queryIndex);
   return WriteQuery(std::move(query));
}

RenderGraphTimestampQuery RenderGraphPass::WriteTimestamps(Ptr<GHI::Query> p_beginQuery, Ptr<GHI::Query> p_endQuery,
                                                           PipelineStageFlags p_beginStage, PipelineStageFlags p_endStage)
{
   ASSERT(p_beginQuery != nullptr, "RenderGraphPass::WriteTimestamps needs a valid begin Query");
   ASSERT(p_endQuery != nullptr, "RenderGraphPass::WriteTimestamps needs a valid end Query");
   ASSERT(p_beginQuery->GetType() == QueryType::Timestamp,
          "RenderGraphPass::WriteTimestamps begin Query must be a timestamp Query");
   ASSERT(p_endQuery->GetType() == QueryType::Timestamp, "RenderGraphPass::WriteTimestamps end Query must be a timestamp Query");
   ASSERT(p_beginQuery->GetQueryPool() != p_endQuery->GetQueryPool() ||
              p_beginQuery->GetQueryIndex() != p_endQuery->GetQueryIndex(),
          "RenderGraphPass::WriteTimestamps needs distinct begin and end Queries");

   m_graph->m_prepared = false;
   // Timestamp begin/end are stored as two query result states but exposed as one promise to the caller.
   Ptr<QueryResultState> beginResultState = std::make_shared<QueryResultState>(p_beginQuery);
   Ptr<QueryResultState> endResultState = std::make_shared<QueryResultState>(p_endQuery);
   m_timestamps = PassTimestampInfo{.m_beginQuery = p_beginQuery,
                                    .m_endQuery = p_endQuery,
                                    .m_beginResultState = beginResultState,
                                    .m_endResultState = endResultState,
                                    .m_beginQueryIndex = p_beginQuery->GetQueryIndex(),
                                    .m_endQueryIndex = p_endQuery->GetQueryIndex(),
                                    .m_beginStage = p_beginStage,
                                    .m_endStage = p_endStage};
   return RenderGraphTimestampQuery(*this, std::move(beginResultState), std::move(endResultState));
}

RenderGraphTimestampQuery RenderGraphPass::WriteTimestamps(Ptr<QueryPool> p_queryPool, uint32_t p_beginQueryIndex,
                                                           uint32_t p_endQueryIndex, PipelineStageFlags p_beginStage,
                                                           PipelineStageFlags p_endStage)
{
   ASSERT(p_queryPool != nullptr, "RenderGraphPass::WriteTimestamps needs a valid QueryPool");
   ASSERT(p_queryPool->GetType() == QueryType::Timestamp, "RenderGraphPass::WriteTimestamps requires a timestamp QueryPool");
   ASSERT(p_beginQueryIndex < p_queryPool->GetQueryCount(), "RenderGraphPass::WriteTimestamps begin query index is out of range");
   ASSERT(p_endQueryIndex < p_queryPool->GetQueryCount(), "RenderGraphPass::WriteTimestamps end query index is out of range");
   ASSERT(p_beginQueryIndex != p_endQueryIndex, "RenderGraphPass::WriteTimestamps needs distinct begin and end query indices");

   Ptr<QueryPool> queryPool = std::move(p_queryPool);
   QueryDescriptor beginDesc{.m_queryPool = queryPool};
   Ptr<GHI::Query> beginQuery =
       std::make_shared<GHI::Query>(queryPool->GetDevice(), std::move(beginDesc), queryPool, p_beginQueryIndex);
   QueryDescriptor endDesc{.m_queryPool = queryPool};
   Ptr<GHI::Query> endQuery =
       std::make_shared<GHI::Query>(queryPool->GetDevice(), std::move(endDesc), std::move(queryPool), p_endQueryIndex);

   return WriteTimestamps(std::move(beginQuery), std::move(endQuery), p_beginStage, p_endStage);
}

RenderGraphOutputList<1> RenderGraphPass::WriteImage(std::string_view p_name, ImageDescriptor p_desc, ResourceUsage p_usage,
                                                     ShaderStageFlag p_shaderStages)
{
   ASSERT(IsWriteOnlyUsage(p_usage), "RenderGraphPass::WriteImage requires a write-only ResourceUsage");

   // Descriptor-backed outputs are graph-owned resources. They are materialized later, after lifetime analysis
   // decides whether transient aliasing is possible.
   RenderGraphResourceHandle handle =
       m_graph->AddImageResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                 QueueFamilyType::Invalid, false, false, RenderGraphResourceHandle::InvalidIndex);
   m_graph->SetResourceProducer(handle, m_passIndex);
   const ResourceAccess access{.m_handle = handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_outputs.push_back(access);
   m_resourceAccesses.push_back(access);
   AddNamedOutput(p_name, handle);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{handle});
}

RenderGraphOutputList<1> RenderGraphPass::WriteBuffer(std::string_view p_name, BufferDescriptor p_desc, ResourceUsage p_usage,
                                                      ShaderStageFlag p_shaderStages)
{
   ASSERT(IsWriteOnlyUsage(p_usage), "RenderGraphPass::WriteBuffer requires a write-only ResourceUsage");

   // Buffer descriptors follow the same deferred materialization path as image descriptors.
   RenderGraphResourceHandle handle =
       m_graph->AddBufferResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                  QueueFamilyType::Invalid, false, false, RenderGraphResourceHandle::InvalidIndex);
   m_graph->SetResourceProducer(handle, m_passIndex);
   const ResourceAccess access{.m_handle = handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_outputs.push_back(access);
   m_resourceAccesses.push_back(access);
   AddNamedOutput(p_name, handle);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{handle});
}

RenderGraphOutputList<1> RenderGraphPass::ReadWriteImage(std::string_view p_name, ImageDescriptor p_desc, ResourceUsage p_usage,
                                                         ShaderStageFlag p_shaderStages)
{
   ASSERT(IsReadWriteUsage(p_usage), "RenderGraphPass::ReadWriteImage requires a read-write ResourceUsage");

   // A named ReadWrite resource starts as a graph-owned descriptor resource and immediately creates an output
   // version, so downstream passes get a unique handle for dependency solving.
   RenderGraphResourceHandle handle =
       m_graph->AddImageResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                 QueueFamilyType::Invalid, false, false, RenderGraphResourceHandle::InvalidIndex);
   const RenderGraphOutputList<1> output = ReadWrite(handle, p_usage, p_shaderStages);
   AddNamedInput(p_name, handle);
   AddNamedOutput(p_name, output.Get<0>());
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{output.Get<0>()});
}

RenderGraphOutputList<1> RenderGraphPass::ReadWriteBuffer(std::string_view p_name, BufferDescriptor p_desc, ResourceUsage p_usage,
                                                          ShaderStageFlag p_shaderStages)
{
   ASSERT(IsReadWriteUsage(p_usage), "RenderGraphPass::ReadWriteBuffer requires a read-write ResourceUsage");

   // See ReadWriteImage: the storage is one resource, but the DAG edge uses the produced logical version.
   RenderGraphResourceHandle handle =
       m_graph->AddBufferResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                  QueueFamilyType::Invalid, false, false, RenderGraphResourceHandle::InvalidIndex);
   const RenderGraphOutputList<1> output = ReadWrite(handle, p_usage, p_shaderStages);
   AddNamedInput(p_name, handle);
   AddNamedOutput(p_name, output.Get<0>());
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{output.Get<0>()});
}

std::string_view RenderGraphPass::GetName() const
{
   return m_name;
}

QueueFamilyType RenderGraphPass::GetQueue() const
{
   return m_queue;
}

const std::vector<RenderGraphPass::ResourceAccess>& RenderGraphPass::GetResourceAccesses() const
{
   return m_resourceAccesses;
}

const std::vector<RenderGraphPass::ResourceAccess>& RenderGraphPass::GetInputs() const
{
   return m_inputs;
}

const std::vector<RenderGraphPass::ResourceAccess>& RenderGraphPass::GetOutputs() const
{
   return m_outputs;
}

const std::vector<RenderGraphResourceHandle>& RenderGraphPass::GetTransients() const
{
   return m_transients;
}

RenderGraphResourceHandle RenderGraphPass::FindNamedInput(std::string_view p_name) const
{
   const auto it = std::find_if(m_namedInputs.begin(), m_namedInputs.end(),
                                [p_name](const NamedResource& p_namedResource) { return p_namedResource.m_name == p_name; });
   ASSERT(it != m_namedInputs.end(), "RenderGraph pass has no input with that name");
   return it->m_handle;
}

RenderGraphResourceHandle RenderGraphPass::FindNamedOutput(std::string_view p_name) const
{
   const auto it = std::find_if(m_namedOutputs.begin(), m_namedOutputs.end(),
                                [p_name](const NamedResource& p_namedResource) { return p_namedResource.m_name == p_name; });
   ASSERT(it != m_namedOutputs.end(), "RenderGraph pass has no output with that name");
   return it->m_handle;
}

bool RenderGraphPass::HasDeclaredResource(RenderGraphResourceHandle p_handle) const
{
   const auto it = std::find_if(m_resourceAccesses.begin(), m_resourceAccesses.end(), [p_handle](const ResourceAccess& p_access) {
      return p_access.m_handle.m_index == p_handle.m_index;
   });
   return it != m_resourceAccesses.end();
}

void RenderGraphPass::AddNamedInput(std::string_view p_name, RenderGraphResourceHandle p_handle)
{
   ASSERT(!p_name.empty(), "RenderGraph named input needs a name");
   const auto it = std::find_if(m_namedInputs.begin(), m_namedInputs.end(),
                                [p_name](const NamedResource& p_namedResource) { return p_namedResource.m_name == p_name; });
   ASSERT(it == m_namedInputs.end(), "RenderGraph pass already has an input with that name");
   m_namedInputs.push_back(NamedResource{.m_name = std::string(p_name), .m_handle = p_handle});
}

void RenderGraphPass::AddNamedOutput(std::string_view p_name, RenderGraphResourceHandle p_handle)
{
   ASSERT(!p_name.empty(), "RenderGraph named output needs a name");
   const auto it = std::find_if(m_namedOutputs.begin(), m_namedOutputs.end(),
                                [p_name](const NamedResource& p_namedResource) { return p_namedResource.m_name == p_name; });
   ASSERT(it == m_namedOutputs.end(), "RenderGraph pass already has an output with that name");
   m_namedOutputs.push_back(NamedResource{.m_name = std::string(p_name), .m_handle = p_handle});
}

// ----------- RenderGraphPrepareContext -----------

// Prepare runs after the DAG order exists but before Execute. It can create pass-local scratch resources,
// which are deliberately not visible to other passes.
RenderGraphPrepareContext::RenderGraphPrepareContext(RenderGraph& p_graph, RenderGraphPass& p_pass, uint32_t p_passOrder)
    : m_graph(&p_graph), m_pass(&p_pass), m_passOrder(p_passOrder)
{
}

std::string_view RenderGraphPrepareContext::GetPassName() const
{
   return m_pass->GetName();
}

size_t RenderGraphPrepareContext::GetInputCount() const
{
   return m_pass->GetInputs().size();
}

size_t RenderGraphPrepareContext::GetOutputCount() const
{
   return m_pass->GetOutputs().size();
}

size_t RenderGraphPrepareContext::GetTransientCount() const
{
   return m_pass->GetTransients().size();
}

RenderGraphResourceHandle RenderGraphPrepareContext::GetInput(size_t p_index) const
{
   ASSERT(p_index < m_pass->GetInputs().size(), "RenderGraph prepare input index is out of range");
   return m_pass->GetInputs()[p_index].m_handle;
}

RenderGraphResourceHandle RenderGraphPrepareContext::GetOutput(size_t p_index) const
{
   ASSERT(p_index < m_pass->GetOutputs().size(), "RenderGraph prepare output index is out of range");
   return m_pass->GetOutputs()[p_index].m_handle;
}

RenderGraphResourceHandle RenderGraphPrepareContext::GetTransient(size_t p_index) const
{
   ASSERT(p_index < m_pass->GetTransients().size(), "RenderGraph prepare transient index is out of range");
   return m_pass->GetTransients()[p_index];
}

RenderGraphResourceHandle RenderGraphPrepareContext::Input(std::string_view p_name) const
{
   return m_pass->FindNamedInput(p_name);
}

RenderGraphResourceHandle RenderGraphPrepareContext::Output(std::string_view p_name) const
{
   return m_pass->FindNamedOutput(p_name);
}

Ptr<ImageView> RenderGraphPrepareContext::GetImageView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(m_pass->HasDeclaredResource(p_handle), "RenderGraph prepare context can only resolve resources declared by this pass");
   return m_graph->GetImageView(p_handle);
}

Ptr<BufferView> RenderGraphPrepareContext::GetBufferView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(m_pass->HasDeclaredResource(p_handle), "RenderGraph prepare context can only resolve resources declared by this pass");
   return m_graph->GetBufferView(p_handle);
}

const ImageDescriptor* RenderGraphPrepareContext::GetImageDescriptor(RenderGraphResourceHandle p_handle) const
{
   return m_graph->GetImageDescriptor(p_handle);
}

const BufferDescriptor* RenderGraphPrepareContext::GetBufferDescriptor(RenderGraphResourceHandle p_handle) const
{
   return m_graph->GetBufferDescriptor(p_handle);
}

bool RenderGraphPrepareContext::WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const
{
   return m_graph->WasResourceCreatedInPrepare(p_handle);
}

bool RenderGraphPrepareContext::CanResourceBeTransient(RenderGraphResourceHandle p_handle) const
{
   return m_graph->CanResourceBeTransient(p_handle);
}

void RenderGraphPrepareContext::ClearAttachment(RenderGraphResourceHandle p_handle, ClearColorValue p_clearValue)
{
   // Forward through RenderGraphPass validation so prepare-time clears obey the same declared-resource rules
   // as declaration-time clears.
   m_pass->ClearAttachment(p_handle, p_clearValue);
}

void RenderGraphPrepareContext::ClearAttachment(size_t p_outputIndex, ClearColorValue p_clearValue)
{
   ClearAttachment(GetOutput(p_outputIndex), p_clearValue);
}

void RenderGraphPrepareContext::ClearAttachment(std::string_view p_outputName, ClearColorValue p_clearValue)
{
   ClearAttachment(Output(p_outputName), p_clearValue);
}

RenderGraphResourceHandle RenderGraphPrepareContext::CreateTransientImage(std::string_view p_name, ImageDescriptor p_desc,
                                                                          ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   // Prepare-created resources get a fixed one-pass lifetime, making them natural transient allocation inputs.
   return m_graph->CreatePassTransientImage(*m_pass, m_passOrder, p_name, std::move(p_desc), p_usage, p_shaderStages);
}

RenderGraphResourceHandle RenderGraphPrepareContext::CreateTransientBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                                           ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   // The pass records the handle as a transient access so barriers still see the resource during Execute.
   return m_graph->CreatePassTransientBuffer(*m_pass, m_passOrder, p_name, std::move(p_desc), p_usage, p_shaderStages);
}

// ----------- RenderGraphTransientResourceWriter -----------

// Backends only materialize concrete Image/Buffer objects. The graph wraps those resources in its default
// full-resource views and validates that each view references the exact object the backend provided.
RenderGraphTransientResourceWriter::RenderGraphTransientResourceWriter(RenderGraph& p_graph) : m_graph(&p_graph)
{
}

void RenderGraphTransientResourceWriter::SetImage(RenderGraphResourceHandle p_handle, Ptr<Image> p_image)
{
   ASSERT(p_image != nullptr, "RenderGraph transient materializer assigned a null Image");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph materialized image handle is out of range");

   const uint32_t storageIndex = m_graph->GetStorageResourceIndex(p_handle);
   RenderGraph::Resource& resource = m_graph->m_resources[storageIndex];
   ASSERT(resource.m_type == RenderGraphResourceType::Image, "RenderGraph materialized resource is not an image");
   ASSERT(resource.m_canBeTransient, "RenderGraph materializer can only assign transient image resources");
   ASSERT(resource.m_imageDesc.has_value(), "RenderGraph transient image needs an ImageDescriptor");

   const Image* image = p_image.get();
   Ptr<Device> device = p_image->GetDevice();
   ASSERT(device != nullptr, "RenderGraph transient image has no Device");

   // The backend may supply platform-specific view construction; otherwise the graph falls back to ResourceFactory.
   if (m_graph->m_imageViewMaterializer)
   {
      resource.m_imageView = m_graph->m_imageViewMaterializer(std::move(p_image), resource.m_imageDesc.value());
   }
   else
   {
      resource.m_imageView = ResourceFactory::Get()->CreateImageView(
          device, CreateDefaultRenderGraphImageViewDescriptor(std::move(p_image), resource.m_imageDesc.value()));
   }
   ASSERT(resource.m_imageView != nullptr, "RenderGraph failed to create a transient ImageView");
   ASSERT(resource.m_imageView->GetImage().get() == image, "RenderGraph transient ImageView must reference the materialized Image");
   resource.m_createdInPrepare = true;
}

void RenderGraphTransientResourceWriter::SetBuffer(RenderGraphResourceHandle p_handle, Ptr<Buffer> p_buffer)
{
   ASSERT(p_buffer != nullptr, "RenderGraph transient materializer assigned a null Buffer");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph materialized buffer handle is out of range");

   const uint32_t storageIndex = m_graph->GetStorageResourceIndex(p_handle);
   RenderGraph::Resource& resource = m_graph->m_resources[storageIndex];
   ASSERT(resource.m_type == RenderGraphResourceType::Buffer, "RenderGraph materialized resource is not a buffer");
   ASSERT(resource.m_canBeTransient, "RenderGraph materializer can only assign transient buffer resources");
   ASSERT(resource.m_bufferDesc.has_value(), "RenderGraph transient buffer needs a BufferDescriptor");

   const Buffer* buffer = p_buffer.get();
   Ptr<Device> device = p_buffer->GetDevice();
   ASSERT(device != nullptr, "RenderGraph transient buffer has no Device");

   // Buffer views are also graph-owned policy: transient materializers should not decide arbitrary view ranges.
   if (m_graph->m_bufferViewMaterializer)
   {
      resource.m_bufferView = m_graph->m_bufferViewMaterializer(std::move(p_buffer), resource.m_bufferDesc.value());
   }
   else
   {
      resource.m_bufferView = ResourceFactory::Get()->CreateBufferView(
          device, CreateDefaultRenderGraphBufferViewDescriptor(std::move(p_buffer), resource.m_bufferDesc.value()));
   }
   ASSERT(resource.m_bufferView != nullptr, "RenderGraph failed to create a transient BufferView");
   ASSERT(resource.m_bufferView->GetBuffer().get() == buffer,
          "RenderGraph transient BufferView must reference the materialized Buffer");
   resource.m_createdInPrepare = true;
}

// ----------- RenderGraph -----------

void RenderGraph::Reset()
{
   // Reset keeps backend hooks intact, but discards all per-frame graph data and solved analysis.
   m_resources.clear();
   m_passes.clear();
   m_executionOrder.clear();
   m_transientResources.clear();
   m_transientAliasGroups.clear();
   m_compiled = false;
   m_prepared = false;
   m_isPreparing = false;
}

void RenderGraph::SetImageMaterializer(RenderGraphImageMaterializer p_materializer)
{
   // Single-resource materializers are used for non-aliased descriptor resources and simple fallback paths.
   m_imageMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetBufferMaterializer(RenderGraphBufferMaterializer p_materializer)
{
   m_bufferMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetImageViewMaterializer(RenderGraphImageViewMaterializer p_materializer)
{
   // View materializers are narrower than transient materializers: they only wrap a concrete resource in a view.
   m_imageViewMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetBufferViewMaterializer(RenderGraphBufferViewMaterializer p_materializer)
{
   m_bufferViewMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetBarrierEmitter(RenderGraphBarrierEmitter p_barrierEmitter)
{
   m_barrierEmitter = std::move(p_barrierEmitter);
}

void RenderGraph::SetQueueFamilyResolver(RenderGraphQueueFamilyResolver p_queueFamilyResolver)
{
   m_queueFamilyResolver = std::move(p_queueFamilyResolver);
}

void RenderGraph::SetTransientCompatibilityChecker(RenderGraphTransientCompatibilityChecker p_checker)
{
   // Pair compatibility gates the basic "can these two storage resources share one allocation?" question.
   m_transientCompatibilityChecker = std::move(p_checker);
   if (m_compiled)
   {
      AnalyzeTransientResources();
   }
   m_prepared = false;
}

void RenderGraph::SetTransientAliasGroupCompatibilityChecker(RenderGraphTransientAliasGroupCompatibilityChecker p_checker)
{
   // Group compatibility lets the backend reject a candidate against the complete alias slot, not only pairs.
   m_transientAliasGroupCompatibilityChecker = std::move(p_checker);
   if (m_compiled)
   {
      AnalyzeTransientResources();
   }
   m_prepared = false;
}

// Changing backend allocation facts can change the alias plan without changing the pass DAG.
void RenderGraph::SetTransientAllocationSizeResolver(RenderGraphTransientAllocationSizeResolver p_resolver)
{
   m_transientAllocationSizeResolver = std::move(p_resolver);
   if (m_compiled)
   {
      AnalyzeTransientResources();
   }
   m_prepared = false;
}

void RenderGraph::SetTransientMaterializer(RenderGraphTransientMaterializer p_materializer)
{
   // A transient materializer consumes the already-scheduled alias groups and must create every requested resource.
   m_transientMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetSubCommandBufferCreator(RenderGraphSubCommandBufferCreator p_creator)
{
   m_subCommandBufferCreator = std::move(p_creator);
}

void RenderGraph::SetQueryReadbackBufferCreator(RenderGraphQueryReadbackBufferCreator p_creator)
{
   m_queryReadbackBufferCreator = std::move(p_creator);
}

void RenderGraph::SetParallelPassRecordingEnabled(bool p_enabled)
{
   m_parallelPassRecordingEnabled = p_enabled;
}

bool RenderGraph::IsParallelPassRecordingEnabled() const
{
   return m_parallelPassRecordingEnabled;
}

RenderGraphResourceHandle RenderGraph::ImportImageView(std::string_view p_name, Ptr<ImageView> p_imageView,
                                                       ResourceUsage p_initialUsage, ShaderStageFlag p_initialShaderStages,
                                                       QueueFamilyType p_initialQueue)
{
   ASSERT(p_imageView != nullptr, "Can't import a null ImageView into the RenderGraph");

   // Imports are externally owned and already valid at graph start. The graph tracks state, but never aliases them.
   return AddImageResource(p_name, std::move(p_imageView), std::nullopt, p_initialUsage, p_initialShaderStages, p_initialQueue,
                           true, false, RenderGraphResourceHandle::InvalidIndex);
}

RenderGraphResourceHandle RenderGraph::ImportBufferView(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                                        ResourceUsage p_initialUsage, ShaderStageFlag p_initialShaderStages,
                                                        QueueFamilyType p_initialQueue)
{
   ASSERT(p_bufferView != nullptr, "Can't import a null BufferView into the RenderGraph");

   // Imported buffers are the same contract as imported images: external lifetime, explicit initial state.
   return AddBufferResource(p_name, std::move(p_bufferView), std::nullopt, p_initialUsage, p_initialShaderStages, p_initialQueue,
                            true, false, RenderGraphResourceHandle::InvalidIndex);
}

RenderGraphPass& RenderGraph::AddPass(std::string_view p_name)
{
   // Pass indices are stable identity. The solved execution order is stored separately after Compile().
   m_compiled = false;
   m_prepared = false;
   const uint32_t passIndex = static_cast<uint32_t>(m_passes.size());
   return m_passes.emplace_back(*this, passIndex, p_name);
}

void RenderGraph::Compile()
{
   m_prepared = false;

   const uint32_t passCount = static_cast<uint32_t>(m_passes.size());
   std::vector<std::vector<uint32_t>> dependencies(passCount);

   // Build dependency edges from resource producers to consumers. Pass insertion order is only a tie-breaker
   // for otherwise independent work.
   for (uint32_t passIndex = 0u; passIndex < passCount; ++passIndex)
   {
      // Validate every declared access before deriving edges; this keeps mistakes close to the pass declaration.
      for (const RenderGraphPass::ResourceAccess& access : m_passes[passIndex].GetResourceAccesses())
      {
         ASSERT(access.m_handle.m_index < m_resources.size(), "RenderGraph pass references an unknown resource");

         const Resource& resource = m_resources[access.m_handle.m_index];
         ValidateResourceAccess(resource, access);
      }

      for (const RenderGraphPass::ResourceAccess& access : m_passes[passIndex].GetInputs())
      {
         ASSERT(access.m_handle.m_index < m_resources.size(), "RenderGraph pass references an unknown resource");

         const Resource& resource = m_resources[access.m_handle.m_index];
         const uint32_t producerPass = resource.m_producerPass;
         if (producerPass != RenderGraphResourceHandle::InvalidIndex && producerPass != passIndex)
         {
            // A pass that reads a produced handle must run after that producer.
            ASSERT(producerPass < passCount, "RenderGraph resource producer pass is out of range");
            AddDependency(dependencies, producerPass, passIndex);
         }
      }

      for (const RenderGraphPass::ResourceAccess& access : m_passes[passIndex].GetOutputs())
      {
         ASSERT(access.m_handle.m_index < m_resources.size(), "RenderGraph pass references an unknown resource");

         const Resource& resource = m_resources[access.m_handle.m_index];
         if (resource.m_previousVersion == RenderGraphResourceHandle::InvalidIndex)
         {
            continue;
         }

         // A write version cannot execute before the producer of the version it overwrites.
         ASSERT(resource.m_previousVersion < m_resources.size(), "RenderGraph resource previous version is out of range");
         const Resource& previousResource = m_resources[resource.m_previousVersion];
         const uint32_t previousProducerPass = previousResource.m_producerPass;
         if (previousProducerPass != RenderGraphResourceHandle::InvalidIndex && previousProducerPass != passIndex)
         {
            ASSERT(previousProducerPass < passCount, "RenderGraph previous resource producer pass is out of range");
            AddDependency(dependencies, previousProducerPass, passIndex);
         }
      }
   }

   // Versioned writes share storage with their previous handle, so old-version readers must run before the overwrite.
   for (uint32_t writerPassIndex = 0u; writerPassIndex < passCount; ++writerPassIndex)
   {
      for (const RenderGraphPass::ResourceAccess& outputAccess : m_passes[writerPassIndex].GetOutputs())
      {
         const Resource& outputResource = m_resources[outputAccess.m_handle.m_index];
         if (outputResource.m_previousVersion == RenderGraphResourceHandle::InvalidIndex)
         {
            continue;
         }

         for (uint32_t readerPassIndex = 0u; readerPassIndex < passCount; ++readerPassIndex)
         {
            if (readerPassIndex == writerPassIndex)
            {
               continue;
            }

            for (const RenderGraphPass::ResourceAccess& inputAccess : m_passes[readerPassIndex].GetInputs())
            {
               if (inputAccess.m_handle.m_index == outputResource.m_previousVersion)
               {
                  // This preserves SSA-like logical handles while still respecting one physical storage object.
                  AddDependency(dependencies, readerPassIndex, writerPassIndex);
               }
            }
         }
      }
   }

   std::vector<uint32_t> indegree(passCount, 0u);
   for (uint32_t from = 0u; from < passCount; ++from)
   {
      for (const uint32_t to : dependencies[from])
      {
         ++indegree[to];
      }
   }

   m_executionOrder.clear();
   m_executionOrder.reserve(passCount);

   // Kahn topological sort. Scanning pass indices keeps the result stable for equal-priority choices.
   std::vector<bool> emitted(passCount, false);
   while (m_executionOrder.size() < passCount)
   {
      uint32_t nextPass = InvalidPass;
      for (uint32_t passIndex = 0u; passIndex < passCount; ++passIndex)
      {
         if (!emitted[passIndex] && indegree[passIndex] == 0u)
         {
            nextPass = passIndex;
            break;
         }
      }

      ASSERT(nextPass != InvalidPass, "RenderGraph contains a dependency cycle");

      // Emitting a zero-indegree pass removes its outgoing edges from the remaining graph.
      emitted[nextPass] = true;
      m_executionOrder.push_back(nextPass);

      for (const uint32_t dependentPass : dependencies[nextPass])
      {
         ASSERT(indegree[dependentPass] > 0u, "RenderGraph dependency indegree underflow");
         --indegree[dependentPass];
      }
   }

   UpdateResourceLifetimes();
   AnalyzeTransientResources();

   // Compile stops before allocation/materialization. Prepare owns that phase because prepare callbacks may
   // add pass-local resources that affect transient aliasing.
   m_compiled = true;
}

void RenderGraph::Prepare()
{
   if (!m_compiled)
   {
      Compile();
   }

   ASSERT(!m_isPreparing, "RenderGraph is already preparing resources");

   // Descriptor-backed graph resources are materialized before pass prepare callbacks, so prepare code can
   // inspect concrete views while still letting the graph decide transient eligibility first.
   AnalyzeTransientResources();
   MaterializeGraphResources();

   m_isPreparing = true;
   for (size_t orderIndex = 0u; orderIndex < m_executionOrder.size(); ++orderIndex)
   {
      const uint32_t passIndex = m_executionOrder[orderIndex];
      RenderGraphPass& pass = m_passes[passIndex];
      if (pass.m_prepare)
      {
         RenderGraphPrepareContext context(*this, pass, static_cast<uint32_t>(orderIndex));
         pass.m_prepare(context);
      }
   }
   m_isPreparing = false;

   // Pass-local prepare resources are discovered during callbacks, so transient analysis runs again.
   AnalyzeTransientResources();
   m_prepared = true;
}

void RenderGraph::Execute(CommandBuffer& p_commandBuffer)
{
   if (!m_compiled)
   {
      Compile();
   }
   if (!m_prepared)
   {
      Prepare();
   }

   std::vector<ResourceState> resourceStates;
   resourceStates.reserve(m_resources.size());
   for (const Resource& resource : m_resources)
   {
      // Resource state starts from imports' explicit initial state or Undefined for graph-created resources.
      resourceStates.push_back(ResourceState{.m_usage = resource.m_initialUsage,
                                             .m_shaderStages = resource.m_initialShaderStages,
                                             .m_queue = resource.m_initialQueue});
   }

   std::vector<Ptr<SubCommandBuffer>> recordedPassCommandBuffers(m_passes.size());
   for (const uint32_t passIndex : m_executionOrder)
   {
      RenderGraphPass& pass = m_passes[passIndex];
      ASSERT(pass.GetQueue() == p_commandBuffer.GetQueueType(),
             "RenderGraph::Execute(CommandBuffer&) only supports passes for the provided CommandBuffer queue");

      if (pass.m_execute)
      {
         // Pass callbacks record into subcommand buffers. The primary command buffer stays reserved for graph
         // orchestration: barriers, rendering scopes, queries, timestamps, resolves, and subcommand execution.
         recordedPassCommandBuffers[passIndex] = CreateSubCommandBuffer(p_commandBuffer.GetDevice());
      }
   }

   auto recordPassCommands = [this, &recordedPassCommandBuffers](uint32_t p_passIndex) {
      Ptr<SubCommandBuffer>& passCommandBuffer = recordedPassCommandBuffers[p_passIndex];
      if (passCommandBuffer == nullptr)
      {
         return;
      }

      RenderGraphPass& pass = m_passes[p_passIndex];
      RenderGraphContext context(*this, pass, *passCommandBuffer);
      // The callback only sees pass-local declared resources and a SubCommandRecorder, so it is safe to run
      // independently from primary command assembly.
      pass.m_execute(context);
   };

   if (m_parallelPassRecordingEnabled)
   {
      // Parallel recording is limited to pass-local subcommand streams. Primary stream ordering remains serial
      // and follows the solved graph order below.
      std::vector<std::future<void>> passRecordingTasks;
      passRecordingTasks.reserve(m_executionOrder.size());
      for (const uint32_t passIndex : m_executionOrder)
      {
         if (recordedPassCommandBuffers[passIndex] != nullptr)
         {
            passRecordingTasks.push_back(std::async(std::launch::async, recordPassCommands, passIndex));
         }
      }

      for (std::future<void>& passRecordingTask : passRecordingTasks)
      {
         passRecordingTask.get();
      }
   }
   else
   {
      for (const uint32_t passIndex : m_executionOrder)
      {
         recordPassCommands(passIndex);
      }
   }

   for (const uint32_t passIndex : m_executionOrder)
   {
      RenderGraphPass& pass = m_passes[passIndex];

      for (const RenderGraphPass::ResourceAccess& access : pass.GetResourceAccesses())
      {
         const uint32_t storageResourceIndex = GetStorageResourceIndex(access.m_handle);
         ResourceState& state = resourceStates[storageResourceIndex];
         // Invalid means "first use on this pass queue"; this avoids emitting meaningless ownership transfers
         // for graph-created resources that have no pre-graph queue owner.
         const QueueFamilyType oldQueue = state.m_queue == QueueFamilyType::Invalid ? pass.GetQueue() : state.m_queue;
         const ResourceState oldState{.m_usage = state.m_usage, .m_shaderStages = state.m_shaderStages, .m_queue = oldQueue};
         const ResourceState nextState{
             .m_usage = access.m_usage, .m_shaderStages = access.m_shaderStages, .m_queue = pass.GetQueue()};

         if (oldState.m_usage != nextState.m_usage || oldState.m_shaderStages != nextState.m_shaderStages ||
             oldState.m_queue != nextState.m_queue)
         {
            // Barrier emission is delegated to the backend; the agnostic graph only decides when a transition exists.
            EmitBarrier(p_commandBuffer, m_resources[storageResourceIndex], oldState, nextState);
            state = nextState;
         }
      }

      Ptr<SubCommandBuffer> passCommandBuffer = recordedPassCommandBuffers[passIndex];
      const bool hasPassCommands = passCommandBuffer != nullptr && !passCommandBuffer->GetRenderCommands().empty();
      const bool hasPrimaryPassWork = hasPassCommands || pass.m_query.has_value() || pass.m_timestamps.has_value();
      if (hasPrimaryPassWork)
      {
         // Primary pass work surrounds the optional subcommand stream. This keeps APIs with strict secondary
         // command rules happy: Vulkan queries/dynamic rendering and D3D12 direct-list queries/RT setup stay primary.
         bool hasRenderingScope = false;
         bool hasRenderArea = false;
         Rect2D renderArea = {};
         std::vector<RenderingAttachmentInfo> colorAttachments;
         std::vector<uint32_t> renderingAttachmentResources;
         RenderingAttachmentInfo depthAttachment = {};
         RenderingAttachmentInfo stencilAttachment = {};

         for (const RenderGraphPass::ResourceAccess& access : pass.GetResourceAccesses())
         {
            const uint32_t storageResourceIndex = GetStorageResourceIndex(access.m_handle);
            const Resource& resource = m_resources[storageResourceIndex];
            if (resource.m_type != RenderGraphResourceType::Image ||
                (!IsColorAttachmentUsage(access.m_usage) && !IsDepthStencilAttachmentUsage(access.m_usage)))
            {
               continue;
            }
            if (std::find(renderingAttachmentResources.begin(), renderingAttachmentResources.end(), storageResourceIndex) !=
                renderingAttachmentResources.end())
            {
               // A ReadWrite attachment can appear as both input and output. One rendering attachment entry is enough.
               continue;
            }
            renderingAttachmentResources.push_back(storageResourceIndex);

            Ptr<ImageView> imageView = resource.m_imageView;
            ASSERT(imageView != nullptr, "RenderGraph render attachment has no materialized ImageView");
            if (!hasRenderArea)
            {
               const glm::uvec3 extent = imageView->GetImageExtend();
               renderArea = Rect2D{.m_offset = glm::ivec2(0, 0), .m_extent = glm::uvec2(extent.x, extent.y)};
               hasRenderArea = true;
            }

            const ResourceUsageInfo usageInfo = ResourceUsageToInfo(access.m_usage, access.m_shaderStages);
            RenderingAttachmentInfo attachment{.m_imageView = imageView,
                                               .m_imageLayout = usageInfo.m_imageLayout,
                                               .m_loadOp = ResourceUsageReads(access.m_usage) ? AttachmentLoadOp::Load
                                                                                              : AttachmentLoadOp::DontCare,
                                               .m_storeOp = AttachmentStoreOp::Store};
            for (const RenderGraphPass::AttachmentClearInfo& clearInfo : pass.m_attachmentClears)
            {
               if (GetStorageResourceIndex(clearInfo.m_handle) == storageResourceIndex)
               {
                  // Clear metadata is applied when building BeginRendering, not recorded inside the pass callback.
                  attachment.m_loadOp = AttachmentLoadOp::Clear;
                  attachment.m_clearValue = clearInfo.m_clearValue;
                  break;
               }
            }

            if (IsColorAttachmentUsage(access.m_usage))
            {
               colorAttachments.push_back(std::move(attachment));
               hasRenderingScope = true;
               continue;
            }

            ASSERT(IsDepthStencilAttachmentUsage(access.m_usage), "RenderGraph render attachment has unsupported usage");
            const ImageAspectFlags aspectMask = imageView->GetAspectMask();
            if (any(aspectMask, ImageAspectFlags::Depth))
            {
               ASSERT(depthAttachment.m_imageView == nullptr, "RenderGraph pass has multiple depth attachments");
               depthAttachment = attachment;
               hasRenderingScope = true;
            }
            if (any(aspectMask, ImageAspectFlags::Stencil))
            {
               ASSERT(stencilAttachment.m_imageView == nullptr, "RenderGraph pass has multiple stencil attachments");
               stencilAttachment = attachment;
               hasRenderingScope = true;
            }
         }

         if (pass.m_timestamps.has_value())
         {
            const RenderGraphPass::PassTimestampInfo& timestamps = pass.m_timestamps.value();
            EnsureQueryReadbackBuffer(timestamps.m_beginResultState);
            EnsureQueryReadbackBuffer(timestamps.m_endResultState);
            // Vulkan requires a query reset before use; keeping this graph-owned also makes the sample API cleaner.
            p_commandBuffer.ResetQueries(timestamps.m_beginQuery->GetQueryPool(), timestamps.m_beginQueryIndex, 1u);
            p_commandBuffer.ResetQueries(timestamps.m_endQuery->GetQueryPool(), timestamps.m_endQueryIndex, 1u);
         }

         if (pass.m_query.has_value())
         {
            const RenderGraphPass::PassQueryInfo& query = pass.m_query.value();
            EnsureQueryReadbackBuffer(query.m_resultState);
            Ptr<GHI::Query> queryObject = query.m_query;
            // Queries are reset immediately before the pass that writes them, after all resource barriers.
            p_commandBuffer.ResetQueries(queryObject->GetQueryPool(), queryObject->GetQueryIndex(), 1u);
         }

         if (pass.m_timestamps.has_value())
         {
            const RenderGraphPass::PassTimestampInfo& timestamps = pass.m_timestamps.value();
            p_commandBuffer.WriteTimestamp(timestamps.m_beginQuery->GetQueryPool(), timestamps.m_beginQueryIndex,
                                           timestamps.m_beginStage);
         }

         if (hasRenderingScope)
         {
            ASSERT(hasRenderArea, "RenderGraph rendering scope needs a valid render area");
            p_commandBuffer.BeginRendering(renderArea, colorAttachments, depthAttachment, stencilAttachment);
         }

         if (pass.m_query.has_value())
         {
            const RenderGraphPass::PassQueryInfo& query = pass.m_query.value();
            p_commandBuffer.BeginQuery(query.m_query);
         }

         if (hasPassCommands)
         {
            // Only the already-recorded pass-local stream is executed here; all graph-owned commands remain primary.
            std::array<Ptr<SubCommandBuffer>, 1u> subCommandBuffers{passCommandBuffer};
            p_commandBuffer.ExecuteSubCommandBuffers(subCommandBuffers);
         }

         if (pass.m_query.has_value())
         {
            const RenderGraphPass::PassQueryInfo& query = pass.m_query.value();
            p_commandBuffer.EndQuery(query.m_query);
         }

         if (hasRenderingScope)
         {
            p_commandBuffer.EndRendering();
         }

         if (pass.m_timestamps.has_value())
         {
            const RenderGraphPass::PassTimestampInfo& timestamps = pass.m_timestamps.value();
            p_commandBuffer.WriteTimestamp(timestamps.m_endQuery->GetQueryPool(), timestamps.m_endQueryIndex,
                                           timestamps.m_endStage);
            // Resolve after the end timestamp so the promise points at host-readable data for this pass.
            p_commandBuffer.ResolveQueryData(timestamps.m_beginResultState);
            p_commandBuffer.ResolveQueryData(timestamps.m_endResultState);
         }

         if (pass.m_query.has_value())
         {
            const RenderGraphPass::PassQueryInfo& query = pass.m_query.value();
            // The query promise and command stream share the same QueryResultState.
            p_commandBuffer.ResolveQueryData(query.m_resultState);
         }
      }
   }
}

Ptr<ImageView> RenderGraph::GetImageView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph image handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Image,
          "RenderGraph handle does not reference an ImageView");
   // Logical versions resolve to the same backing view as their storage resource.
   return m_resources[GetStorageResourceIndex(p_handle)].m_imageView;
}

Ptr<BufferView> RenderGraph::GetBufferView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph buffer handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Buffer,
          "RenderGraph handle does not reference a BufferView");
   // Buffer queries also resolve through storage so Write/ReadWrite handles remain ergonomic in callbacks.
   return m_resources[GetStorageResourceIndex(p_handle)].m_bufferView;
}

const ImageDescriptor* RenderGraph::GetImageDescriptor(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph image descriptor handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Image,
          "RenderGraph handle does not reference an image resource");

   const std::optional<ImageDescriptor>& desc = m_resources[GetStorageResourceIndex(p_handle)].m_imageDesc;
   if (!desc.has_value())
   {
      return nullptr;
   }
   return &desc.value();
}

const BufferDescriptor* RenderGraph::GetBufferDescriptor(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph buffer descriptor handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Buffer,
          "RenderGraph handle does not reference a buffer resource");

   const std::optional<BufferDescriptor>& desc = m_resources[GetStorageResourceIndex(p_handle)].m_bufferDesc;
   if (!desc.has_value())
   {
      return nullptr;
   }
   return &desc.value();
}

bool RenderGraph::WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[GetStorageResourceIndex(p_handle)].m_createdInPrepare;
}

bool RenderGraph::CanResourceBeTransient(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[GetStorageResourceIndex(p_handle)].m_canBeTransient;
}

bool RenderGraph::CanResourceLifetimesAlias(RenderGraphResourceHandle p_first, RenderGraphResourceHandle p_second) const
{
   ASSERT(p_first.m_index < m_resources.size(), "First RenderGraph alias handle is out of range");
   ASSERT(p_second.m_index < m_resources.size(), "Second RenderGraph alias handle is out of range");

   const uint32_t firstStorageIndex = GetStorageResourceIndex(p_first);
   const uint32_t secondStorageIndex = GetStorageResourceIndex(p_second);
   if (firstStorageIndex == secondStorageIndex)
   {
      return false;
   }

   const Resource& firstResource = m_resources[firstStorageIndex];
   const Resource& secondResource = m_resources[secondStorageIndex];
   if (!firstResource.m_canBeTransient || !secondResource.m_canBeTransient)
   {
      // Imported resources, resources with initial data, and unbounded resources cannot share graph-managed memory.
      return false;
   }

   const bool firstHasLifetime = firstResource.m_firstUseOrder != RenderGraphResourceHandle::InvalidIndex &&
                                 firstResource.m_lastUseOrder != RenderGraphResourceHandle::InvalidIndex;
   const bool secondHasLifetime = secondResource.m_firstUseOrder != RenderGraphResourceHandle::InvalidIndex &&
                                  secondResource.m_lastUseOrder != RenderGraphResourceHandle::InvalidIndex;
   if (!firstHasLifetime || !secondHasLifetime)
   {
      return false;
   }

   // Non-overlap in solved execution order is the agnostic lifetime rule for aliasing.
   return firstResource.m_lastUseOrder < secondResource.m_firstUseOrder ||
          secondResource.m_lastUseOrder < firstResource.m_firstUseOrder;
}

bool RenderGraph::CanResourcesShareTransientAllocation(RenderGraphResourceHandle p_first, RenderGraphResourceHandle p_second) const
{
   if (!CanResourceLifetimesAlias(p_first, p_second))
   {
      return false;
   }

   const uint32_t firstStorageIndex = GetStorageResourceIndex(p_first);
   const uint32_t secondStorageIndex = GetStorageResourceIndex(p_second);
   const Resource& firstResource = m_resources[firstStorageIndex];
   const Resource& secondResource = m_resources[secondStorageIndex];

   const RenderGraphResourceHandle firstStorageHandle{.m_index = firstStorageIndex};
   const RenderGraphResourceHandle secondStorageHandle{.m_index = secondStorageIndex};
   if (m_transientCompatibilityChecker)
   {
      // Backends refine the lifetime rule with API/device memory compatibility.
      return m_transientCompatibilityChecker(firstStorageHandle, secondStorageHandle);
   }

   // Without backend data, only alias resources of the same broad type.
   return firstResource.m_type == secondResource.m_type;
}

bool RenderGraph::CanResourceJoinTransientAliasGroup(const std::vector<RenderGraphResourceHandle>& p_groupResources,
                                                     RenderGraphResourceHandle p_candidate) const
{
   ASSERT(p_candidate.m_index < m_resources.size(), "RenderGraph alias group candidate handle is out of range");

   const uint32_t candidateStorageIndex = GetStorageResourceIndex(p_candidate);
   const RenderGraphResourceHandle candidateStorageHandle{.m_index = candidateStorageIndex};
   if (!CanResourceBeTransient(candidateStorageHandle))
   {
      return false;
   }

   std::vector<RenderGraphResourceHandle> storageGroupResources;
   storageGroupResources.reserve(p_groupResources.size());
   for (const RenderGraphResourceHandle groupResource : p_groupResources)
   {
      ASSERT(groupResource.m_index < m_resources.size(), "RenderGraph alias group resource handle is out of range");

      const uint32_t groupStorageIndex = GetStorageResourceIndex(groupResource);
      const RenderGraphResourceHandle groupStorageHandle{.m_index = groupStorageIndex};
      if (!CanResourcesShareTransientAllocation(groupStorageHandle, candidateStorageHandle))
      {
         return false;
      }

      // The group request should contain storage handles only once, even if logical versions appear in the slot.
      const bool alreadyAdded = std::find_if(storageGroupResources.begin(), storageGroupResources.end(),
                                             [groupStorageIndex](RenderGraphResourceHandle p_handle) {
                                                return p_handle.m_index == groupStorageIndex;
                                             }) != storageGroupResources.end();
      if (!alreadyAdded)
      {
         storageGroupResources.push_back(groupStorageHandle);
      }
   }

   if (m_transientAliasGroupCompatibilityChecker)
   {
      // Some APIs need whole-slot compatibility, for example intersecting Vulkan memory type bits.
      return m_transientAliasGroupCompatibilityChecker(storageGroupResources, candidateStorageHandle);
   }

   return true;
}

uint64_t RenderGraph::GetTransientAllocationSize(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph transient allocation size handle is out of range");

   const uint32_t storageIndex = GetStorageResourceIndex(p_handle);
   const RenderGraphResourceHandle storageHandle{.m_index = storageIndex};
   if (m_transientAllocationSizeResolver)
   {
      const uint64_t resolvedSize = m_transientAllocationSizeResolver(storageHandle);
      ASSERT(resolvedSize > 0u, "RenderGraph transient allocation size resolver returned zero");
      return resolvedSize;
   }

   return EstimateTransientAllocationSize(storageHandle);
}

const std::vector<RenderGraphTransientResourceInfo>& RenderGraph::GetTransientResources() const
{
   return m_transientResources;
}

const std::vector<RenderGraphTransientAliasGroup>& RenderGraph::GetTransientAliasGroups() const
{
   return m_transientAliasGroups;
}

uint32_t RenderGraph::GetResourceFirstUseOrder(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[GetStorageResourceIndex(p_handle)].m_firstUseOrder;
}

uint32_t RenderGraph::GetResourceLastUseOrder(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[GetStorageResourceIndex(p_handle)].m_lastUseOrder;
}

RenderGraphResourceType RenderGraph::GetResourceType(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[p_handle.m_index].m_type;
}

std::string_view RenderGraph::GetResourceName(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[p_handle.m_index].m_name;
}

void RenderGraph::AddDependency(std::vector<std::vector<uint32_t>>& p_dependencies, uint32_t p_from, uint32_t p_to) const
{
   if (p_from == p_to)
   {
      return;
   }

   // Dependencies are stored as an adjacency list with duplicate edges folded away.
   std::vector<uint32_t>& passDependencies = p_dependencies[p_from];
   if (std::find(passDependencies.begin(), passDependencies.end(), p_to) == passDependencies.end())
   {
      passDependencies.push_back(p_to);
   }
}

RenderGraphResourceHandle RenderGraph::AddImageResource(std::string_view p_name, Ptr<ImageView> p_imageView,
                                                        std::optional<ImageDescriptor> p_desc, ResourceUsage p_initialUsage,
                                                        ShaderStageFlag p_initialShaderStages, QueueFamilyType p_initialQueue,
                                                        bool p_imported, bool p_passLocal, uint32_t p_ownerPass)
{
   ASSERT(p_imageView != nullptr || p_desc.has_value(), "RenderGraph image resources need a view or descriptor");

   // A resource record can be either imported (view exists now) or graph-owned (descriptor exists now).
   const uint32_t index = static_cast<uint32_t>(m_resources.size());
   m_resources.push_back(Resource{.m_name = std::string(p_name),
                                  .m_type = RenderGraphResourceType::Image,
                                  .m_imageView = std::move(p_imageView),
                                  .m_imageDesc = std::move(p_desc),
                                  .m_imported = p_imported,
                                  .m_passLocal = p_passLocal,
                                  .m_initialUsage = p_initialUsage,
                                  .m_initialShaderStages = p_initialShaderStages,
                                  .m_initialQueue = p_initialQueue,
                                  .m_storageResource = index,
                                  .m_ownerPass = p_ownerPass});
   if (!m_isPreparing)
   {
      m_compiled = false;
      m_prepared = false;
   }
   return RenderGraphResourceHandle{.m_index = index};
}

RenderGraphResourceHandle RenderGraph::AddBufferResource(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                                         std::optional<BufferDescriptor> p_desc, ResourceUsage p_initialUsage,
                                                         ShaderStageFlag p_initialShaderStages, QueueFamilyType p_initialQueue,
                                                         bool p_imported, bool p_passLocal, uint32_t p_ownerPass)
{
   ASSERT(p_bufferView != nullptr || p_desc.has_value(), "RenderGraph buffer resources need a view or descriptor");

   // Buffers use the same storage/version model as images.
   const uint32_t index = static_cast<uint32_t>(m_resources.size());
   m_resources.push_back(Resource{.m_name = std::string(p_name),
                                  .m_type = RenderGraphResourceType::Buffer,
                                  .m_bufferView = std::move(p_bufferView),
                                  .m_bufferDesc = std::move(p_desc),
                                  .m_imported = p_imported,
                                  .m_passLocal = p_passLocal,
                                  .m_initialUsage = p_initialUsage,
                                  .m_initialShaderStages = p_initialShaderStages,
                                  .m_initialQueue = p_initialQueue,
                                  .m_storageResource = index,
                                  .m_ownerPass = p_ownerPass});
   if (!m_isPreparing)
   {
      m_compiled = false;
      m_prepared = false;
   }
   return RenderGraphResourceHandle{.m_index = index};
}

RenderGraphResourceHandle RenderGraph::CreatePassTransientImage(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                                std::string_view p_name, ImageDescriptor p_desc,
                                                                ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   ASSERT(m_isPreparing, "RenderGraph pass-local images can only be created during Prepare");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass-local image uses an invalid ResourceUsage");

   RenderGraphResourceHandle handle = AddImageResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined,
                                                       ShaderStageFlag::All, p_pass.GetQueue(), false, true, p_pass.m_passIndex);

   // Prepare resources have no cross-pass producers, so their lifetime is exactly the owning pass order.
   Resource& resource = m_resources[handle.m_index];
   resource.m_createdInPrepare = true;
   resource.m_canBeTransient = true;
   resource.m_firstUseOrder = p_passOrder;
   resource.m_lastUseOrder = p_passOrder;

   // A transient group materializer needs the full post-prepare alias plan, so pass-local resources are
   // deferred when that backend hook is installed. The legacy single-resource materializer can still create
   // the view immediately for tests and simple backends.
   if (!m_transientMaterializer && m_imageMaterializer)
   {
      resource.m_imageView = m_imageMaterializer(resource.m_imageDesc.value(), resource.m_canBeTransient);
      ASSERT(resource.m_imageView != nullptr, "RenderGraph image materializer returned a null ImageView");
   }

   const RenderGraphPass::ResourceAccess access{.m_handle = handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   p_pass.m_transients.push_back(handle);
   p_pass.m_resourceAccesses.push_back(access);
   return handle;
}

RenderGraphResourceHandle RenderGraph::CreatePassTransientBuffer(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                                 std::string_view p_name, BufferDescriptor p_desc,
                                                                 ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   ASSERT(m_isPreparing, "RenderGraph pass-local buffers can only be created during Prepare");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass-local buffer uses an invalid ResourceUsage");

   RenderGraphResourceHandle handle = AddBufferResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined,
                                                        ShaderStageFlag::All, p_pass.GetQueue(), false, true, p_pass.m_passIndex);

   // Pass-local buffers also get a one-pass lifetime and participate in the same transient alias plan.
   Resource& resource = m_resources[handle.m_index];
   resource.m_createdInPrepare = true;
   resource.m_canBeTransient = true;
   resource.m_firstUseOrder = p_passOrder;
   resource.m_lastUseOrder = p_passOrder;

   if (!m_transientMaterializer && m_bufferMaterializer)
   {
      resource.m_bufferView = m_bufferMaterializer(resource.m_bufferDesc.value(), resource.m_canBeTransient);
      ASSERT(resource.m_bufferView != nullptr, "RenderGraph buffer materializer returned a null BufferView");
   }

   const RenderGraphPass::ResourceAccess access{.m_handle = handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   p_pass.m_transients.push_back(handle);
   p_pass.m_resourceAccesses.push_back(access);
   return handle;
}

RenderGraphResourceHandle RenderGraph::CreateResourceVersion(RenderGraphResourceHandle p_previousVersion, uint32_t p_producerPass)
{
   ASSERT(p_previousVersion.m_index < m_resources.size(), "RenderGraph version source handle is out of range");

   const Resource& previousResource = m_resources[p_previousVersion.m_index];
   const uint32_t storageResource = GetStorageResourceIndex(p_previousVersion);
   const uint32_t index = static_cast<uint32_t>(m_resources.size());

   // A logical version is a DAG node, not a new allocation. It points back to the same storage resource.
   m_resources.push_back(Resource{.m_name = previousResource.m_name,
                                  .m_type = previousResource.m_type,
                                  .m_initialUsage = ResourceUsage::Undefined,
                                  .m_initialShaderStages = ShaderStageFlag::All,
                                  .m_initialQueue = QueueFamilyType::Invalid,
                                  .m_producerPass = p_producerPass,
                                  .m_storageResource = storageResource,
                                  .m_previousVersion = p_previousVersion.m_index,
                                  .m_ownerPass = previousResource.m_ownerPass});

   if (!m_isPreparing)
   {
      m_compiled = false;
      m_prepared = false;
   }
   return RenderGraphResourceHandle{.m_index = index};
}

uint32_t RenderGraph::GetStorageResourceIndex(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");

   const Resource& resource = m_resources[p_handle.m_index];
   if (resource.m_storageResource == RenderGraphResourceHandle::InvalidIndex)
   {
      return p_handle.m_index;
   }

   // Storage indirection is what lets unique output handles coexist with one physical image/buffer.
   ASSERT(resource.m_storageResource < m_resources.size(), "RenderGraph resource storage handle is out of range");
   return resource.m_storageResource;
}

void RenderGraph::SetResourceProducer(RenderGraphResourceHandle p_handle, uint32_t p_passIndex)
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph producer handle is out of range");

   Resource& resource = m_resources[p_handle.m_index];
   ASSERT(resource.m_producerPass == RenderGraphResourceHandle::InvalidIndex || resource.m_producerPass == p_passIndex,
          "RenderGraph resource has multiple producer passes");

   resource.m_producerPass = p_passIndex;
   m_prepared = false;
}

void RenderGraph::MaterializeGraphResources()
{
   if (m_transientMaterializer)
   {
      // The transient materializer gets the solved alias slots, not arbitrary access to the graph internals.
      std::vector<RenderGraphTransientAliasGroupRequest> requests = BuildTransientMaterializationRequests();
      RenderGraphTransientResourceWriter writer(*this);
      m_transientMaterializer(requests, writer);
      ValidateTransientResourcesMaterialized();
   }

   const uint32_t resourceCount = static_cast<uint32_t>(m_resources.size());
   for (uint32_t resourceIndex = 0u; resourceIndex < resourceCount; ++resourceIndex)
   {
      Resource& resource = m_resources[resourceIndex];
      if (resource.m_storageResource != resourceIndex)
      {
         continue;
      }

      if (resource.m_imported)
      {
         continue;
      }

      if (resource.m_canBeTransient && m_transientMaterializer)
      {
         // Transient-capable resources are owned by the alias-slot materializer when it is installed.
         continue;
      }

      if (resource.m_type == RenderGraphResourceType::Image && resource.m_imageView == nullptr &&
          resource.m_imageDesc.has_value() && m_imageMaterializer)
      {
         // Non-transient or fallback graph-owned resources use the regular one-resource materializer.
         resource.m_imageView = m_imageMaterializer(resource.m_imageDesc.value(), resource.m_canBeTransient);
         ASSERT(resource.m_imageView != nullptr, "RenderGraph image materializer returned a null ImageView");
         resource.m_createdInPrepare = true;
         continue;
      }

      if (resource.m_type == RenderGraphResourceType::Buffer && resource.m_bufferView == nullptr &&
          resource.m_bufferDesc.has_value() && m_bufferMaterializer)
      {
         // Same fallback path for buffers when no alias-slot materializer is responsible for them.
         resource.m_bufferView = m_bufferMaterializer(resource.m_bufferDesc.value(), resource.m_canBeTransient);
         ASSERT(resource.m_bufferView != nullptr, "RenderGraph buffer materializer returned a null BufferView");
         resource.m_createdInPrepare = true;
      }
   }

   ValidateTransientResourcesMaterialized();
}

std::vector<RenderGraphTransientAliasGroupRequest> RenderGraph::BuildTransientMaterializationRequests() const
{
   // Requests are immutable backend input: the backend can create resources, but it cannot mutate graph structure.
   std::vector<RenderGraphTransientAliasGroupRequest> requests;
   requests.reserve(m_transientAliasGroups.size());

   for (const RenderGraphTransientAliasGroup& group : m_transientAliasGroups)
   {
      RenderGraphTransientAliasGroupRequest request{.m_firstUseOrder = group.m_firstUseOrder,
                                                    .m_lastUseOrder = group.m_lastUseOrder,
                                                    .m_allocationSize = group.m_allocationSize};
      request.m_resources.reserve(group.m_resources.size());

      for (const RenderGraphResourceHandle handle : group.m_resources)
      {
         const uint32_t storageIndex = GetStorageResourceIndex(handle);
         const Resource& resource = m_resources[storageIndex];
         ASSERT(resource.m_canBeTransient, "RenderGraph alias group contains a non-transient resource");

         // Descriptor pointers stay valid for the duration of materialization because resources are not appended here.
         request.m_resources.push_back(
             RenderGraphTransientResourceRequest{.m_handle = handle,
                                                 .m_type = resource.m_type,
                                                 .m_name = resource.m_name,
                                                 .m_imageDesc = resource.m_imageDesc ? &resource.m_imageDesc.value() : nullptr,
                                                 .m_bufferDesc = resource.m_bufferDesc ? &resource.m_bufferDesc.value() : nullptr,
                                                 .m_firstUseOrder = resource.m_firstUseOrder,
                                                 .m_lastUseOrder = resource.m_lastUseOrder,
                                                 .m_allocationSize = GetTransientAllocationSize(handle)});
      }

      requests.push_back(std::move(request));
   }

   return requests;
}

void RenderGraph::ValidateTransientResourcesMaterialized() const
{
   // This is the hard contract for the backend: every transient request must become a concrete ImageView/BufferView.
   for (const RenderGraphTransientResourceInfo& transientResource : m_transientResources)
   {
      const uint32_t storageIndex = GetStorageResourceIndex(transientResource.m_handle);
      const Resource& resource = m_resources[storageIndex];
      ASSERT(resource.m_canBeTransient, "RenderGraph transient list contains a non-transient resource");

      if (resource.m_type == RenderGraphResourceType::Image)
      {
         ASSERT(resource.m_imageView != nullptr, "RenderGraph transient image was not materialized");
         continue;
      }

      ASSERT(resource.m_type == RenderGraphResourceType::Buffer, "Unsupported RenderGraph transient resource type");
      ASSERT(resource.m_bufferView != nullptr, "RenderGraph transient buffer was not materialized");
   }
}

void RenderGraph::UpdateResourceLifetimes()
{
   // Lifetime orders are indices into m_executionOrder, not original pass insertion indices.
   for (Resource& resource : m_resources)
   {
      resource.m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
      resource.m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   }

   for (size_t orderIndex = 0u; orderIndex < m_executionOrder.size(); ++orderIndex)
   {
      const RenderGraphPass& pass = m_passes[m_executionOrder[orderIndex]];
      for (const RenderGraphPass::ResourceAccess& access : pass.GetResourceAccesses())
      {
         // Track the logical handle's lifetime first, then stretch the backing storage lifetime below.
         Resource& resource = m_resources[access.m_handle.m_index];
         if (resource.m_firstUseOrder == RenderGraphResourceHandle::InvalidIndex)
         {
            resource.m_firstUseOrder = static_cast<uint32_t>(orderIndex);
         }
         resource.m_lastUseOrder = static_cast<uint32_t>(orderIndex);

         // Logical versions share storage, so the storage resource lifetime spans all version accesses.
         Resource& storageResource = m_resources[GetStorageResourceIndex(access.m_handle)];
         if (&storageResource != &resource)
         {
            if (storageResource.m_firstUseOrder == RenderGraphResourceHandle::InvalidIndex)
            {
               storageResource.m_firstUseOrder = static_cast<uint32_t>(orderIndex);
            }
            storageResource.m_lastUseOrder = static_cast<uint32_t>(orderIndex);
         }
      }
   }
}

void RenderGraph::AnalyzeTransientResources()
{
   // Transient eligibility is derived after ordering because first/last use determines whether aliasing is legal.
   const uint32_t resourceCount = static_cast<uint32_t>(m_resources.size());
   for (uint32_t resourceIndex = 0u; resourceIndex < resourceCount; ++resourceIndex)
   {
      Resource& resource = m_resources[resourceIndex];
      if (resource.m_storageResource != resourceIndex)
      {
         // Logical versions never own allocations. Only their storage resource can be transient.
         resource.m_canBeTransient = false;
         continue;
      }

      const bool hasDescriptor = resource.m_imageDesc.has_value() || resource.m_bufferDesc.has_value();
      const bool hasGraphLifetime = resource.m_firstUseOrder != RenderGraphResourceHandle::InvalidIndex &&
                                    resource.m_lastUseOrder != RenderGraphResourceHandle::InvalidIndex;
      const bool hasInitialData = (resource.m_imageDesc.has_value() && resource.m_imageDesc.value().m_initialData != nullptr &&
                                   resource.m_imageDesc.value().m_initialDataSize > 0u) ||
                                  (resource.m_bufferDesc.has_value() && resource.m_bufferDesc.value().m_initialData != nullptr &&
                                   resource.m_bufferDesc.value().m_initialDataSize > 0u);

      // Transient means graph-owned, descriptor-backed, and bounded by the solved execution timeline.
      resource.m_canBeTransient = !resource.m_imported && hasDescriptor && hasGraphLifetime && !hasInitialData;
   }

   UpdateTransientAliasing();
}

void RenderGraph::UpdateTransientAliasing()
{
   // The transient resource list is allocator input. Alias groups are the selected schedule.
   m_transientResources.clear();
   m_transientAliasGroups.clear();

   const uint32_t resourceCount = static_cast<uint32_t>(m_resources.size());
   for (uint32_t resourceIndex = 0u; resourceIndex < resourceCount; ++resourceIndex)
   {
      const Resource& resource = m_resources[resourceIndex];
      if (resource.m_storageResource != resourceIndex || !resource.m_canBeTransient)
      {
         continue;
      }

      m_transientResources.push_back(RenderGraphTransientResourceInfo{
          .m_handle = RenderGraphResourceHandle{.m_index = resourceIndex},
          .m_type = resource.m_type,
          .m_firstUseOrder = resource.m_firstUseOrder,
          .m_lastUseOrder = resource.m_lastUseOrder,
          .m_allocationSize = GetTransientAllocationSize(RenderGraphResourceHandle{.m_index = resourceIndex})});
   }

   std::sort(m_transientResources.begin(), m_transientResources.end(),
             [](const RenderGraphTransientResourceInfo& p_lhs, const RenderGraphTransientResourceInfo& p_rhs) {
                // Stable lifetime ordering makes alias choices deterministic across runs.
                if (p_lhs.m_firstUseOrder != p_rhs.m_firstUseOrder)
                {
                   return p_lhs.m_firstUseOrder < p_rhs.m_firstUseOrder;
                }

                if (p_lhs.m_lastUseOrder != p_rhs.m_lastUseOrder)
                {
                   return p_lhs.m_lastUseOrder < p_rhs.m_lastUseOrder;
                }

                return p_lhs.m_handle.m_index < p_rhs.m_handle.m_index;
             });

   // Linear-scan best-fit scheduling:
   // - process resources by first use
   // - reuse a slot only after its last use
   // - require backend compatibility with every resource already in the slot
   // - choose the slot with the smallest allocation growth, then least waste, then latest previous use
   for (const RenderGraphTransientResourceInfo& resourceInfo : m_transientResources)
   {
      uint32_t bestGroupIndex = RenderGraphResourceHandle::InvalidIndex;
      uint64_t bestAddedCost = 0u;
      uint64_t bestWastedSize = 0u;
      uint32_t bestGroupLastUse = 0u;

      for (uint32_t groupIndex = 0u; groupIndex < m_transientAliasGroups.size(); ++groupIndex)
      {
         const RenderGraphTransientAliasGroup& group = m_transientAliasGroups[groupIndex];
         ASSERT(!group.m_resources.empty(), "RenderGraph transient alias group can't be empty");

         if (group.m_lastUseOrder >= resourceInfo.m_firstUseOrder)
         {
            continue;
         }

         if (!CanResourceJoinTransientAliasGroup(group.m_resources, resourceInfo.m_handle))
         {
            continue;
         }

         const uint64_t newAllocationSize = std::max(group.m_allocationSize, resourceInfo.m_allocationSize);
         const uint64_t addedCost = newAllocationSize - group.m_allocationSize;
         const uint64_t wastedSize = newAllocationSize - resourceInfo.m_allocationSize;

         const bool isBetterGroup =
             bestGroupIndex == RenderGraphResourceHandle::InvalidIndex || addedCost < bestAddedCost ||
             (addedCost == bestAddedCost && wastedSize < bestWastedSize) ||
             (addedCost == bestAddedCost && wastedSize == bestWastedSize && group.m_lastUseOrder > bestGroupLastUse);
         if (isBetterGroup)
         {
            bestGroupIndex = groupIndex;
            bestAddedCost = addedCost;
            bestWastedSize = wastedSize;
            bestGroupLastUse = group.m_lastUseOrder;
         }
      }

      if (bestGroupIndex == RenderGraphResourceHandle::InvalidIndex)
      {
         // No existing slot can accept the resource, so it starts a new allocation slot.
         m_transientAliasGroups.push_back(RenderGraphTransientAliasGroup{.m_resources = {resourceInfo.m_handle},
                                                                         .m_firstUseOrder = resourceInfo.m_firstUseOrder,
                                                                         .m_lastUseOrder = resourceInfo.m_lastUseOrder,
                                                                         .m_allocationSize = resourceInfo.m_allocationSize});
         continue;
      }

      RenderGraphTransientAliasGroup& bestGroup = m_transientAliasGroups[bestGroupIndex];
      // Extending the slot turns several disjoint resources into one backend allocation request.
      bestGroup.m_resources.push_back(resourceInfo.m_handle);
      bestGroup.m_firstUseOrder = std::min(bestGroup.m_firstUseOrder, resourceInfo.m_firstUseOrder);
      bestGroup.m_lastUseOrder = std::max(bestGroup.m_lastUseOrder, resourceInfo.m_lastUseOrder);
      bestGroup.m_allocationSize = std::max(bestGroup.m_allocationSize, resourceInfo.m_allocationSize);
   }
}

uint64_t RenderGraph::EstimateTransientAllocationSize(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph transient allocation estimate handle is out of range");

   const Resource& resource = m_resources[GetStorageResourceIndex(p_handle)];
   if (resource.m_type == RenderGraphResourceType::Buffer)
   {
      ASSERT(resource.m_bufferDesc.has_value(), "RenderGraph transient buffer needs a descriptor for size estimation");
      return std::max<uint64_t>(resource.m_bufferDesc.value().m_requestBufferSize, 1u);
   }

   ASSERT(resource.m_type == RenderGraphResourceType::Image, "Unsupported RenderGraph transient resource type");
   ASSERT(resource.m_imageDesc.has_value(), "RenderGraph transient image needs a descriptor for size estimation");

   // This is intentionally rough. Real APIs should install a resolver that returns device memory requirements.
   const ImageDescriptor& desc = resource.m_imageDesc.value();
   const uint64_t width = std::max<uint32_t>(desc.m_extend.x, 1u);
   const uint64_t height = std::max<uint32_t>(desc.m_extend.y, 1u);
   const uint64_t depth = std::max<uint32_t>(desc.m_extend.z, 1u);
   const uint64_t layers = std::max<uint32_t>(desc.m_arrayLayers, 1u);
   const uint64_t mips = std::max<uint32_t>(desc.m_mipLevels, 1u);
   return std::max<uint64_t>(width * height * depth * layers * mips * GetResourceFormatByteSize(desc.m_format), 1u);
}

Ptr<SubCommandBuffer> RenderGraph::CreateSubCommandBuffer(Ptr<Device> p_device) const
{
   ASSERT(p_device != nullptr, "RenderGraph pass-local command recording needs a Device");

   if (m_subCommandBufferCreator)
   {
      Ptr<SubCommandBuffer> subCommandBuffer = m_subCommandBufferCreator(std::move(p_device));
      ASSERT(subCommandBuffer != nullptr, "RenderGraph sub command buffer creator returned null");
      return subCommandBuffer;
   }

   // Unit tests can exercise the agnostic graph without installing a platform ResourceFactory.
   // Real backends should install a creator so the recorded pass stream owns a native secondary/bundle handle.
   return std::make_shared<RecordedSubCommandBuffer>();
}

Ptr<Buffer> RenderGraph::CreateQueryReadbackBuffer(Ptr<Device> p_device, uint64_t p_size) const
{
   ASSERT(p_device != nullptr, "RenderGraph query readback needs a Device");
   ASSERT(p_size > 0u, "RenderGraph query readback Buffer needs a non-zero size");

   const BufferDescriptor desc{.m_requestBufferSize = p_size,
                               .m_bufferUsageFlags = BufferUsageFlags::TransferDestination,
                               .m_queueFamilyAccess = QueueTypeFlags::AllQueues,
                               .m_memoryProperties = MemoryPropertyFlags::HostVisible | MemoryPropertyFlags::HostCoherent};

   if (m_queryReadbackBufferCreator)
   {
      Ptr<Buffer> buffer = m_queryReadbackBufferCreator(std::move(p_device), desc);
      ASSERT(buffer != nullptr, "RenderGraph query readback Buffer creator returned null");
      return buffer;
   }

   ASSERT(ResourceFactory::Get() != nullptr,
          "RenderGraph query readback needs either a readback Buffer creator or a ResourceFactory");
   BufferDescriptor mutableDesc = desc;
   return ResourceFactory::Get()->CreateBuffer(std::move(p_device), std::move(mutableDesc));
}

void RenderGraph::EnsureQueryReadbackBuffer(const Ptr<QueryResultState>& p_queryResult) const
{
   ASSERT(p_queryResult != nullptr, "RenderGraph query readback needs a valid QueryResultState");
   if (p_queryResult->HasReadbackBuffer())
   {
      return;
   }

   Ptr<GHI::Query> query = p_queryResult->GetQuery();
   p_queryResult->SetReadbackBuffer(CreateQueryReadbackBuffer(query->GetDevice(), p_queryResult->GetReadbackSize()));
}

QueueFamilyInfo RenderGraph::ResolveQueueFamilyInfo(QueueFamilyType p_queueType) const
{
   // Queue-family resolution is backend data. The fallback is only useful for tests and single-family backends.
   if (p_queueType == QueueFamilyType::Invalid)
   {
      return QueueFamilyInfo{};
   }

   if (m_queueFamilyResolver)
   {
      return m_queueFamilyResolver(p_queueType);
   }

   return QueueFamilyInfo{.m_queueType = p_queueType,
                          .m_supportedQueues = QueueFamilyTypeToQueueTypeFlags(p_queueType),
                          .m_familyIndex = static_cast<uint32_t>(p_queueType),
                          .m_queueIndex = 0u};
}

void RenderGraph::EmitBarrier(CommandBuffer& p_commandBuffer, const Resource& p_resource, ResourceState p_oldState,
                              ResourceState p_newState) const
{
   ASSERT(m_barrierEmitter, "RenderGraph needs a backend barrier emitter before it can execute transitions");

   // The graph passes semantic old/new states to the backend; API-specific access masks/layouts live there.
   m_barrierEmitter(p_commandBuffer, RenderGraphBarrierInfo{.m_resourceType = p_resource.m_type,
                                                            .m_imageView = p_resource.m_imageView,
                                                            .m_bufferView = p_resource.m_bufferView,
                                                            .m_oldUsage = p_oldState.m_usage,
                                                            .m_newUsage = p_newState.m_usage,
                                                            .m_oldShaderStages = p_oldState.m_shaderStages,
                                                            .m_newShaderStages = p_newState.m_shaderStages,
                                                            .m_oldQueue = p_oldState.m_queue,
                                                            .m_newQueue = p_newState.m_queue,
                                                            .m_oldQueueFamily = ResolveQueueFamilyInfo(p_oldState.m_queue),
                                                            .m_newQueueFamily = ResolveQueueFamilyInfo(p_newState.m_queue)});
}

void RenderGraph::ValidateResourceAccess(const Resource& p_resource, const RenderGraphPass::ResourceAccess& p_access) const
{
   // Validate only cross-cutting semantic mistakes here. Backend-specific restrictions should stay in backend hooks.
   const ResourceUsageInfo usageInfo = ResourceUsageToInfo(p_access.m_usage, p_access.m_shaderStages);

   if (p_resource.m_type == RenderGraphResourceType::Image)
   {
      ASSERT(usageInfo.m_imageLayout != ImageLayout::Invalid, "ResourceUsage is not valid for an image resource");
   }
}

} // namespace GHI

} // namespace Render
