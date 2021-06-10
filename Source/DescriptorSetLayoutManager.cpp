#include <DescriptorSetLayoutManager.h>

#include <std/vector.h>

#include <Util/MurmurHash3.h>

#include <DescriptorSetLayout.h>
#include <Renderer.h>

namespace Render
{

DescriptorSetLayoutManager::DescriptorSetLayoutManager(DescriptorSetLayoutManagerDescriptor&& p_desc)
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
}

DescriptorSetLayoutManager::~DescriptorSetLayoutManager()
{
}

ResourceRef<DescriptorSetLayout> DescriptorSetLayoutManager::CreateOrGetDescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc)
{
   std::lock_guard<std::mutex> lock(m_descriptorSetLayoutMapMutex);
   const uint64_t generatedHash = RendererHelper::CalculateHashFromDescriptorSetLayoutDescriptor(p_desc);
   // Check if a new DescriptorSetLayout needs to be created, or can be re-used
   auto descriptorSetLayoutItr = m_descriptorSetLayoutMap.find(generatedHash);
   // If there is none yet, create it
   if (descriptorSetLayoutItr == m_descriptorSetLayoutMap.end())
   {
      ResourceRef<DescriptorSetLayout>& descriptorSetLayout = m_descriptorSetLayoutMap[generatedHash];
      descriptorSetLayout = DescriptorSetLayout::CreateInstance(eastl::move(p_desc));
      return descriptorSetLayout;
   }

   // Return the existing pointer
   return descriptorSetLayoutItr->second;
}

}; // namespace Render
