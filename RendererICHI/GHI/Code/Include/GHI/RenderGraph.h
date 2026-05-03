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
#include <GHI/Image.h>
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

struct RenderGraphResourceHandle
{
   static constexpr uint32_t InvalidIndex = static_cast<uint32_t>(-1);

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
};

class RenderGraphContext final
{
 public:
   RenderGraphContext() = delete;

   CommandBuffer& GetCommandBuffer() const;
   std::string_view GetPassName() const;
   size_t GetInputCount() const;
   size_t GetOutputCount() const;
   size_t GetTransientCount() const;
   RenderGraphResourceHandle GetInput(size_t p_index) const;
   RenderGraphResourceHandle GetOutput(size_t p_index) const;
   RenderGraphResourceHandle GetTransient(size_t p_index) const;
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

   std::string_view GetPassName() const;
   size_t GetInputCount() const;
   size_t GetOutputCount() const;
   size_t GetTransientCount() const;
   RenderGraphResourceHandle GetInput(size_t p_index) const;
   RenderGraphResourceHandle GetOutput(size_t p_index) const;
   RenderGraphResourceHandle GetTransient(size_t p_index) const;

   Ptr<ImageView> GetImageView(RenderGraphResourceHandle p_handle) const;
   Ptr<BufferView> GetBufferView(RenderGraphResourceHandle p_handle) const;
   const ImageDescriptor* GetImageDescriptor(RenderGraphResourceHandle p_handle) const;
   const BufferDescriptor* GetBufferDescriptor(RenderGraphResourceHandle p_handle) const;
   bool WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const;
   bool CanResourceBeTransient(RenderGraphResourceHandle p_handle) const;

   RenderGraphResourceHandle CreateTransientImage(std::string_view p_name, ImageDescriptor p_desc,
                                                  ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                                  ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphResourceHandle CreateTransientBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                   ResourceUsage p_usage = ResourceUsage::StorageReadWrite,
                                                   ShaderStageFlag p_shaderStages = ShaderStageFlag::All);

   void SetImageView(RenderGraphResourceHandle p_handle, Ptr<ImageView> p_imageView);
   void SetBufferView(RenderGraphResourceHandle p_handle, Ptr<BufferView> p_bufferView);

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
using RenderGraphImageMaterializer = std::function<Ptr<ImageView>(const ImageDescriptor&)>;
using RenderGraphBufferMaterializer = std::function<Ptr<BufferView>(const BufferDescriptor&)>;
using RenderGraphBarrierEmitter = std::function<void(CommandBuffer&, const RenderGraphBarrierInfo&)>;

template <size_t t_count>
class RenderGraphOutputList final
{
 public:
   RenderGraphOutputList(RenderGraphPass& p_pass, std::array<RenderGraphResourceHandle, t_count> p_handles)
       : m_pass(&p_pass), m_handles(p_handles)
   {
   }

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

   RenderGraphOutputList& Execute(RenderGraphExecuteCallback p_execute);
   RenderGraphOutputList& Prepare(RenderGraphPrepareCallback p_prepare);
   RenderGraphOutputList& SideEffect();

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

   RenderGraphPass& Read(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                         ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphPass& Write(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                          ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphPass& ReadWrite(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                              ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphPass& Use(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                        ShaderStageFlag p_shaderStages = ShaderStageFlag::All);
   RenderGraphPass& Prepare(RenderGraphPrepareCallback p_prepare);
   RenderGraphPass& Execute(RenderGraphExecuteCallback p_execute);
   RenderGraphPass& SideEffect();

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

 private:
   friend class RenderGraph;
   friend class RenderGraphContext;
   friend class RenderGraphPrepareContext;

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

 private:
   RenderGraph* m_graph = nullptr;
   uint32_t m_passIndex = 0u;
   std::string m_name;
   std::vector<ResourceAccess> m_inputs;
   std::vector<ResourceAccess> m_outputs;
   std::vector<ResourceAccess> m_resourceAccesses;
   std::vector<RenderGraphResourceHandle> m_transients;
   RenderGraphPrepareCallback m_prepare;
   RenderGraphExecuteCallback m_execute;
   bool m_hasSideEffect = false;
};

class RenderGraph final
{
 public:
   RenderGraph() = default;

   void Reset();

   void SetImageMaterializer(RenderGraphImageMaterializer p_materializer);
   void SetBufferMaterializer(RenderGraphBufferMaterializer p_materializer);
   void SetBarrierEmitter(RenderGraphBarrierEmitter p_barrierEmitter);

   RenderGraphResourceHandle ImportImageView(std::string_view p_name, Ptr<ImageView> p_imageView,
                                             ResourceUsage p_initialUsage = ResourceUsage::Undefined,
                                             ShaderStageFlag p_initialShaderStages = ShaderStageFlag::All);
   RenderGraphResourceHandle ImportBufferView(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                              ResourceUsage p_initialUsage = ResourceUsage::Undefined,
                                              ShaderStageFlag p_initialShaderStages = ShaderStageFlag::All);

   RenderGraphPass& AddPass(std::string_view p_name);

   void Compile();
   void Prepare();
   void Execute(CommandBuffer& p_commandBuffer);

   Ptr<ImageView> GetImageView(RenderGraphResourceHandle p_handle) const;
   Ptr<BufferView> GetBufferView(RenderGraphResourceHandle p_handle) const;
   const ImageDescriptor* GetImageDescriptor(RenderGraphResourceHandle p_handle) const;
   const BufferDescriptor* GetBufferDescriptor(RenderGraphResourceHandle p_handle) const;
   bool WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const;
   bool CanResourceBeTransient(RenderGraphResourceHandle p_handle) const;
   uint32_t GetResourceFirstUseOrder(RenderGraphResourceHandle p_handle) const;
   uint32_t GetResourceLastUseOrder(RenderGraphResourceHandle p_handle) const;
   RenderGraphResourceType GetResourceType(RenderGraphResourceHandle p_handle) const;
   std::string_view GetResourceName(RenderGraphResourceHandle p_handle) const;

 private:
   friend class RenderGraphPrepareContext;
   friend class RenderGraphPass;

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
      uint32_t m_producerPass = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_ownerPass = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_firstUseOrder = RenderGraphResourceHandle::InvalidIndex;
      uint32_t m_lastUseOrder = RenderGraphResourceHandle::InvalidIndex;
   };

   struct ResourceState
   {
      ResourceUsage m_usage = ResourceUsage::Undefined;
      ShaderStageFlag m_shaderStages = ShaderStageFlag::All;
   };

   void AddDependency(std::vector<std::vector<uint32_t>>& p_dependencies, uint32_t p_from, uint32_t p_to) const;
   RenderGraphResourceHandle AddImageResource(std::string_view p_name, Ptr<ImageView> p_imageView,
                                              std::optional<ImageDescriptor> p_desc, ResourceUsage p_initialUsage,
                                              ShaderStageFlag p_initialShaderStages, bool p_imported,
                                              bool p_passLocal, uint32_t p_ownerPass);
   RenderGraphResourceHandle AddBufferResource(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                               std::optional<BufferDescriptor> p_desc, ResourceUsage p_initialUsage,
                                               ShaderStageFlag p_initialShaderStages, bool p_imported,
                                               bool p_passLocal, uint32_t p_ownerPass);
   RenderGraphResourceHandle CreatePassTransientImage(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                      std::string_view p_name, ImageDescriptor p_desc,
                                                      ResourceUsage p_usage, ShaderStageFlag p_shaderStages);
   RenderGraphResourceHandle CreatePassTransientBuffer(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                       std::string_view p_name, BufferDescriptor p_desc,
                                                       ResourceUsage p_usage, ShaderStageFlag p_shaderStages);
   void SetResourceProducer(RenderGraphResourceHandle p_handle, uint32_t p_passIndex);
   void SetImageView(RenderGraphResourceHandle p_handle, Ptr<ImageView> p_imageView);
   void SetBufferView(RenderGraphResourceHandle p_handle, Ptr<BufferView> p_bufferView);
   void MaterializeGraphResources();
   void UpdateResourceLifetimes();
   void AnalyzeTransientResources();
   void EmitBarrier(CommandBuffer& p_commandBuffer, const Resource& p_resource, ResourceState p_oldState,
                    ResourceState p_newState) const;
   void ValidateResourceAccess(const Resource& p_resource, const RenderGraphPass::ResourceAccess& p_access) const;

 private:
   std::vector<Resource> m_resources;
   std::deque<RenderGraphPass> m_passes;
   std::vector<uint32_t> m_executionOrder;
   RenderGraphImageMaterializer m_imageMaterializer;
   RenderGraphBufferMaterializer m_bufferMaterializer;
   RenderGraphBarrierEmitter m_barrierEmitter;
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
RenderGraphOutputList<t_count>& RenderGraphOutputList<t_count>::SideEffect()
{
   m_pass->SideEffect();
   return *this;
}

} // namespace GHI

} // namespace Render
