//#pragma once
//
//#include <inttypes.h>
//#include <stdbool.h>
//
//#include <vulkan/vulkan.h>
//
//
//#include <GHI/DescriptorSetLayout.h>
//
//using namespace Foundation;
//
//namespace Render
//{
//
//namespace GHI
//{
//
//namespace Vulkan
//{
//
//
//
//class DescriptorSetLayout final : public GHI::DescriptorSetLayout
//{
// public:
//   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayout, 12u);
//
// private:
//   DescriptorSetLayout() = delete;
//   DescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc);
//
// public:
//   ~DescriptorSetLayout() final;
//
// public:
//   // Get the DescriptorSetLayout Vulkan resource
//   const VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;
//
// private:
//   void GenerateHash();
//
// private:
//   // NOTE: These are sorted by their binding index
//   Std::vector<LayoutBinding> m_layoutBindings;
//   uint64_t m_descriptorSetLayoutHash = 0u;
//
//   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
//};
//
//} // namespace Vulkan
//
//} // namespace GHI
//
//}; // namespace Render
