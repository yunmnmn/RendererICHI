#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <std/string.h>

#include <Logger.h>

#include <VulkanInstance.h>
#include <VulkanDevice.h>
#include <RendererState.h>

using namespace Foundation;

namespace Render
{

// TODO: extend this: https://www.lunarg.com/wp-content/uploads/2018/05/Vulkan-Debug-Utils_05_18_v1.pdf
VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT p_messageSeverity,
                                                           VkDebugUtilsMessageTypeFlagsEXT p_messageType,
                                                           const VkDebugUtilsMessengerCallbackDataEXT* p_callbackData,
                                                           void* p_userData)
{
   UNUSED(p_userData);
   UNUSED(p_messageType);

   const Std::string debugMessage = Foundation::Util::SimpleSprintf<Std::string>(
       "[%d] [%s] %s", p_callbackData->messageIdNumber, p_callbackData->pMessageIdName, p_callbackData->pMessage);

   if (p_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
   {
      LOG_INFO(debugMessage.c_str());
   }
   else if (p_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
   {
      LOG_INFO(debugMessage.c_str());
   }
   else if (p_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
   {
      LOG_WARNING(debugMessage.c_str());
   }
   else if (p_messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
   {
      LOG_ERROR(debugMessage.c_str());

      // Abort
      return VK_TRUE;
   }

   return VK_FALSE;
}

VulkanInstance::VulkanInstance(VulkanInstanceDescriptor&& p_desc)
{
   m_applicationInfo = {};
   m_applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   m_applicationInfo.pNext = nullptr;
   m_applicationInfo.pApplicationName = p_desc.m_instanceName.GetCStr();
   m_applicationInfo.pEngineName = p_desc.m_instanceName.GetCStr();
   m_applicationInfo.apiVersion = p_desc.m_version;

   m_debugging = p_desc.m_debug;

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

   // Add the Instance Extensions
   {
      // If debug is enabled, add the Instance Extension
      if (m_debugging)
      {
         m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
      }

      // Add mandatory Instance Extensions
      uint32_t requiredExtensionCount = 0u;
      const char** requiredInstanceExtensions = glfwGetRequiredInstanceExtensions(&requiredExtensionCount);
      for (uint32_t i = 0u; i < requiredExtensionCount; i++)
      {
         m_instanceExtensions.push_back(requiredInstanceExtensions[i]);
      }

      // Add the extensions the user passed
      for (const auto& extension : p_desc.m_instanceExtensions)
      {
         bool supported = false;

         for (const auto& extensionProperty : m_instanceExtensionProperties)
         {
            // Check if the Instance Extension is supported
            if (strcmp(extensionProperty.extensionName, extension) == 0)
            {
               supported = true;
               break;
            }
         }

         if (supported)
         {
            // Check if the Instance Extensions already is added
            bool exist = false;
            for (const auto& instanceExtension : m_instanceExtensions)
            {
               if (strcmp(instanceExtension.GetCStr(), extension) == 0)
               {
                  exist = true;
                  break;
               }
            }

            // If it isn't in the list, add it
            if (!exist)
            {
               m_instanceExtensions.push_back(extension);
            }
         }
         else
         {
            LOG_WARNING_VAR("Vulkan instance doesn't support Extension \"%s\"", extension);
         }
      }
   }

   // Create the Vulkan instance
   Std::vector<const char*> instanceLayers;
   for (const auto& layer : m_instanceLayers)
   {
      instanceLayers.push_back(layer.GetCStr());
   }

   Std::vector<const char*> instanceExtensions;
   for (const auto& extension : m_instanceExtensions)
   {
      instanceExtensions.push_back(extension.GetCStr());
   }

   // Create the native Vulkan Instance Resource
   VkInstanceCreateInfo instanceCreateInfo = {};
   instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   instanceCreateInfo.pNext = nullptr;
   instanceCreateInfo.pApplicationInfo = &m_applicationInfo;
   instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(instanceLayers.size());
   instanceCreateInfo.ppEnabledLayerNames = instanceLayers.data();
   instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
   instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
   VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
   ASSERT(result == VK_SUCCESS, "Failed to create a Vulkan instance");

   // Register it to the VulkanInstanceInterface
   Render::VulkanInstanceInterface::Register(this);

   // Call it again with a valid Vulkan instance
   // gladLoadVulkan(VkPhysicalDevice{}, extensionLoader);

   // Set the debug properties
   if (m_debugging)
   {
      EnableDebugging();
   }

   // Create the RenderState instance
   {
      m_renderState = RenderState::CreateInstance(RenderStateDescriptor{});
      // Register the RenderState
      RenderStateInterface::Get()->Register(m_renderState.Get());
   }

   // Physical device
   uint32_t physicalDeviceCount = 0u;
   // Get number of available physical devices
   vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
   m_physicalDevices.resize(physicalDeviceCount);
   vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, m_physicalDevices.data());
}

VulkanInstance::~VulkanInstance()
{
   // Unregister the render instance
   Render::VulkanInstanceInterface::Unregister();
}

void VulkanInstance::EnableDebugging()
{
   VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{};
   debugUtilsMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
   debugUtilsMessengerCreateInfo.pNext = nullptr;
   debugUtilsMessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
   debugUtilsMessengerCreateInfo.messageType =
       VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
   debugUtilsMessengerCreateInfo.pfnUserCallback = debugUtilsMessengerCallback;

   PFN_vkCreateDebugUtilsMessengerEXT CreateDebugUtilsMessenger = VK_NULL_HANDLE;
   CreateDebugUtilsMessenger =
       (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");

   VkResult result = CreateDebugUtilsMessenger(m_instance, &debugUtilsMessengerCreateInfo, nullptr, &m_debugUtilsMessenger);
   ASSERT(result == VK_SUCCESS, "Failed to create the DebugUtilsMessenger");
}

const Std::vector<VkPhysicalDevice> VulkanInstance::GetPhysicalDevicesNative() const
{
   return m_physicalDevices;
}

const VkPhysicalDevice VulkanInstance::GetPhysicalDeviceNative(uint32_t p_physicalDeviceIndex) const
{
   return m_physicalDevices[p_physicalDeviceIndex];
}

uint32_t VulkanInstance::GetPhysicalDevicesCount() const
{
   return static_cast<uint32_t>(m_physicalDevices.size());
}

VkInstance VulkanInstance::GetInstanceNative() const
{
   return m_instance;
}

bool Render::VulkanInstance::IsLayerUsed(Foundation::Util::HashName layerName) const
{
   for (const auto& layer : m_instanceLayers)
   {
      if (layer.Hash() == layerName.Hash())
      {
         return true;
      }
   }
   return false;
}

bool Render::VulkanInstance::IsExtensionUsed(Foundation::Util::HashName extensionName) const
{
   for (const auto& extension : m_instanceExtensions)
   {
      if (extension.Hash() == extensionName.Hash())
      {
         return true;
      }
   }
   return false;
}

// ResourceRef<VulkanDevice> Render::VulkanInstance::GetSelectedVulkanDevice()
//{
//   ASSERT(m_physicalDevices.size() > 0, "There are no PhysicalDevice available");
//   ASSERT(m_physicalDeviceIndex != InvalidPhysicalDeviceIndex, "There is no PhysicalDevice selected");
//
//   return m_physicalDevices[m_physicalDeviceIndex];
//}

}; // namespace Render
