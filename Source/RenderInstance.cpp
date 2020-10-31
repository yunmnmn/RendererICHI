#include <RenderInstance.h>

#include <Logger.h>

#include <std/string.h>

#include <GLFW/glfw3.h>

namespace Render
{
GLADapiproc tmp(const char* extension)
{
   return glfwGetInstanceProcAddress(NULL, extension);
}

// TODO: extend this: https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                           void* pUserData)
{
   const Render::string debugMessage = Foundation::Util::SimpleSprintf<Render::string>(
       "[%d] [%s] %s", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);

   if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
   {
      LOG_INFO(debugMessage.c_str());
   }
   else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
   {
      LOG_INFO(debugMessage.c_str());
   }
   else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
   {
      LOG_WARNING(debugMessage.c_str());
   }
   else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
   {
      LOG_ERROR(debugMessage.c_str());

      // Abort
      return VK_TRUE;
   }

   return VK_FALSE;
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

   // Add the layers that the user passed
   for (const auto& layer : p_desc.m_layers)
   {
      bool added = false;
      for (const auto& layerProperty : m_instanceLayerProperties)
      {
         if (strcmp(layerProperty.layerName, layer) == 0)
         {
            m_instanceLayers.push_back(layer);
            added = true;
            break;
         }
      }
      if (!added)
      {
         LOG_WARNING_VAR("Vulkan layer doesn't support Extension \"%s\"", layer);
      }
   }

   // Add the extensions the user passed
   for (const auto& extension : p_desc.m_extensions)
   {
      bool added = false;
      for (const auto& extensionProperty : m_instanceExtensionProperties)
      {
         if (strcmp(extensionProperty.extensionName, extension) == 0)
         {
            m_instanceExtensions.push_back(extension);
            break;
         }
      }

      if (added)
      {
         LOG_WARNING_VAR("Vulkan instance doesn't support Extension \"%s\"", extension);
      }
   }

   // Create the Vulkan instance
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
   instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
   instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
   instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
   instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
   VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
   ASSERT(result == VK_SUCCESS, "Failed to create a Vulkan instance");

   // TODO: make this optional
   // Enable debugging
   VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
   debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   debugUtilsMessengerCreateInfo.messageType =
       VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
   debugUtilsMessengerCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;
   result = vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsMessengerCreateInfo, nullptr, &m_debugUtilsMessenger);
   ASSERT(result == VK_SUCCESS, "Failed to create the DebugUtilsMessenger");
}

RenderInstance::~RenderInstance()
{
}

const VkInstance& RenderInstance::GetInstance() const
{
   return m_instance;
}

}; // namespace Render
