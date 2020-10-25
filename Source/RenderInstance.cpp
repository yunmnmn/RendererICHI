#include <RenderInstance.h>

#include <GLFW/glfw3.h>

namespace Render
{
GLADapiproc tmp(const char* extension)
{
   return glfwGetInstanceProcAddress(NULL, extension);
}

eastl::unique_ptr<RenderInstance> RenderInstance::CreateInstance(Descriptor&& p_desc)
{
   return eastl::unique_ptr<RenderInstance>(new RenderInstance(p_desc));
}

RenderInstance::RenderInstance(Descriptor p_desc)
{
   m_applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   m_applicationInfo.pApplicationName = p_desc.m_instanceName.GetCStr();
   m_applicationInfo.pEngineName = p_desc.m_instanceName.GetCStr();
   m_applicationInfo.apiVersion = p_desc.m_version;

   // Check if glfw is loaded and supported
   ASSERT(glfwVulkanSupported(), "Vulkan isn't available");

   // load glad extensions here
   gladLoadVulkan(VkPhysicalDevice{}, tmp);

   // Get the available layers of this instance
   uint32_t instanceLayerCount = 0u;
   vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
   m_instanceLayerProperties.resize(instanceLayerCount);
   vkEnumerateInstanceLayerProperties(&instanceLayerCount, m_instanceLayerProperties.data());

   // Get all extensions
   uint32_t instanceExtensionCount = 0u;
   vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
   m_instanceExtensionProperties.resize(instanceExtensionCount);
   vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, m_instanceExtensionProperties.data());
}

RenderInstance::~RenderInstance()
{
}

void RenderInstance::AddLayer(Render::vector<Foundation::Util::HashName>&& layers)
{
   for (const auto& layer : layers)
   {
      for (const auto& layerProperty : m_instanceLayerProperties)
      {
         if (strcmp(layerProperty.layerName, layer.GetCStr()) == 0)
         {
            m_instanceLayers.push_back(layer);
            break;
         }
      }
   }
}

void RenderInstance::AddExtension(Render::vector<Foundation::Util::HashName>&& extensions)
{
   for (const auto& extension : extensions)
   {
      for (const auto& extensionProperty : m_instanceExtensionProperties)
      {
         if (strcmp(extensionProperty.extensionName, extension.GetCStr()) == 0)
         {
            m_instanceExtensions.push_back(extension);
            break;
         }
      }
   }
}

void RenderInstance::CompileInstance()
{
   Render::vector<const char*> instanceLayers;
   for (const auto& layer : m_instanceLayers)
   {
      instanceLayers.push_back(layer.GetCStr());
   }

   Render::vector<const char*> instanceExtensions;
   for (const auto& extension : m_instanceExtensions)
   {
      instanceExtensions.push_back(extension.GetCStr());
   }

   // Create the instance
   VkInstanceCreateInfo instanceCreateInfo = {};
   instanceCreateInfo.pApplicationInfo = &m_applicationInfo;
   instanceCreateInfo.enabledLayerCount = instanceLayers.size();
   instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
   instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
   instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
   vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
}

const VkInstance& RenderInstance::GetInstance() const
{
   return m_instance;
}

}; // namespace Render
