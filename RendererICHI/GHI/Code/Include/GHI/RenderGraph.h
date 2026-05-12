#pragma once

#include <array>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/Query.h>
#include <GHI/QueryResult.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

// RenderGraph is deliberately split into API-agnostic scheduling and backend-provided execution details.
// This file owns logical resources, pass dependencies, transient lifetime analysis, query promises, and the
// high-level primary/subcommand recording policy. Vulkan/D3D12-specific layout masks, memory requirements,
// native command-buffer allocation, and transient materialization are injected through hooks.

class BufferView;
class CommandBuffer;
class Device;
class ImageView;
class Query;
class QueryPool;
class RenderGraph;
class RenderGraphPrepareContext;
class RenderGraphPass;
class SubCommandBuffer;
class SubCommandRecorder;
class RenderGraphTransientResourceWriter;

struct RenderGraphResourceHandle
{
   static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);

   // Handles are logical graph resources. Several handles can refer to the same backing storage after Write/ReadWrite.
   uint32_t m_index = InvalidIndex;

   bool IsValid() const { return m_index != InvalidIndex; }
};

enum class RenderGraphResourceType : uint32_t
{
   Image = 0u,
   Buffer,

   Count,
   Invalid = Count,
};

struct RenderGraphBarrierInfo
{
   RenderGraphResourceType m_resourceType = RenderGraphResourceType::Invalid;
   Ptr<ImageView> m_imageView;
   Ptr<BufferView> m_bufferView;
   ResourceUsage m_oldUsage = ResourceUsage::Undefined;
   ResourceUsage m_newUsage = ResourceUsage::Undefined;
   ShaderStageFlag m_oldShaderStages = ShaderStageFlag::All;
   ShaderStageFlag m_newShaderStages = ShaderStageFlag::All;
   QueueFamilyType m_oldQueue = QueueFamilyType::Invalid;
   QueueFamilyType m_newQueue = QueueFamilyType::Invalid;
   QueueFamilyInfo m_oldQueueFamily;
   QueueFamilyInfo m_newQueueFamily;

   // Backends use this to distinguish an ordinary layout/access barrier from a queue-family ownership transfer.
   bool RequiresQueueFamilyOwnershipTransfer() const
   {
      return m_oldQueueFamily.IsValid() && m_newQueueFamily.IsValid() &&
             !m_oldQueueFamily.SharesQueueFamilyWith(m_newQueueFamily);
   }
};

// A transient resource is graph-owned storage with a lifetime fully described by the solved pass order.
// The backend can use this as allocator input without needing to understand the pass DAG.
struct RenderGraphTransientResourceInfo
{
   RenderGraphResourceHandle m_handle;
   RenderGraphResourceType m_type = RenderGraphResourceType::Invalid;
   uint32_t m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint32_t m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint64_t m_allocationSize = 0u;
};

// Alias groups are the graph's lifetime/size-aware allocation slots. A backend may split these further
// if device memory requirements disagree, but resources in one group never overlap in graph time.
struct RenderGraphTransientAliasGroup
{
   std::vector<RenderGraphResourceHandle> m_resources;
   uint32_t m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint32_t m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint64_t m_allocationSize = 0u;
};

struct RenderGraphTransientResourceRequest
{
   // Immutable description passed to a backend transient materializer. The materializer responds through
   // RenderGraphTransientResourceWriter instead of mutating graph state directly.
   RenderGraphResourceHandle m_handle;
   RenderGraphResourceType m_type = RenderGraphResourceType::Invalid;
   std::string_view m_name;
   const ImageDescriptor* m_imageDesc = nullptr;
   const BufferDescriptor* m_bufferDesc = nullptr;
   uint32_t m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint32_t m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint64_t m_allocationSize = 0u;
};

struct RenderGraphTransientAliasGroupRequest
{
   // One alias group corresponds to one backend allocation slot. The resources in this list have disjoint
   // graph lifetimes and have already passed the graph/backend compatibility checks.
   std::vector<RenderGraphTransientResourceRequest> m_resources;
   uint32_t m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint32_t m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   uint64_t m_allocationSize = 0u;
};

ImageViewDescriptor CreateDefaultRenderGraphImageViewDescriptor(Ptr<Image> p_image, const ImageDescriptor& p_desc);
BufferViewDescriptor CreateDefaultRenderGraphBufferViewDescriptor(Ptr<Buffer> p_buffer, const BufferDescriptor& p_desc);

class RenderGraphContext final
{
 public:
   RenderGraphContext() = delete;

   // Execute callbacks record pass-local commands. The RenderGraph owns primary command orchestration.
   SubCommandRecorder& GetRecorder() const;
   std::string_view GetPassName() const;
   // Inputs/outputs are exactly the resources declared on the pass builder. Transients are pass-local prepare resources.
   size_t GetInputCount() const;
   size_t GetOutputCount() const;
   size_t GetTransientCount() const;
   RenderGraphResourceHandle GetInput(size_t p_index) const;
   RenderGraphResourceHandle GetOutput(size_t p_index) const;
   RenderGraphResourceHandle GetTransient(size_t p_index) const;
   RenderGraphResourceHandle Input(std::string_view p_name) const;
   RenderGraphResourceHandle Output(std::string_view p_name) const;
   Ptr<ImageView> GetImageView(RenderGraphResourceHandle p_handle) const;
   Ptr<BufferView> GetBufferView(RenderGraphResourceHandle p_handle) const;

 private:
   friend class RenderGraph;

   RenderGraphContext(RenderGraph& p_graph, RenderGraphPass& p_pass, SubCommandRecorder& p_recorder);

 private:
   RenderGraph* m_graph = nullptr;
   RenderGraphPass* m_pass = nullptr;
   SubCommandRecorder* m_recorder = nullptr;
};

class RenderGraphPrepareContext final
{
 public:
   RenderGraphPrepareContext() = delete;

   // Prepare runs after the DAG is solved and before Execute. It is meant for pass-local scratch resources
   // and setup that needs materialized graph resources, not for declaring cross-pass dependencies.
   std::string_view GetPassName() const;
   size_t GetInputCount() const;
   size_t GetOutputCount() const;
   size_t GetTransientCount() const;
   RenderGraphResourceHandle GetInput(size_t p_index) const;
   RenderGraphResourceHandle GetOutput(size_t p_index) const;
   RenderGraphResourceHandle GetTransient(size_t p_index) const;
   RenderGraphResourceHandle Input(std::string_view p_name) const;
   RenderGraphResourceHandle Output(std::string_view p_name) const;

   Ptr<ImageView> GetImageView(RenderGraphResourceHandle p_handle) const;
   Ptr<BufferView> GetBufferView(RenderGraphResourceHandle p_handle) const;
   const ImageDescriptor* GetImageDescriptor(RenderGraphResourceHandle p_handle) const;
   const BufferDescriptor* GetBufferDescriptor(RenderGraphResourceHandle p_handle) const;
   bool WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const;
   bool CanResourceBeTransient(RenderGraphResourceHandle p_handle) const;
   // ClearAttachment only mutates rendering metadata for outputs already declared on this pass.
   // It does not create dependencies, resources, or pass-local command work.
   void ClearAttachment(RenderGraphResourceHandle p_handle, ClearColorValue p_clearValue);
   void ClearAttachment(size_t p_outputIndex, ClearColorValue p_clearValue);
   void ClearAttachment(std::string_view p_outputName, ClearColorValue p_clearValue);

   // Creates pass-local scratch storage. These resources are visible to this pass only and can be transient.
   RenderGraphResourceHandle CreateTransientImage(std::string_view p_name, ImageDescriptor p_desc,
                                                  ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                                  ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphResourceHandle CreateTransientBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                   ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                                   ShaderStageFlag p_shaderStages = ShaderStageFlag::All);

 private:
   friend class RenderGraph;

   RenderGraphPrepareContext(RenderGraph& p_graph, RenderGraphPass& p_pass, uint32_t p_passOrder);

 private:
   RenderGraph* m_graph = nullptr;
   RenderGraphPass* m_pass = nullptr;
   uint32_t m_passOrder = RenderGraphResourceHandle::InvalidIndex;
};

using RenderGraphPrepareCallback = std::function<void(RenderGraphPrepareContext&)>;
using RenderGraphExecuteCallback = std::function<void(RenderGraphContext&)>;
// Materializers create concrete views for graph-owned descriptor resources. The bool tells the backend
// whether the graph has proven the resource is transient-capable before allocation happens.
using RenderGraphImageMaterializer = std::function<Ptr<ImageView>(const ImageDescriptor&, bool)>;
using RenderGraphBufferMaterializer = std::function<Ptr<BufferView>(const BufferDescriptor&, bool)>;
using RenderGraphImageViewMaterializer = std::function<Ptr<ImageView>(Ptr<Image>, const ImageDescriptor&)>;
using RenderGraphBufferViewMaterializer = std::function<Ptr<BufferView>(Ptr<Buffer>, const BufferDescriptor&)>;
// Barrier emission is backend-owned because layouts/access masks differ per graphics API.
using RenderGraphBarrierEmitter = std::function<void(CommandBuffer&, const RenderGraphBarrierInfo&)>;
using RenderGraphQueueFamilyResolver = std::function<QueueFamilyInfo(QueueFamilyType)>;
// Backend-owned memory rules stay behind this hook; the graph only schedules resources that pass it.
using RenderGraphTransientCompatibilityChecker =
    std::function<bool(RenderGraphResourceHandle, RenderGraphResourceHandle)>;
using RenderGraphTransientAliasGroupCompatibilityChecker =
    std::function<bool(const std::vector<RenderGraphResourceHandle>&, RenderGraphResourceHandle)>;
// Backends can provide exact memory requirements. The agnostic fallback is only a rough descriptor estimate.
using RenderGraphTransientAllocationSizeResolver = std::function<uint64_t(RenderGraphResourceHandle)>;
// Backend-owned materialization for transient alias groups. This is where API-specific shared-memory binding happens.
using RenderGraphTransientMaterializer =
    std::function<void(std::span<const RenderGraphTransientAliasGroupRequest>, RenderGraphTransientResourceWriter&)>;
using RenderGraphSubCommandBufferCreator = std::function<Ptr<SubCommandBuffer>(Ptr<Device>)>;
using RenderGraphQueryReadbackBufferCreator = std::function<Ptr<Buffer>(Ptr<Device>, const BufferDescriptor&)>;

class RenderGraphQuery final
{
 public:
   RenderGraphQuery() = default;

   // Query promises are returned while building the graph. They become readable after graph execution resolves
   // the query into the graph-owned readback buffer.
   bool IsValid() const;
   Ptr<GHI::Query> GetQuery() const;
   Ptr<Buffer> GetReadbackBuffer() const;

   std::optional<QueryReadbackData> Readback() const;
   QueryReadbackData ReadbackWait() const;

   // Continuation helpers let a query-producing call stay in the fluent pass chain.
   RenderGraphQuery Execute(RenderGraphExecuteCallback p_execute);
   RenderGraphQuery Prepare(RenderGraphPrepareCallback p_prepare);
   RenderGraphQuery NeverCull();

 private:
   friend class RenderGraphPass;

   RenderGraphQuery(RenderGraphPass& p_pass, Ptr<QueryResultState> p_state);

 private:
   RenderGraphPass* m_pass = nullptr;
   Ptr<QueryResultState> m_state;
};

class RenderGraphTimestampQuery final
{
 public:
   RenderGraphTimestampQuery() = default;

   // Timestamp promises wrap two timestamp queries: begin and end. Readback returns both values in order.
   bool IsValid() const;
   Ptr<GHI::Query> GetBeginQuery() const;
   Ptr<GHI::Query> GetEndQuery() const;
   Ptr<Buffer> GetBeginReadbackBuffer() const;
   Ptr<Buffer> GetEndReadbackBuffer() const;

   std::optional<QueryReadbackData> Readback() const;
   QueryReadbackData ReadbackWait() const;

   RenderGraphTimestampQuery Execute(RenderGraphExecuteCallback p_execute);
   RenderGraphTimestampQuery Prepare(RenderGraphPrepareCallback p_prepare);
   RenderGraphTimestampQuery NeverCull();
   // A timestamp-producing chain can append a regular begin/end query to the same pass.
   RenderGraphQuery WriteQuery(QueryDescriptor p_desc);
   RenderGraphQuery WriteQuery(Ptr<QueryPool> p_queryPool,
                               QueryControlFlags p_controlFlags = QueryControlFlags::None);
   RenderGraphQuery WriteQuery(Ptr<GHI::Query> p_query);
   RenderGraphQuery WriteQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex,
                               QueryControlFlags p_controlFlags = QueryControlFlags::None);

 private:
   friend class RenderGraphPass;

   RenderGraphTimestampQuery(RenderGraphPass& p_pass, Ptr<QueryResultState> p_beginState,
                             Ptr<QueryResultState> p_endState);

 private:
   RenderGraphPass* m_pass = nullptr;
   Ptr<QueryResultState> m_beginState;
   Ptr<QueryResultState> m_endState;
};

template <size_t t_count, typename... t_extras>
class RenderGraphOutputList final
{
 public:
   // Output lists are tuple-like builder continuations. The first t_count values are resource handles; extra
   // values are promises appended by WriteTimestamps/WriteQuery so structured bindings stay compact.
   RenderGraphOutputList(RenderGraphPass& p_pass, std::array<RenderGraphResourceHandle, t_count> p_handles,
                         std::tuple<t_extras...> p_extras = std::tuple<t_extras...>{})
       : m_pass(&p_pass), m_handles(p_handles), m_extras(std::move(p_extras))
   {
   }

   // Fluent continuation after a write-producing call. These calls still mutate the same pass.
   RenderGraphOutputList& Read(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                               ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList& Read(std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                               ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> Write(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                                         ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> Write(std::string_view p_name,
                                                         RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                                         ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> ReadWrite(RenderGraphResourceHandle p_handle,
                                                             ResourceUsage p_usage,
                                                             ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> ReadWrite(std::string_view p_name,
                                                             RenderGraphResourceHandle p_handle,
                                                             ResourceUsage p_usage,
                                                             ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> WriteImage(
       std::string_view p_name, ImageDescriptor p_desc, ResourceUsage p_usage = ResourceUsage::ColorAttachmentWrite,
       ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> WriteBuffer(
       std::string_view p_name, BufferDescriptor p_desc, ResourceUsage p_usage = ResourceUsage::StorageWrite,
       ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> ReadWriteImage(
       std::string_view p_name, ImageDescriptor p_desc, ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
       ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1, t_extras...> ReadWriteBuffer(
       std::string_view p_name, BufferDescriptor p_desc, ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
       ShaderStageFlag p_shaderStages = ShaderStageFlag::All);

   // Terminal-style callbacks return the output list so structured bindings still work.
   RenderGraphOutputList& Execute(RenderGraphExecuteCallback p_execute);
   RenderGraphOutputList& Prepare(RenderGraphPrepareCallback p_prepare);
   RenderGraphOutputList& NeverCull();
   RenderGraphOutputList& ClearAttachment(size_t p_outputIndex, ClearColorValue p_clearValue);
   template <size_t t_index>
   RenderGraphOutputList& ClearAttachment(ClearColorValue p_clearValue);
   RenderGraphOutputList<t_count, t_extras..., RenderGraphQuery> WriteQuery(Ptr<GHI::Query> p_query);
   RenderGraphOutputList<t_count, t_extras..., RenderGraphQuery> WriteQuery(
       Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex,
       QueryControlFlags p_controlFlags = QueryControlFlags::None);
   RenderGraphOutputList<t_count, t_extras..., RenderGraphTimestampQuery> WriteTimestamps(
       Ptr<GHI::Query> p_beginQuery, Ptr<GHI::Query> p_endQuery,
       PipelineStageFlags p_beginStage = PipelineStageFlags::TopOfPipe,
       PipelineStageFlags p_endStage = PipelineStageFlags::BottomOfPipe);
   RenderGraphOutputList<t_count, t_extras..., RenderGraphTimestampQuery> WriteTimestamps(
       Ptr<QueryPool> p_queryPool, uint32_t p_beginQueryIndex, uint32_t p_endQueryIndex,
       PipelineStageFlags p_beginStage = PipelineStageFlags::TopOfPipe,
       PipelineStageFlags p_endStage = PipelineStageFlags::BottomOfPipe);
   RenderGraphResourceHandle Input(std::string_view p_name) const;
   RenderGraphResourceHandle Output(std::string_view p_name) const;

   template <size_t t_index>
   auto Get() const
   {
      // Indices before t_count read resource handles. Later indices read appended promise values.
      static_assert(t_index < t_count + sizeof...(t_extras));
      if constexpr (t_index < t_count)
      {
         return m_handles[t_index];
      }
      else
      {
         return std::get<t_index - t_count>(m_extras);
      }
   }

 private:
   template <typename t_extra>
   RenderGraphOutputList<t_count, t_extras..., t_extra> Append(t_extra p_extra) const;

 private:
   RenderGraphPass* m_pass = nullptr;
   std::array<RenderGraphResourceHandle, t_count> m_handles;
   std::tuple<t_extras...> m_extras;
};

class RenderGraphPass final
{
 public:
   RenderGraphPass(RenderGraph& p_graph, uint32_t p_passIndex, std::string_view p_name);

   // Read consumes an existing logical handle. Write and ReadWrite return a new logical version,
   // which is what later passes should read to express the DAG edge.
   RenderGraphPass& Read(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                         ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphPass& Read(std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                         ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> Write(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                  ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> Write(std::string_view p_name, RenderGraphResourceHandle p_handle,
                                  ResourceUsage p_usage, ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> ReadWrite(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                      ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> ReadWrite(std::string_view p_name, RenderGraphResourceHandle p_handle,
                                      ResourceUsage p_usage, ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphPass& Use(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                        ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   // Selects the queue this pass records for. Single-command-buffer execution currently requires all
   // executed passes to match the provided command buffer queue.
   RenderGraphPass& Queue(QueueFamilyType p_queue);
   RenderGraphPass& Prepare(RenderGraphPrepareCallback p_prepare);
   // Execute records pass-local work into a SubCommandRecorder. The graph records barriers, rendering scopes,
   // queries, timestamps, resolves, and ExecuteSubCommandBuffers on the primary command stream.
   RenderGraphPass& Execute(RenderGraphExecuteCallback p_execute);
   // Clear metadata is consumed when the graph infers the BeginRendering attachments for this pass.
   RenderGraphPass& ClearAttachment(RenderGraphResourceHandle p_handle, ClearColorValue p_clearValue);
   // Prevents a zero-output or externally visible pass from being culled by future pruning logic.
   RenderGraphPass& NeverCull();
   // Query commands are recorded by the graph into the primary frame stream around pass-local subcommands.
   RenderGraphQuery WriteQuery(QueryDescriptor p_desc);
   RenderGraphQuery WriteQuery(Ptr<QueryPool> p_queryPool,
                               QueryControlFlags p_controlFlags = QueryControlFlags::None);
   RenderGraphQuery WriteQuery(Ptr<GHI::Query> p_query);
   RenderGraphQuery WriteQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex,
                               QueryControlFlags p_controlFlags = QueryControlFlags::None);
   RenderGraphTimestampQuery WriteTimestamps(Ptr<GHI::Query> p_beginQuery, Ptr<GHI::Query> p_endQuery,
                                             PipelineStageFlags p_beginStage = PipelineStageFlags::TopOfPipe,
                                             PipelineStageFlags p_endStage = PipelineStageFlags::BottomOfPipe);
   RenderGraphTimestampQuery WriteTimestamps(Ptr<QueryPool> p_queryPool, uint32_t p_beginQueryIndex,
                                             uint32_t p_endQueryIndex,
                                             PipelineStageFlags p_beginStage = PipelineStageFlags::TopOfPipe,
                                             PipelineStageFlags p_endStage = PipelineStageFlags::BottomOfPipe);

   // Descriptor overloads create graph-owned resources. Import* is for resources that already exist before the graph.
   RenderGraphOutputList<1> WriteImage(std::string_view p_name, ImageDescriptor p_desc,
                                       ResourceUsage p_usage = ResourceUsage::ColorAttachmentWrite,
                                       ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> WriteBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                        ResourceUsage p_usage = ResourceUsage::StorageWrite,
                                        ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> ReadWriteImage(std::string_view p_name, ImageDescriptor p_desc,
                                           ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                           ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<1> ReadWriteBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                            ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                            ShaderStageFlag p_shaderStages = ShaderStageFlag::All);

   std::string_view GetName() const;
   QueueFamilyType GetQueue() const;

 private:
   friend class RenderGraph;
   friend class RenderGraphContext;
   friend class RenderGraphPrepareContext;
   template <size_t, typename...>
   friend class RenderGraphOutputList;

   struct ResourceAccess
   {
      // One declared use of a logical resource by this pass. Dependency solving and barrier emission both
      // derive from this compact record.
      RenderGraphResourceHandle m_handle;
      ResourceUsage m_usage = ResourceUsage::Invalid;
      ShaderStageFlag m_shaderStages = ShaderStageFlag::All;
   };

   const std::vector<ResourceAccess>& GetResourceAccesses() const;
   const std::vector<ResourceAccess>& GetInputs() const;
   const std::vector<ResourceAccess>& GetOutputs() const;
   const std::vector<RenderGraphResourceHandle>& GetTransients() const;
   RenderGraphResourceHandle FindNamedInput(std::string_view p_name) const;
   RenderGraphResourceHandle FindNamedOutput(std::string_view p_name) const;
   bool HasDeclaredResource(RenderGraphResourceHandle p_handle) const;
   void AddNamedInput(std::string_view p_name, RenderGraphResourceHandle p_handle);
   void AddNamedOutput(std::string_view p_name, RenderGraphResourceHandle p_handle);

   struct NamedResource
   {
      std::string m_name;
      RenderGraphResourceHandle m_handle;
   };

   struct PassQueryInfo
   {
      // Queries live in the primary graph stream so begin/end can safely wrap pass-local subcommands.
      Ptr<GHI::Query> m_query;
      Ptr<QueryResultState> m_resultState;
   };

   struct PassTimestampInfo
   {
      // Timestamp writes also belong to the primary stream. The two result states share the same pass promise.
      Ptr<GHI::Query> m_beginQuery;
      Ptr<GHI::Query> m_endQuery;
      Ptr<QueryResultState> m_beginResultState;
      Ptr<QueryResultState> m_endResultState;
      uint32_t m_beginQueryIndex = 0u;
      uint32_t m_endQueryIndex = 0u;
      PipelineStageFlags m_beginStage = PipelineStageFlags::TopOfPipe;
      PipelineStageFlags m_endStage = PipelineStageFlags::BottomOfPipe;
   };

   struct AttachmentClearInfo
   {
      // Stored by logical handle but matched by storage resource at execution time, so clear metadata follows
      // Write/ReadWrite versions that share the same image.
      RenderGraphResourceHandle m_handle;
      ClearColorValue m_clearValue = {};
   };

 private:
   RenderGraph* m_graph = nullptr;
   uint32_t m_passIndex = 0u;
   QueueFamilyType m_queue = QueueFamilyType::GraphicsQueue;
   std::string m_name;
   std::vector<ResourceAccess> m_inputs;
   std::vector<ResourceAccess> m_outputs;
   std::vector<ResourceAccess> m_resourceAccesses;
   std::vector<RenderGraphResourceHandle> m_transients;
   std::vector<NamedResource> m_namedInputs;
   std::vector<NamedResource> m_namedOutputs;
   std::vector<AttachmentClearInfo> m_attachmentClears;
   std::optional<PassQueryInfo> m_query;
   std::optional<PassTimestampInfo> m_timestamps;
   RenderGraphPrepareCallback m_prepare;
   RenderGraphExecuteCallback m_execute;
   bool m_neverCull = false;
};

class RenderGraphTransientResourceWriter final
{
 public:
   RenderGraphTransientResourceWriter() = delete;

   // Backend transient materializers call these to bind concrete resources to the graph's scheduled slots.
   void SetImage(RenderGraphResourceHandle p_handle, Ptr<Image> p_image);
   void SetBuffer(RenderGraphResourceHandle p_handle, Ptr<Buffer> p_buffer);

 private:
   friend class RenderGraph;

   explicit RenderGraphTransientResourceWriter(RenderGraph& p_graph);

 private:
   RenderGraph* m_graph = nullptr;
};

class RenderGraph final
{
 public:
   RenderGraph() = default;

   // Clears passes, resources, solved order, and transient allocation analysis.
   void Reset();

   // Backend integration hooks. Configure these before Execute/Prepare when descriptor-backed resources are used.
   void SetImageMaterializer(RenderGraphImageMaterializer p_materializer);
   void SetBufferMaterializer(RenderGraphBufferMaterializer p_materializer);
   void SetImageViewMaterializer(RenderGraphImageViewMaterializer p_materializer);
   void SetBufferViewMaterializer(RenderGraphBufferViewMaterializer p_materializer);
   void SetBarrierEmitter(RenderGraphBarrierEmitter p_barrierEmitter);
   void SetQueueFamilyResolver(RenderGraphQueueFamilyResolver p_queueFamilyResolver);
   void SetTransientCompatibilityChecker(RenderGraphTransientCompatibilityChecker p_checker);
   void SetTransientAliasGroupCompatibilityChecker(RenderGraphTransientAliasGroupCompatibilityChecker p_checker);
   void SetTransientAllocationSizeResolver(RenderGraphTransientAllocationSizeResolver p_resolver);
   void SetTransientMaterializer(RenderGraphTransientMaterializer p_materializer);
   void SetSubCommandBufferCreator(RenderGraphSubCommandBufferCreator p_creator);
   void SetQueryReadbackBufferCreator(RenderGraphQueryReadbackBufferCreator p_creator);
   void SetParallelPassRecordingEnabled(bool p_enabled);
   bool IsParallelPassRecordingEnabled() const;

   // Imports an existing resource as valid before the graph starts. Imported resources are not transient.
   RenderGraphResourceHandle ImportImageView(std::string_view p_name, Ptr<ImageView> p_imageView,
                                             ResourceUsage p_initialUsage = ResourceUsage::Undefined,
                                             ShaderStageFlag p_initialShaderStages = ShaderStageFlag::All,
                                             QueueFamilyType p_initialQueue = QueueFamilyType::GraphicsQueue);
   RenderGraphResourceHandle ImportBufferView(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                              ResourceUsage p_initialUsage = ResourceUsage::Undefined,
                                              ShaderStageFlag p_initialShaderStages = ShaderStageFlag::All,
                                              QueueFamilyType p_initialQueue = QueueFamilyType::GraphicsQueue);

   RenderGraphPass& AddPass(std::string_view p_name);

   // Compile solves pass order and resource lifetimes. Prepare materializes resources and allows pass-local
   // transient declarations. Execute records barriers and pass callbacks into the provided command buffer.
   void Compile();
   void Prepare();
   void Execute(CommandBuffer& p_commandBuffer);

   // Resource queries accept logical handles and resolve to the backing storage where appropriate.
   Ptr<ImageView> GetImageView(RenderGraphResourceHandle p_handle) const;
   Ptr<BufferView> GetBufferView(RenderGraphResourceHandle p_handle) const;
   const ImageDescriptor* GetImageDescriptor(RenderGraphResourceHandle p_handle) const;
   const BufferDescriptor* GetBufferDescriptor(RenderGraphResourceHandle p_handle) const;
   bool WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const;
   bool CanResourceBeTransient(RenderGraphResourceHandle p_handle) const;
   bool CanResourceLifetimesAlias(RenderGraphResourceHandle p_first, RenderGraphResourceHandle p_second) const;
   // This combines graph lifetime rules with backend compatibility rules.
   bool CanResourcesShareTransientAllocation(RenderGraphResourceHandle p_first,
                                             RenderGraphResourceHandle p_second) const;
   bool CanResourceJoinTransientAliasGroup(const std::vector<RenderGraphResourceHandle>& p_groupResources,
                                           RenderGraphResourceHandle p_candidate) const;
   uint64_t GetTransientAllocationSize(RenderGraphResourceHandle p_handle) const;
   // Transient resources are allocator input; alias groups are the graph's selected lifetime/size schedule.
   const std::vector<RenderGraphTransientResourceInfo>& GetTransientResources() const;
   const std::vector<RenderGraphTransientAliasGroup>& GetTransientAliasGroups() const;
   uint32_t GetResourceFirstUseOrder(RenderGraphResourceHandle p_handle) const;
   uint32_t GetResourceLastUseOrder(RenderGraphResourceHandle p_handle) const;
   RenderGraphResourceType GetResourceType(RenderGraphResourceHandle p_handle) const;
   std::string_view GetResourceName(RenderGraphResourceHandle p_handle) const;

 private:
   friend class RenderGraphPrepareContext;
   friend class RenderGraphPass;
   friend class RenderGraphTransientResourceWriter;

   struct Resource
   {
      // Resource records serve two roles: storage resources own concrete views/descriptors, while logical
      // versions point at storage through m_storageResource and carry producer/ordering information.
      std::string m_name;
      RenderGraphResourceType m_type = RenderGraphResourceType::Invalid;
      Ptr<ImageView> m_imageView;
      Ptr<BufferView> m_bufferView;
      std::optional<ImageDescriptor> m_imageDesc;
      std::optional<BufferDescriptor> m_bufferDesc;
      bool m_imported = false;
      bool m_passLocal = false;
      bool m_createdInPrepare = false;
      bool m_canBeTransient = false;
      ResourceUsage m_initialUsage = ResourceUsage::Undefined;
      ShaderStageFlag m_initialShaderStages = ShaderStageFlag::All;
      QueueFamilyType m_initialQueue = QueueFamilyType::Invalid;
      uint32_t m_producerPass = RenderGraphResourceHandle::InvalidIndex;
      // Logical versions produced by Write/ReadWrite share one storage resource, but keep distinct DAG edges.
      uint32_t m_storageResource = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_previousVersion = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_ownerPass = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   };

   struct ResourceState
   {
      // Current semantic state for one storage resource while Execute walks the solved pass order.
      ResourceUsage m_usage = ResourceUsage::Undefined;
      ShaderStageFlag m_shaderStages = ShaderStageFlag::All;
      QueueFamilyType m_queue = QueueFamilyType::Invalid;
   };

   void AddDependency(std::vector<std::vector<uint32_t>>& p_dependencies, uint32_t p_from, uint32_t p_to) const;
   // Add*Resource creates storage resources. CreateResourceVersion creates logical DAG versions that reuse storage.
   RenderGraphResourceHandle AddImageResource(std::string_view p_name, Ptr<ImageView> p_imageView,
                                              std::optional<ImageDescriptor> p_desc, ResourceUsage p_initialUsage,
                                              ShaderStageFlag p_initialShaderStages, QueueFamilyType p_initialQueue,
                                              bool p_imported, bool p_passLocal, uint32_t p_ownerPass);
   RenderGraphResourceHandle AddBufferResource(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                               std::optional<BufferDescriptor> p_desc, ResourceUsage p_initialUsage,
                                               ShaderStageFlag p_initialShaderStages, QueueFamilyType p_initialQueue,
                                               bool p_imported, bool p_passLocal, uint32_t p_ownerPass);
   RenderGraphResourceHandle CreatePassTransientImage(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                      std::string_view p_name, ImageDescriptor p_desc,
                                                      ResourceUsage p_usage, ShaderStageFlag p_shaderStages);
   RenderGraphResourceHandle CreatePassTransientBuffer(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                       std::string_view p_name, BufferDescriptor p_desc,
                                                       ResourceUsage p_usage, ShaderStageFlag p_shaderStages);
   RenderGraphResourceHandle CreateResourceVersion(RenderGraphResourceHandle p_previousVersion, uint32_t p_producerPass);
   uint32_t GetStorageResourceIndex(RenderGraphResourceHandle p_handle) const;
   void SetResourceProducer(RenderGraphResourceHandle p_handle, uint32_t p_passIndex);
   // Materialization turns graph-owned descriptors into concrete views after lifetimes and aliasing are known.
   void MaterializeGraphResources();
   std::vector<RenderGraphTransientAliasGroupRequest> BuildTransientMaterializationRequests() const;
   void ValidateTransientResourcesMaterialized() const;
   void UpdateResourceLifetimes();
   // Transient analysis computes eligibility and then schedules alias groups for backend allocation.
   void AnalyzeTransientResources();
   void UpdateTransientAliasing();
   uint64_t EstimateTransientAllocationSize(RenderGraphResourceHandle p_handle) const;
   Ptr<SubCommandBuffer> CreateSubCommandBuffer(Ptr<Device> p_device) const;
   Ptr<Buffer> CreateQueryReadbackBuffer(Ptr<Device> p_device, uint64_t p_size) const;
   void EnsureQueryReadbackBuffer(const Ptr<QueryResultState>& p_queryResult) const;
   QueueFamilyInfo ResolveQueueFamilyInfo(QueueFamilyType p_queueType) const;
   void EmitBarrier(CommandBuffer& p_commandBuffer, const Resource& p_resource, ResourceState p_oldState,
                    ResourceState p_newState) const;
   void ValidateResourceAccess(const Resource& p_resource, const RenderGraphPass::ResourceAccess& p_access) const;

 private:
   std::vector<Resource> m_resources;
   std::deque<RenderGraphPass> m_passes;
   std::vector<uint32_t> m_executionOrder;
   std::vector<RenderGraphTransientResourceInfo> m_transientResources;
   std::vector<RenderGraphTransientAliasGroup> m_transientAliasGroups;
   RenderGraphImageMaterializer m_imageMaterializer;
   RenderGraphBufferMaterializer m_bufferMaterializer;
   RenderGraphImageViewMaterializer m_imageViewMaterializer;
   RenderGraphBufferViewMaterializer m_bufferViewMaterializer;
   RenderGraphBarrierEmitter m_barrierEmitter;
   RenderGraphQueueFamilyResolver m_queueFamilyResolver;
   RenderGraphTransientCompatibilityChecker m_transientCompatibilityChecker;
   RenderGraphTransientAliasGroupCompatibilityChecker m_transientAliasGroupCompatibilityChecker;
   RenderGraphTransientAllocationSizeResolver m_transientAllocationSizeResolver;
   RenderGraphTransientMaterializer m_transientMaterializer;
   RenderGraphSubCommandBufferCreator m_subCommandBufferCreator;
   RenderGraphQueryReadbackBufferCreator m_queryReadbackBufferCreator;
   bool m_parallelPassRecordingEnabled = false;
   bool m_compiled = false;
   bool m_prepared = false;
   bool m_isPreparing = false;
};

} // namespace GHI

} // namespace Render

namespace Render
{

namespace GHI
{

namespace Detail
{

// tuple_element needs to choose between a resource handle and one of the appended promise types without
// instantiating an out-of-range std::tuple_element for handle indices.
template <bool t_isHandle, size_t t_index, size_t t_count, typename... t_extras>
struct RenderGraphOutputListElement;

template <size_t t_index, size_t t_count, typename... t_extras>
struct RenderGraphOutputListElement<true, t_index, t_count, t_extras...>
{
   using type = RenderGraphResourceHandle;
};

template <size_t t_index, size_t t_count, typename... t_extras>
struct RenderGraphOutputListElement<false, t_index, t_count, t_extras...>
{
   using type = std::tuple_element_t<t_index - t_count, std::tuple<t_extras...>>;
};

} // namespace Detail

} // namespace GHI

} // namespace Render

namespace std
{

// Structured binding support for RenderGraphOutputList. This is intentionally specialized here so pass builders
// can return one object that decomposes into handles and query/timestamp promises.
template <size_t t_count, typename... t_extras>
struct tuple_size<Render::GHI::RenderGraphOutputList<t_count, t_extras...>>
    : integral_constant<size_t, t_count + sizeof...(t_extras)>
{
};

template <size_t t_index, size_t t_count, typename... t_extras>
struct tuple_element<t_index, Render::GHI::RenderGraphOutputList<t_count, t_extras...>>
{
   using type = typename Render::GHI::Detail::RenderGraphOutputListElement<(t_index < t_count), t_index, t_count,
                                                                           t_extras...>::type;
};

} // namespace std

namespace Render
{

namespace GHI
{

template <size_t t_index, size_t t_count, typename... t_extras>
auto get(const RenderGraphOutputList<t_count, t_extras...>& p_outputs)
{
   return p_outputs.template Get<t_index>();
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::Read(
    RenderGraphResourceHandle p_handle, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   m_pass->Read(p_handle, p_usage, p_shaderStages);
   return *this;
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::Read(
    std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
    ShaderStageFlag p_shaderStages)
{
   m_pass->Read(p_name, p_handle, p_usage, p_shaderStages);
   return *this;
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::Write(
    RenderGraphResourceHandle p_handle, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->Write(p_handle, p_usage, p_shaderStages);

   // Preserve every previously returned handle, append the new output handle, and keep promise extras unchanged.
   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::Write(
    std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
    ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->Write(p_name, p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::ReadWrite(
    RenderGraphResourceHandle p_handle, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWrite(p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::ReadWrite(
    std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
    ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWrite(p_name, p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::WriteImage(
    std::string_view p_name, ImageDescriptor p_desc, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->WriteImage(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::WriteBuffer(
    std::string_view p_name, BufferDescriptor p_desc, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->WriteBuffer(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::ReadWriteImage(
    std::string_view p_name, ImageDescriptor p_desc, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWriteImage(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count + 1, t_extras...> RenderGraphOutputList<t_count, t_extras...>::ReadWriteBuffer(
    std::string_view p_name, BufferDescriptor p_desc, ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWriteBuffer(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1, t_extras...>(*m_pass, handles, m_extras);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::Execute(
    RenderGraphExecuteCallback p_execute)
{
   m_pass->Execute(std::move(p_execute));
   return *this;
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::Prepare(
    RenderGraphPrepareCallback p_prepare)
{
   m_pass->Prepare(std::move(p_prepare));
   return *this;
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::NeverCull()
{
   m_pass->NeverCull();
   return *this;
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::ClearAttachment(
    size_t p_outputIndex, ClearColorValue p_clearValue)
{
   ASSERT(p_outputIndex < t_count, "RenderGraphOutputList::ClearAttachment output index is out of range");
   m_pass->ClearAttachment(m_handles[p_outputIndex], p_clearValue);
   return *this;
}

template <size_t t_count, typename... t_extras>
template <size_t t_index>
RenderGraphOutputList<t_count, t_extras...>& RenderGraphOutputList<t_count, t_extras...>::ClearAttachment(
    ClearColorValue p_clearValue)
{
   static_assert(t_index < t_count);
   return ClearAttachment(t_index, p_clearValue);
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras..., RenderGraphQuery> RenderGraphOutputList<t_count, t_extras...>::WriteQuery(
    Ptr<GHI::Query> p_query)
{
   if (p_query == nullptr)
   {
      // Optional backend features can pass null and still keep a stable structured-binding shape.
      return Append(RenderGraphQuery{});
   }
   return Append(m_pass->WriteQuery(std::move(p_query)));
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras..., RenderGraphQuery> RenderGraphOutputList<t_count, t_extras...>::WriteQuery(
    Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex, QueryControlFlags p_controlFlags)
{
   if (p_queryPool == nullptr)
   {
      // Optional backend features can pass null and still keep a stable structured-binding shape.
      return Append(RenderGraphQuery{});
   }
   return Append(m_pass->WriteQuery(std::move(p_queryPool), p_queryIndex, p_controlFlags));
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras..., RenderGraphTimestampQuery>
RenderGraphOutputList<t_count, t_extras...>::WriteTimestamps(Ptr<GHI::Query> p_beginQuery,
                                                             Ptr<GHI::Query> p_endQuery,
                                                             PipelineStageFlags p_beginStage,
                                                             PipelineStageFlags p_endStage)
{
   return Append(m_pass->WriteTimestamps(std::move(p_beginQuery), std::move(p_endQuery), p_beginStage, p_endStage));
}

template <size_t t_count, typename... t_extras>
RenderGraphOutputList<t_count, t_extras..., RenderGraphTimestampQuery>
RenderGraphOutputList<t_count, t_extras...>::WriteTimestamps(Ptr<QueryPool> p_queryPool,
                                                             uint32_t p_beginQueryIndex,
                                                             uint32_t p_endQueryIndex,
                                                             PipelineStageFlags p_beginStage,
                                                             PipelineStageFlags p_endStage)
{
   return Append(m_pass->WriteTimestamps(std::move(p_queryPool), p_beginQueryIndex, p_endQueryIndex, p_beginStage,
                                         p_endStage));
}

template <size_t t_count, typename... t_extras>
RenderGraphResourceHandle RenderGraphOutputList<t_count, t_extras...>::Input(std::string_view p_name) const
{
   return m_pass->FindNamedInput(p_name);
}

template <size_t t_count, typename... t_extras>
RenderGraphResourceHandle RenderGraphOutputList<t_count, t_extras...>::Output(std::string_view p_name) const
{
   return m_pass->FindNamedOutput(p_name);
}

template <size_t t_count, typename... t_extras>
template <typename t_extra>
RenderGraphOutputList<t_count, t_extras..., t_extra> RenderGraphOutputList<t_count, t_extras...>::Append(
    t_extra p_extra) const
{
   // Appending an extra does not touch the resource-handle portion of the tuple-like result.
   return RenderGraphOutputList<t_count, t_extras..., t_extra>(
       *m_pass, m_handles, std::tuple_cat(m_extras, std::tuple<t_extra>{std::move(p_extra)}));
}

} // namespace GHI

} // namespace Render
