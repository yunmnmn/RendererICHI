#pragma once

#include <array>
#include <deque>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

class BufferView;
class CommandBuffer;
class ImageView;
class RenderGraph;
class RenderGraphPrepareContext;
class RenderGraphPass;
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

   // Execute callbacks record commands against this command buffer. Resource access order is already solved.
   CommandBuffer& GetCommandBuffer() const;
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

   RenderGraphContext(RenderGraph& p_graph, RenderGraphPass& p_pass, CommandBuffer& p_commandBuffer);

 private:
   RenderGraph* m_graph = nullptr;
   RenderGraphPass* m_pass = nullptr;
   CommandBuffer* m_commandBuffer = nullptr;
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

template <size_t t_count>
class RenderGraphOutputList final
{
 public:
   RenderGraphOutputList(RenderGraphPass& p_pass, std::array<RenderGraphResourceHandle, t_count> p_handles)
       : m_pass(&p_pass), m_handles(p_handles)
   {
   }

   // Fluent continuation after a write-producing call. These calls still mutate the same pass.
   RenderGraphOutputList& Read(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                               ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList& Read(std::string_view p_name, RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                               ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> Write(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                            ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> Write(std::string_view p_name, RenderGraphResourceHandle p_handle,
                                            ResourceUsage p_usage,
                                            ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> ReadWrite(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                                ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> ReadWrite(std::string_view p_name, RenderGraphResourceHandle p_handle,
                                                ResourceUsage p_usage,
                                                ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> WriteImage(std::string_view p_name, ImageDescriptor p_desc,
                                                 ResourceUsage p_usage = ResourceUsage::ColorAttachmentWrite,
                                                 ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> WriteBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                  ResourceUsage p_usage = ResourceUsage::StorageWrite,
                                                  ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> ReadWriteImage(std::string_view p_name, ImageDescriptor p_desc,
                                                     ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                                     ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphOutputList<t_count + 1> ReadWriteBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                      ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                                      ShaderStageFlag p_shaderStages = ShaderStageFlag::All);

   // Terminal-style callbacks return the output list so structured bindings still work.
   RenderGraphOutputList& Execute(RenderGraphExecuteCallback p_execute);
   RenderGraphOutputList& Prepare(RenderGraphPrepareCallback p_prepare);
   RenderGraphOutputList& NeverCull();
   RenderGraphResourceHandle Input(std::string_view p_name) const;
   RenderGraphResourceHandle Output(std::string_view p_name) const;

   template <size_t t_index>
   RenderGraphResourceHandle Get() const
   {
      static_assert(t_index < t_count);
      return m_handles[t_index];
   }

 private:
   RenderGraphPass* m_pass = nullptr;
   std::array<RenderGraphResourceHandle, t_count> m_handles;
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
   RenderGraphPass& Execute(RenderGraphExecuteCallback p_execute);
   // Prevents a zero-output or externally visible pass from being culled by future pruning logic.
   RenderGraphPass& NeverCull();

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
   template <size_t>
   friend class RenderGraphOutputList;

   struct ResourceAccess
   {
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
   RenderGraphPrepareCallback m_prepare;
   RenderGraphExecuteCallback m_execute;
   bool m_neverCull = false;
};

class RenderGraphTransientResourceWriter final
{
 public:
   RenderGraphTransientResourceWriter() = delete;

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
      ResourceUsage m_usage = ResourceUsage::Undefined;
      ShaderStageFlag m_shaderStages = ShaderStageFlag::All;
      QueueFamilyType m_queue = QueueFamilyType::Invalid;
   };

   void AddDependency(std::vector<std::vector<uint32_t>>& p_dependencies, uint32_t p_from, uint32_t p_to) const;
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
   void MaterializeGraphResources();
   std::vector<RenderGraphTransientAliasGroupRequest> BuildTransientMaterializationRequests() const;
   void ValidateTransientResourcesMaterialized() const;
   void UpdateResourceLifetimes();
   void AnalyzeTransientResources();
   void UpdateTransientAliasing();
   uint64_t EstimateTransientAllocationSize(RenderGraphResourceHandle p_handle) const;
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
   bool m_compiled = false;
   bool m_prepared = false;
   bool m_isPreparing = false;
};

} // namespace GHI

} // namespace Render

namespace std
{

template <size_t t_count>
struct tuple_size<Render::GHI::RenderGraphOutputList<t_count>> : integral_constant<size_t, t_count>
{
};

template <size_t t_index, size_t t_count>
struct tuple_element<t_index, Render::GHI::RenderGraphOutputList<t_count>>
{
   using type = Render::GHI::RenderGraphResourceHandle;
};

} // namespace std

namespace Render
{

namespace GHI
{

template <size_t t_index, size_t t_count>
RenderGraphResourceHandle get(const RenderGraphOutputList<t_count>& p_outputs)
{
   return p_outputs.template Get<t_index>();
}

template <size_t t_count>
RenderGraphOutputList<t_count>& RenderGraphOutputList<t_count>::Read(RenderGraphResourceHandle p_handle,
                                                                      ResourceUsage p_usage,
                                                                      ShaderStageFlag p_shaderStages)
{
   m_pass->Read(p_handle, p_usage, p_shaderStages);
   return *this;
}

template <size_t t_count>
RenderGraphOutputList<t_count>& RenderGraphOutputList<t_count>::Read(std::string_view p_name,
                                                                      RenderGraphResourceHandle p_handle,
                                                                      ResourceUsage p_usage,
                                                                      ShaderStageFlag p_shaderStages)
{
   m_pass->Read(p_name, p_handle, p_usage, p_shaderStages);
   return *this;
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::Write(RenderGraphResourceHandle p_handle,
                                                                          ResourceUsage p_usage,
                                                                          ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->Write(p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::Write(std::string_view p_name,
                                                                          RenderGraphResourceHandle p_handle,
                                                                          ResourceUsage p_usage,
                                                                          ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->Write(p_name, p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::ReadWrite(RenderGraphResourceHandle p_handle,
                                                                              ResourceUsage p_usage,
                                                                              ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWrite(p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::ReadWrite(std::string_view p_name,
                                                                              RenderGraphResourceHandle p_handle,
                                                                              ResourceUsage p_usage,
                                                                              ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWrite(p_name, p_handle, p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::WriteImage(std::string_view p_name,
                                                                               ImageDescriptor p_desc,
                                                                               ResourceUsage p_usage,
                                                                               ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->WriteImage(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::WriteBuffer(std::string_view p_name,
                                                                                BufferDescriptor p_desc,
                                                                                ResourceUsage p_usage,
                                                                                ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->WriteBuffer(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::ReadWriteImage(std::string_view p_name,
                                                                                   ImageDescriptor p_desc,
                                                                                   ResourceUsage p_usage,
                                                                                   ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWriteImage(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count + 1> RenderGraphOutputList<t_count>::ReadWriteBuffer(std::string_view p_name,
                                                                                    BufferDescriptor p_desc,
                                                                                    ResourceUsage p_usage,
                                                                                    ShaderStageFlag p_shaderStages)
{
   const RenderGraphOutputList<1> next = m_pass->ReadWriteBuffer(p_name, std::move(p_desc), p_usage, p_shaderStages);

   std::array<RenderGraphResourceHandle, t_count + 1> handles = {};
   for (size_t i = 0u; i < t_count; ++i)
   {
      handles[i] = m_handles[i];
   }
   handles[t_count] = next.template Get<0>();
   return RenderGraphOutputList<t_count + 1>(*m_pass, handles);
}

template <size_t t_count>
RenderGraphOutputList<t_count>& RenderGraphOutputList<t_count>::Execute(RenderGraphExecuteCallback p_execute)
{
   m_pass->Execute(std::move(p_execute));
   return *this;
}

template <size_t t_count>
RenderGraphOutputList<t_count>& RenderGraphOutputList<t_count>::Prepare(RenderGraphPrepareCallback p_prepare)
{
   m_pass->Prepare(std::move(p_prepare));
   return *this;
}

template <size_t t_count>
RenderGraphOutputList<t_count>& RenderGraphOutputList<t_count>::NeverCull()
{
   m_pass->NeverCull();
   return *this;
}

template <size_t t_count>
RenderGraphResourceHandle RenderGraphOutputList<t_count>::Input(std::string_view p_name) const
{
   return m_pass->FindNamedInput(p_name);
}

template <size_t t_count>
RenderGraphResourceHandle RenderGraphOutputList<t_count>::Output(std::string_view p_name) const
{
   return m_pass->FindNamedOutput(p_name);
}

} // namespace GHI

} // namespace Render
