#include <algorithm>
#include <queuej>
#include <vector>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <TaskScheduler.h>

#include <Util/Util.h>
#include <Util/HashName.h>
#include <Logger.h>
#include <IO/FileIO.h>

#include <GHI/RenderResource.h>
#include <GHI/RenderWindow.h>
#include <GHI/Device.h>
#include <GHI/Buffer.h>
#include <GHI/Renderer.h>
#include <GHI/CommandBuffer.h>
#include <GHI/Fence.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ShaderModule.h>
#include <GHI/ShaderStage.h>
#include <GHI/DescriptorSet.h>
#include <GHI/ShaderResourceSet.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/RenderWindow.h>
#include <GHI/Surface.h>
#include <GHI/Swapchain.h>
#include <GHI/VertexInputState.h>
#include <GHi/RendererState.h>
#include <GHI/Fence.h>
#include <GHI/BufferView.h>
#include <GHI/CommandPool.h>
#include <GHI/AsyncUploadQueue.h>
#include <GHI/ResourceDeleter.h>
#include <GHI/CommandPoolManager.h>
#include <GHI/DescriptorPoolManager.h>

#include <catch2/catch_test_macros.hpp>

using namespace Foundation;

TEST_CASE("test", "[test]")
{
   REQUIRE(true);
}
