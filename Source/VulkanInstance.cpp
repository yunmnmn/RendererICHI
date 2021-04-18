#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <VulkanInstance.h>

#include <Logger.h>

#include <std/string.h>

#include <VulkanDevice.h>

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

   const Render::string debugMessage = Foundation::Util::SimpleSprintf<Render::string>(
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

   // Check if glfw is loaded and supported
   ASSERT(glfwVulkanSupported(), "Vulkan isn't available");

   // Load glad extensions here
   const auto extensionLoader = [](const char* extension) -> GLADapiproc {
      VulkanInstanceInterface* vulkanInterface = VulkanInstanceInterface::Get();
      if (vulkanInterface)
      {
         return glfwGetInstanceProcAddress(vulkanInterface->GetInstanceNative(), extension);
      }
      else
      {
         return glfwGetInstanceProcAddress(VK_NULL_HANDLE, extension);
      }
   };

   // Load for the first time, with an invalid instance
   gladLoadVulkan(VkPhysicalDevice{}, extensionLoader);

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
   gladLoadVulkan(VkPhysicalDevice{}, extensionLoader);

   // Set the debug properties
   if (m_debugging)
   {
      EnableDebugging();
   }
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
   VkResult result = vkCreateDebugUtilsMessengerEXT(m_instance, &debugUtilsMessengerCreateInfo, nullptr, &m_debugUtilsMessenger);
   ASSERT(result == VK_SUCCESS, "Failed to create the DebugUtilsMessenger");
}

void VulkanInstance::CreatePhysicalDevices()
{
   ASSERT(m_instance != VK_NULL_HANDLE, "There is no valid Vulkan instance");

   // Physical device
   uint32_t gpuCount = 0u;
   // Get number of available physical devices
   vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr);
   std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
   vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());

   // Create physical device instances
   for (const auto& physicalDevice : physicalDevices)
   {
      m_physicalDevices.push_back(
          VulkanDevice::CreateInstance({.m_physicalDevice = physicalDevice, .m_vulkanInstance = GetReference()}));
   }
}

// Find a PhysicalDevice that supports these extensions, and create a logical device from
void VulkanInstance::SelectAndCreateLogicalDevice(ResourceRef<RenderWindow> p_window,
                                                  Render::vector<const char*>&& p_deviceExtensions)
{
   // Iterate through all the PhysicalDevices, and get the SwapChain properties
   for (auto& vulkanDevice : m_physicalDevices)
   {
      vulkanDevice->SetSwapchainDetails(p_window);
   }

   // Iterate through all the physical devices, and see if it supports the passed device extensions
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_physicalDevices.size()); i++)
   {
      bool isSupported = true;

      ResourceUniqueRef<VulkanDevice>& vulkanDevice = m_physicalDevices[i];

      // Check if all the extensions are supported
      for (const char* deviceExtension : p_deviceExtensions)
      {
         if (!vulkanDevice->IsDeviceExtensionSupported(deviceExtension))
         {
            isSupported = false;
            break;
         }
      }

      // Check if the there is a QueueFamily that supports Graphics, Compute and Transfer
      {
         const uint32_t queueFamilyIndex =
             vulkanDevice->SupportQueueFamilyFlags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT);
         if (queueFamilyIndex == static_cast<uint32_t>(-1))
         {
            isSupported = false;
         }
      }

      // Check if Presenting is supported
      {
         // Check if presenting is supported in the physical device
         if (!vulkanDevice->SupportPresenting())
         {
            isSupported = false;
         }
      }

      // Check if the swapchain is supported on the device
      {
         if (!vulkanDevice->SupportSwapchain())
         {
            isSupported = false;
         }
      }

      // TODO: only support discrete GPUs for now
      // Check if it's a discrete GPU
      {
         if (!vulkanDevice->IsDiscreteGpu())
         {
            isSupported = false;
         }
      }

      // If all device extensions, queues, presenting, swapchain, discrete GPU, pick that device
      if (isSupported)
      {
         m_physicalDeviceIndex = i;
         break;
      }
   }

   ASSERT(m_physicalDeviceIndex != InvalidPhysicalDeviceIndex,
          "There is no PhysicalDevice that is compatible with the required device extensions and/or supports Presenting");

   // Get a reference of the selected device
   ResourceUniqueRef<VulkanDevice>& selectedDevice = m_physicalDevices[m_physicalDeviceIndex];

   // If Debug is enabled, add the marker extension if a graphics debugger is attached to it
   if (m_debugging)
   {
      if (m_physicalDevices[m_physicalDeviceIndex]->IsDeviceExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
      {
         p_deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
      }
   }

   // Load the physical device specific function pointers
   const auto extensionLoader = [](const char* extension) -> GLADapiproc {
      VulkanInstanceInterface* vulkanInterface = VulkanInstanceInterface::Get();
      if (vulkanInterface)
      {
         return glfwGetInstanceProcAddress(vulkanInterface->GetInstanceNative(), extension);
      }
      else
      {
         return glfwGetInstanceProcAddress(VK_NULL_HANDLE, extension);
      }
   };
   gladLoadVulkan(selectedDevice->GetPhysicalDeviceNative(), extensionLoader);

   // Select the compatible physical device, and create a logical device
   selectedDevice->CreateLogicalDevice(eastl::move(p_deviceExtensions));
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

ResourceRef<VulkanDevice> Render::VulkanInstance::GetSelectedPhysicalDevice()
{
   ASSERT(m_physicalDevices.size() > 0, "There are no PhysicalDevice available");
   ASSERT(m_physicalDeviceIndex != InvalidPhysicalDeviceIndex, "There is no PhysicalDevice selected");

   return m_physicalDevices[m_physicalDeviceIndex].GetResourceReference();
}

}; // namespace Render
