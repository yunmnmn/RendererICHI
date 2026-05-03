#include <GHI/RenderGraph.h>

#include <algorithm>
#include <utility>

#include <Util/Assert.h>

#include <GHI/BufferView.h>
#include <GHI/CommandBuffer.h>
#include <GHI/ImageView.h>

namespace Render
{

namespace GHI
{

namespace
{

constexpr uint32_t InvalidPass = static_cast<uint32_t>(-1);

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

} // namespace

RenderGraphContext::RenderGraphContext(RenderGraph& p_graph, RenderGraphPass& p_pass, CommandBuffer& p_commandBuffer)
    : m_graph(&p_graph), m_pass(&p_pass), m_commandBuffer(&p_commandBuffer)
{
}

CommandBuffer& RenderGraphContext::GetCommandBuffer() const
{
   return *m_commandBuffer;
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

Ptr<ImageView> RenderGraphContext::GetImageView(RenderGraphResourceHandle p_handle) const
{
   return m_graph->GetImageView(p_handle);
}

Ptr<BufferView> RenderGraphContext::GetBufferView(RenderGraphResourceHandle p_handle) const
{
   return m_graph->GetBufferView(p_handle);
}

RenderGraphPass::RenderGraphPass(RenderGraph& p_graph, uint32_t p_passIndex, std::string_view p_name)
    : m_graph(&p_graph), m_passIndex(p_passIndex), m_name(p_name)
{
}

RenderGraphPass& RenderGraphPass::Read(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                       ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(IsReadOnlyUsage(p_usage), "RenderGraphPass::Read requires a read-only ResourceUsage");
   ASSERT(p_handle.IsValid(), "RenderGraph pass reads an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass reads with an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   const ResourceAccess access{.m_handle = p_handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_inputs.push_back(access);
   m_resourceAccesses.push_back(access);
   return *this;
}

RenderGraphPass& RenderGraphPass::Write(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                        ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(IsWriteOnlyUsage(p_usage), "RenderGraphPass::Write requires a write-only ResourceUsage");
   ASSERT(p_handle.IsValid(), "RenderGraph pass writes an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass writes with an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   m_graph->SetResourceProducer(p_handle, m_passIndex);

   const ResourceAccess access{.m_handle = p_handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_outputs.push_back(access);
   m_resourceAccesses.push_back(access);
   return *this;
}

RenderGraphPass& RenderGraphPass::ReadWrite(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                            ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(IsReadWriteUsage(p_usage), "RenderGraphPass::ReadWrite requires a read-write ResourceUsage");
   ASSERT(p_handle.IsValid(), "RenderGraph pass read-writes an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass read-writes with an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   m_graph->SetResourceProducer(p_handle, m_passIndex);

   const ResourceAccess access{.m_handle = p_handle, .m_usage = p_usage, .m_shaderStages = p_shaderStages};
   m_inputs.push_back(access);
   m_outputs.push_back(access);
   m_resourceAccesses.push_back(access);
   return *this;
}

RenderGraphPass& RenderGraphPass::Use(RenderGraphResourceHandle p_handle, ResourceUsage p_usage,
                                      ShaderStageFlag p_shaderStages)
{
   m_graph->m_compiled = false;
   m_graph->m_prepared = false;

   ASSERT(p_handle.IsValid(), "RenderGraph pass uses an invalid resource handle");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass uses an invalid resource usage");
   ASSERT(p_handle.m_index < m_graph->m_resources.size(), "RenderGraph pass references an unknown resource");

   if (ResourceUsageWrites(p_usage))
   {
      m_graph->SetResourceProducer(p_handle, m_passIndex);
      m_outputs.push_back(ResourceAccess{.m_handle = p_handle,
                                         .m_usage = p_usage,
                                         .m_shaderStages = p_shaderStages});
   }

   if (ResourceUsageReads(p_usage))
   {
      m_inputs.push_back(ResourceAccess{.m_handle = p_handle,
                                        .m_usage = p_usage,
                                        .m_shaderStages = p_shaderStages});
   }

   m_resourceAccesses.push_back(ResourceAccess{.m_handle = p_handle,
                                               .m_usage = p_usage,
                                               .m_shaderStages = p_shaderStages});
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

RenderGraphPass& RenderGraphPass::SideEffect()
{
   m_graph->m_prepared = false;
   m_hasSideEffect = true;
   return *this;
}

RenderGraphOutputList<1> RenderGraphPass::WriteImage(std::string_view p_name, ImageDescriptor p_desc,
                                                     ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   RenderGraphResourceHandle handle =
       m_graph->AddImageResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                 false, false, RenderGraphResourceHandle::InvalidIndex);
   Write(handle, p_usage, p_shaderStages);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{handle});
}

RenderGraphOutputList<1> RenderGraphPass::WriteBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                      ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   RenderGraphResourceHandle handle =
       m_graph->AddBufferResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                  false, false, RenderGraphResourceHandle::InvalidIndex);
   Write(handle, p_usage, p_shaderStages);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{handle});
}

RenderGraphOutputList<1> RenderGraphPass::ReadWriteImage(std::string_view p_name, ImageDescriptor p_desc,
                                                         ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   RenderGraphResourceHandle handle =
       m_graph->AddImageResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                 false, false, RenderGraphResourceHandle::InvalidIndex);
   ReadWrite(handle, p_usage, p_shaderStages);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{handle});
}

RenderGraphOutputList<1> RenderGraphPass::ReadWriteBuffer(std::string_view p_name, BufferDescriptor p_desc,
                                                          ResourceUsage p_usage, ShaderStageFlag p_shaderStages)
{
   RenderGraphResourceHandle handle =
       m_graph->AddBufferResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All,
                                  false, false, RenderGraphResourceHandle::InvalidIndex);
   ReadWrite(handle, p_usage, p_shaderStages);
   return RenderGraphOutputList<1>(*this, std::array<RenderGraphResourceHandle, 1u>{handle});
}

std::string_view RenderGraphPass::GetName() const
{
   return m_name;
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

RenderGraphPrepareContext::RenderGraphPrepareContext(RenderGraph& p_graph, RenderGraphPass& p_pass,
                                                     uint32_t p_passOrder)
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

Ptr<ImageView> RenderGraphPrepareContext::GetImageView(RenderGraphResourceHandle p_handle) const
{
   return m_graph->GetImageView(p_handle);
}

Ptr<BufferView> RenderGraphPrepareContext::GetBufferView(RenderGraphResourceHandle p_handle) const
{
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

RenderGraphResourceHandle RenderGraphPrepareContext::CreateTransientImage(std::string_view p_name,
                                                                          ImageDescriptor p_desc,
                                                                          ResourceUsage p_usage,
                                                                          ShaderStageFlag p_shaderStages)
{
   return m_graph->CreatePassTransientImage(*m_pass, m_passOrder, p_name, std::move(p_desc), p_usage,
                                            p_shaderStages);
}

RenderGraphResourceHandle RenderGraphPrepareContext::CreateTransientBuffer(std::string_view p_name,
                                                                           BufferDescriptor p_desc,
                                                                           ResourceUsage p_usage,
                                                                           ShaderStageFlag p_shaderStages)
{
   return m_graph->CreatePassTransientBuffer(*m_pass, m_passOrder, p_name, std::move(p_desc), p_usage,
                                             p_shaderStages);
}

void RenderGraphPrepareContext::SetImageView(RenderGraphResourceHandle p_handle, Ptr<ImageView> p_imageView)
{
   m_graph->SetImageView(p_handle, std::move(p_imageView));
}

void RenderGraphPrepareContext::SetBufferView(RenderGraphResourceHandle p_handle, Ptr<BufferView> p_bufferView)
{
   m_graph->SetBufferView(p_handle, std::move(p_bufferView));
}

void RenderGraph::Reset()
{
   m_resources.clear();
   m_passes.clear();
   m_executionOrder.clear();
   m_compiled = false;
   m_prepared = false;
   m_isPreparing = false;
}

void RenderGraph::SetImageMaterializer(RenderGraphImageMaterializer p_materializer)
{
   m_imageMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetBufferMaterializer(RenderGraphBufferMaterializer p_materializer)
{
   m_bufferMaterializer = std::move(p_materializer);
   m_prepared = false;
}

void RenderGraph::SetBarrierEmitter(RenderGraphBarrierEmitter p_barrierEmitter)
{
   m_barrierEmitter = std::move(p_barrierEmitter);
}

RenderGraphResourceHandle RenderGraph::ImportImageView(std::string_view p_name, Ptr<ImageView> p_imageView,
                                                       ResourceUsage p_initialUsage,
                                                       ShaderStageFlag p_initialShaderStages)
{
   ASSERT(p_imageView != nullptr, "Can't import a null ImageView into the RenderGraph");

   return AddImageResource(p_name, std::move(p_imageView), std::nullopt, p_initialUsage, p_initialShaderStages, true,
                           false, RenderGraphResourceHandle::InvalidIndex);
}

RenderGraphResourceHandle RenderGraph::ImportBufferView(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                                        ResourceUsage p_initialUsage,
                                                        ShaderStageFlag p_initialShaderStages)
{
   ASSERT(p_bufferView != nullptr, "Can't import a null BufferView into the RenderGraph");

   return AddBufferResource(p_name, std::move(p_bufferView), std::nullopt, p_initialUsage, p_initialShaderStages, true,
                            false, RenderGraphResourceHandle::InvalidIndex);
}

RenderGraphPass& RenderGraph::AddPass(std::string_view p_name)
{
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

   for (uint32_t passIndex = 0u; passIndex < passCount; ++passIndex)
   {
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
            ASSERT(producerPass < passCount, "RenderGraph resource producer pass is out of range");
            AddDependency(dependencies, producerPass, passIndex);
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

      emitted[nextPass] = true;
      m_executionOrder.push_back(nextPass);

      for (const uint32_t dependentPass : dependencies[nextPass])
      {
         ASSERT(indegree[dependentPass] > 0u, "RenderGraph dependency indegree underflow");
         --indegree[dependentPass];
      }
   }

   UpdateResourceLifetimes();

   m_compiled = true;
}

void RenderGraph::Prepare()
{
   if (!m_compiled)
   {
      Compile();
   }

   ASSERT(!m_isPreparing, "RenderGraph is already preparing resources");

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
      resourceStates.push_back(ResourceState{.m_usage = resource.m_initialUsage,
                                             .m_shaderStages = resource.m_initialShaderStages});
   }

   for (const uint32_t passIndex : m_executionOrder)
   {
      RenderGraphPass& pass = m_passes[passIndex];
      for (const RenderGraphPass::ResourceAccess& access : pass.GetResourceAccesses())
      {
         ResourceState& state = resourceStates[access.m_handle.m_index];
         const ResourceState nextState{.m_usage = access.m_usage, .m_shaderStages = access.m_shaderStages};

         if (state.m_usage != nextState.m_usage || state.m_shaderStages != nextState.m_shaderStages)
         {
            EmitBarrier(p_commandBuffer, m_resources[access.m_handle.m_index], state, nextState);
            state = nextState;
         }
      }

      if (pass.m_execute)
      {
         RenderGraphContext context(*this, pass, p_commandBuffer);
         pass.m_execute(context);
      }
   }
}

Ptr<ImageView> RenderGraph::GetImageView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph image handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Image,
          "RenderGraph handle does not reference an ImageView");
   return m_resources[p_handle.m_index].m_imageView;
}

Ptr<BufferView> RenderGraph::GetBufferView(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph buffer handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Buffer,
          "RenderGraph handle does not reference a BufferView");
   return m_resources[p_handle.m_index].m_bufferView;
}

const ImageDescriptor* RenderGraph::GetImageDescriptor(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph image descriptor handle is out of range");
   ASSERT(m_resources[p_handle.m_index].m_type == RenderGraphResourceType::Image,
          "RenderGraph handle does not reference an image resource");

   const std::optional<ImageDescriptor>& desc = m_resources[p_handle.m_index].m_imageDesc;
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

   const std::optional<BufferDescriptor>& desc = m_resources[p_handle.m_index].m_bufferDesc;
   if (!desc.has_value())
   {
      return nullptr;
   }
   return &desc.value();
}

bool RenderGraph::WasResourceCreatedInPrepare(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[p_handle.m_index].m_createdInPrepare;
}

bool RenderGraph::CanResourceBeTransient(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[p_handle.m_index].m_canBeTransient;
}

uint32_t RenderGraph::GetResourceFirstUseOrder(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[p_handle.m_index].m_firstUseOrder;
}

uint32_t RenderGraph::GetResourceLastUseOrder(RenderGraphResourceHandle p_handle) const
{
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph resource handle is out of range");
   return m_resources[p_handle.m_index].m_lastUseOrder;
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

   std::vector<uint32_t>& passDependencies = p_dependencies[p_from];
   if (std::find(passDependencies.begin(), passDependencies.end(), p_to) == passDependencies.end())
   {
      passDependencies.push_back(p_to);
   }
}

RenderGraphResourceHandle RenderGraph::AddImageResource(std::string_view p_name, Ptr<ImageView> p_imageView,
                                                        std::optional<ImageDescriptor> p_desc,
                                                        ResourceUsage p_initialUsage,
                                                        ShaderStageFlag p_initialShaderStages, bool p_imported,
                                                        bool p_passLocal, uint32_t p_ownerPass)
{
   ASSERT(p_imageView != nullptr || p_desc.has_value(), "RenderGraph image resources need a view or descriptor");

   const uint32_t index = static_cast<uint32_t>(m_resources.size());
   m_resources.push_back(Resource{.m_name = std::string(p_name),
                                  .m_type = RenderGraphResourceType::Image,
                                  .m_imageView = std::move(p_imageView),
                                  .m_imageDesc = std::move(p_desc),
                                  .m_imported = p_imported,
                                  .m_passLocal = p_passLocal,
                                  .m_initialUsage = p_initialUsage,
                                  .m_initialShaderStages = p_initialShaderStages,
                                  .m_ownerPass = p_ownerPass});
   if (!m_isPreparing)
   {
      m_compiled = false;
      m_prepared = false;
   }
   return RenderGraphResourceHandle{.m_index = index};
}

RenderGraphResourceHandle RenderGraph::AddBufferResource(std::string_view p_name, Ptr<BufferView> p_bufferView,
                                                         std::optional<BufferDescriptor> p_desc,
                                                         ResourceUsage p_initialUsage,
                                                         ShaderStageFlag p_initialShaderStages, bool p_imported,
                                                         bool p_passLocal, uint32_t p_ownerPass)
{
   ASSERT(p_bufferView != nullptr || p_desc.has_value(), "RenderGraph buffer resources need a view or descriptor");

   const uint32_t index = static_cast<uint32_t>(m_resources.size());
   m_resources.push_back(Resource{.m_name = std::string(p_name),
                                  .m_type = RenderGraphResourceType::Buffer,
                                  .m_bufferView = std::move(p_bufferView),
                                  .m_bufferDesc = std::move(p_desc),
                                  .m_imported = p_imported,
                                  .m_passLocal = p_passLocal,
                                  .m_initialUsage = p_initialUsage,
                                  .m_initialShaderStages = p_initialShaderStages,
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
                                                                ResourceUsage p_usage,
                                                                ShaderStageFlag p_shaderStages)
{
   ASSERT(m_isPreparing, "RenderGraph pass-local images can only be created during Prepare");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass-local image uses an invalid ResourceUsage");

   RenderGraphResourceHandle handle =
       AddImageResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All, false,
                        true, p_pass.m_passIndex);

   Resource& resource = m_resources[handle.m_index];
   resource.m_createdInPrepare = true;
   resource.m_firstUseOrder = p_passOrder;
   resource.m_lastUseOrder = p_passOrder;

   if (m_imageMaterializer)
   {
      resource.m_imageView = m_imageMaterializer(resource.m_imageDesc.value());
      ASSERT(resource.m_imageView != nullptr, "RenderGraph image materializer returned a null ImageView");
   }

   const RenderGraphPass::ResourceAccess access{.m_handle = handle,
                                                .m_usage = p_usage,
                                                .m_shaderStages = p_shaderStages};
   p_pass.m_transients.push_back(handle);
   p_pass.m_resourceAccesses.push_back(access);
   return handle;
}

RenderGraphResourceHandle RenderGraph::CreatePassTransientBuffer(RenderGraphPass& p_pass, uint32_t p_passOrder,
                                                                 std::string_view p_name, BufferDescriptor p_desc,
                                                                 ResourceUsage p_usage,
                                                                 ShaderStageFlag p_shaderStages)
{
   ASSERT(m_isPreparing, "RenderGraph pass-local buffers can only be created during Prepare");
   ASSERT(p_usage != ResourceUsage::Invalid, "RenderGraph pass-local buffer uses an invalid ResourceUsage");

   RenderGraphResourceHandle handle =
       AddBufferResource(p_name, nullptr, std::move(p_desc), ResourceUsage::Undefined, ShaderStageFlag::All, false,
                         true, p_pass.m_passIndex);

   Resource& resource = m_resources[handle.m_index];
   resource.m_createdInPrepare = true;
   resource.m_firstUseOrder = p_passOrder;
   resource.m_lastUseOrder = p_passOrder;

   if (m_bufferMaterializer)
   {
      resource.m_bufferView = m_bufferMaterializer(resource.m_bufferDesc.value());
      ASSERT(resource.m_bufferView != nullptr, "RenderGraph buffer materializer returned a null BufferView");
   }

   const RenderGraphPass::ResourceAccess access{.m_handle = handle,
                                                .m_usage = p_usage,
                                                .m_shaderStages = p_shaderStages};
   p_pass.m_transients.push_back(handle);
   p_pass.m_resourceAccesses.push_back(access);
   return handle;
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

void RenderGraph::SetImageView(RenderGraphResourceHandle p_handle, Ptr<ImageView> p_imageView)
{
   ASSERT(m_isPreparing, "RenderGraph ImageView materialization is only valid during Prepare");
   ASSERT(p_imageView != nullptr, "Can't materialize a RenderGraph image resource with a null ImageView");
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph image materialization handle is out of range");

   Resource& resource = m_resources[p_handle.m_index];
   ASSERT(resource.m_type == RenderGraphResourceType::Image,
          "RenderGraph handle does not reference an image resource");

   if (resource.m_imageView != nullptr)
   {
      ASSERT(resource.m_imageView == p_imageView, "RenderGraph image resource was already materialized");
      return;
   }

   resource.m_createdInPrepare = !resource.m_imported && resource.m_imageDesc.has_value();
   resource.m_canBeTransient = false;
   resource.m_imageView = std::move(p_imageView);
}

void RenderGraph::SetBufferView(RenderGraphResourceHandle p_handle, Ptr<BufferView> p_bufferView)
{
   ASSERT(m_isPreparing, "RenderGraph BufferView materialization is only valid during Prepare");
   ASSERT(p_bufferView != nullptr, "Can't materialize a RenderGraph buffer resource with a null BufferView");
   ASSERT(p_handle.m_index < m_resources.size(), "RenderGraph buffer materialization handle is out of range");

   Resource& resource = m_resources[p_handle.m_index];
   ASSERT(resource.m_type == RenderGraphResourceType::Buffer,
          "RenderGraph handle does not reference a buffer resource");

   if (resource.m_bufferView != nullptr)
   {
      ASSERT(resource.m_bufferView == p_bufferView, "RenderGraph buffer resource was already materialized");
      return;
   }

   resource.m_createdInPrepare = !resource.m_imported && resource.m_bufferDesc.has_value();
   resource.m_canBeTransient = false;
   resource.m_bufferView = std::move(p_bufferView);
}

void RenderGraph::MaterializeGraphResources()
{
   for (Resource& resource : m_resources)
   {
      if (resource.m_imported || resource.m_passLocal)
      {
         continue;
      }

      if (resource.m_type == RenderGraphResourceType::Image && resource.m_imageView == nullptr &&
          resource.m_imageDesc.has_value() && m_imageMaterializer)
      {
         resource.m_imageView = m_imageMaterializer(resource.m_imageDesc.value());
         ASSERT(resource.m_imageView != nullptr, "RenderGraph image materializer returned a null ImageView");
         resource.m_createdInPrepare = true;
         resource.m_canBeTransient = false;
         continue;
      }

      if (resource.m_type == RenderGraphResourceType::Buffer && resource.m_bufferView == nullptr &&
          resource.m_bufferDesc.has_value() && m_bufferMaterializer)
      {
         resource.m_bufferView = m_bufferMaterializer(resource.m_bufferDesc.value());
         ASSERT(resource.m_bufferView != nullptr, "RenderGraph buffer materializer returned a null BufferView");
         resource.m_createdInPrepare = true;
         resource.m_canBeTransient = false;
      }
   }
}

void RenderGraph::UpdateResourceLifetimes()
{
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
         Resource& resource = m_resources[access.m_handle.m_index];
         if (resource.m_firstUseOrder == RenderGraphResourceHandle::InvalidIndex)
         {
            resource.m_firstUseOrder = static_cast<uint32_t>(orderIndex);
         }
         resource.m_lastUseOrder = static_cast<uint32_t>(orderIndex);
      }
   }
}

void RenderGraph::AnalyzeTransientResources()
{
   for (Resource& resource : m_resources)
   {
      const bool hasDescriptor = resource.m_imageDesc.has_value() || resource.m_bufferDesc.has_value();
      const bool hasMaterializedView = resource.m_imageView != nullptr || resource.m_bufferView != nullptr;
      const bool hasGraphLifetime = resource.m_firstUseOrder != RenderGraphResourceHandle::InvalidIndex &&
                                    resource.m_lastUseOrder != RenderGraphResourceHandle::InvalidIndex;

      resource.m_canBeTransient = resource.m_createdInPrepare && !resource.m_imported && hasDescriptor &&
                                  hasMaterializedView && hasGraphLifetime;
   }
}

void RenderGraph::EmitBarrier(CommandBuffer& p_commandBuffer, const Resource& p_resource, ResourceState p_oldState,
                              ResourceState p_newState) const
{
   ASSERT(m_barrierEmitter, "RenderGraph needs a backend barrier emitter before it can execute transitions");

   m_barrierEmitter(p_commandBuffer, RenderGraphBarrierInfo{.m_resourceType = p_resource.m_type,
                                                            .m_imageView = p_resource.m_imageView,
                                                            .m_bufferView = p_resource.m_bufferView,
                                                            .m_oldUsage = p_oldState.m_usage,
                                                            .m_newUsage = p_newState.m_usage,
                                                            .m_oldShaderStages = p_oldState.m_shaderStages,
                                                            .m_newShaderStages = p_newState.m_shaderStages});
}

void RenderGraph::ValidateResourceAccess(const Resource& p_resource, const RenderGraphPass::ResourceAccess& p_access) const
{
   const ResourceUsageInfo usageInfo = ResourceUsageToInfo(p_access.m_usage, p_access.m_shaderStages);

   if (p_resource.m_type == RenderGraphResourceType::Image)
   {
      ASSERT(usageInfo.m_imageLayout != ImageLayout::Invalid, "ResourceUsage is not valid for an image resource");
   }
}

} // namespace GHI

} // namespace Render
