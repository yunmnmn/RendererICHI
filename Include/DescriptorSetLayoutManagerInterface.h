#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <EASTL/shared_ptr.h>
#include <std/vector.h>

#include <glad/vulkan.h>

namespace Render
{
struct DescriptorSetlayoutDescriptor
{
   friend DescriptorSetLayout;

   DescriptorSetlayoutDescriptor() = delete;
   DescriptorSetlayoutDescriptor(Render::vector<VkDescriptorSetLayoutBinding>&& p_layoutBindings)
   {
      m_layoutBindings = p_layoutBindings;

      // Sort the DescriptorSetLayoutBindings in order of it
      const auto predicate = [](VkDescriptorSetLayoutBinding* p_a, VkDescriptorSetLayoutBinding* p_b) {
         return static_cast<uint32_t>(p_a->descriptorType) < static_cast<uint32_t>(p_b->descriptorType);
      };
      eastl::sort(m_layoutBindings.begin(), m_layoutBindings.end(), predicate);

      // Hash the array
      constexpr uint32_t seed = 1991u;
      MurmurHash3_x64_64(m_layoutBindings.data(), m_layoutBindings.size() * sizeof(VkDescriptorSetLayoutBinding), seed,
                         (void*)&m_descriptorSetHash);
   }

   uint64_t GetHash() const
   {
      return m_descriptorSetHash;
   }

 private:
   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   uint64_t m_descriptorSetHash = 0u;
};

class DescriptorSetLayoutManagerInterface : public Foundation::Util::ManagerInterface<DescriptorSetLayoutManagerInterface>
{
 public:
   virtual eastl::shared_ptr<class DescriptorSetLayout*> CreateOrGetDescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc) = 0;
};

}; // namespace Render
