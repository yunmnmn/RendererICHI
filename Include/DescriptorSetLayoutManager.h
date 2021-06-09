#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <Memory/ClassAllocator.h>

#include <EASTL/shared_ptr.h>

#include <std/unordered_map.h>

#include <DescriptorSetLayoutManagerInterface.h>

#include <ResourceReference.h>

namespace Render
{
class DescriptorSetLayout;
class VulkanDevice;

struct DescriptorSetLayoutManagerDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDevice;
};

class DescriptorSetLayoutManager : public DescriptorSetLayoutManagerInterface, public RenderResource<DescriptorSetLayoutManager>
{
 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayoutManager, 1u, static_cast<uint32_t>(sizeof(DescriptorSetLayoutManager)));

   DescriptorSetLayoutManager() = delete;
   DescriptorSetLayoutManager(DescriptorSetLayoutManagerDescriptor&& p_desc);

   ~DescriptorSetLayoutManager();

   // DescriptorSetLayoutManagerInterface overrides...
   ResourceRef<DescriptorSetLayout> CreateOrGetDescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc) final;

 private:
   Render::unordered_map<uint64_t, ResourceRef<DescriptorSetLayout>> m_descriptorSetLayoutMap;
   std::mutex m_descriptorSetLayoutMapMutex;

   ResourceRef<VulkanDevice> m_vulkanDevice;
};

}; // namespace Render
