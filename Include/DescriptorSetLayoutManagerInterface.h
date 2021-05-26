#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Util/ManagerInterface.h>

#include <EASTL/weak_ptr.h>
#include <EASTL/sort.h>
#include <std/vector.h>

#include <ResourceReference.h>
#include <VulkanDevice.h>

namespace Render
{
class VulkanDevice;

struct DescriptorSetLayoutDescriptor
{
 public:
   DescriptorSetLayoutDescriptor() = delete;
   DescriptorSetLayoutDescriptor(Render::vector<VkDescriptorSetLayoutBinding>&& p_layoutBindings)
   {
      m_layoutBindings = p_layoutBindings;

      // Sort the DescriptorSetLayoutBindings in order of it
      const auto predicate = [](const VkDescriptorSetLayoutBinding& p_a, const VkDescriptorSetLayoutBinding& p_b) {
         return static_cast<uint32_t>(p_a.descriptorType) < static_cast<uint32_t>(p_b.descriptorType);
      };
      eastl::sort(m_layoutBindings.begin(), m_layoutBindings.end(), predicate);

      // Hash the array
      constexpr uint32_t seed = 1991u;
      MurmurHash3_x64_64(m_layoutBindings.data(), static_cast<int>(m_layoutBindings.size()) * sizeof(VkDescriptorSetLayoutBinding),
                         seed, (void*)&m_descriptorSetHash);
   }

   uint64_t GetHash() const
   {
      return m_descriptorSetHash;
   }

   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;

 private:
   uint64_t m_descriptorSetHash = 0u;
};

class DescriptorSetLayoutManagerInterface : public Foundation::Util::ManagerInterface<DescriptorSetLayoutManagerInterface>
{
 public:
   virtual ResourceRef<class DescriptorSetLayout> CreateOrGetDescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc) = 0;
};

}; // namespace Render
