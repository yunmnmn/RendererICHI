#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>

#include <std/vector.h>
#include <glad/vulkan.h>

namespace Render
{
struct DescriptorSetlayoutDescriptor;

class DescriptorSetLayout
{
   friend class DescriptorSetLayoutManager;

 public:
   static constexpr size_t MaxDescriptorSetLayoutCountPerPage = 1024u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayout, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorSetLayout) * MaxDescriptorSetLayoutCountPerPage));

   static eastl::unique_ptr<DescriptorSetLayout> CreateInstance(DescriptorSetlayoutDescriptor&& p_desc)
   {
      eastl::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout(eastl::move(p_desc)));
   }

   DescriptorSetLayout() = delete;
   DescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc);
   ~DescriptorSetLayout();

   // Get the DescriptorSetLayout Vulkan resource
   VkDescriptorSetLayout GetDescriptorSetLayout() const;

   // Get the descriptorSetLayoutBindings
   const Render::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetlayoutBindings() const;

   // Get the DescriptorSet's hash
   uint64_t GetDescriptorSetLayoutHash() const;

 private:
   // Get the descriptorSetLayout
   eastl::weak_ptr<DescriptorSetLayout*> GetDescriptorSetLayoutReference();

   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
   uint64_t m_descriptorSetLayoutHash = 0u;

   // Shared reference of "this" pointer that will be passed to instances that use this DescriptorSetLayout
   eastl::shared_ptr<DescriptorSetLayout*> m_descriptorSetLayoutReference = nullptr;
};
}; // namespace Render
