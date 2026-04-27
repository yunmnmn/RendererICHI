#pragma once

#include <memory>

#include <string_view>
#include <string>

#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

template <typename t_resource>
class RenderResource;

class Resource
{
 public:
   Resource()
   {
   }

   virtual ~Resource()
   {
   }

   void SetName(std::string_view p_name)
   {
      m_name = p_name;
   }

   std::string_view GetName() const
   {
      return m_name;
   }

 protected:
   virtual void ReleaseInternal() = 0;

 private:
   std::string m_name;
};

// ----------- DescriptorResource -----------

template <typename t_descriptor>
class DescriptorResource
{
 protected:
   DescriptorResource() = delete;
   DescriptorResource(t_descriptor&& p_desc)
   {
      m_desc = std::move(p_desc);
   }

 public:
   virtual ~DescriptorResource() = 0;

 public:

   const t_descriptor& GetDesc() const
   {
      return m_desc;
   }

 private:
   t_descriptor m_desc;
};

template <typename t_descriptor>
inline DescriptorResource<t_descriptor>::~DescriptorResource() {}

// ----------- RenderResource -----------

template <typename t_descriptor>
class RenderResource : public DescriptorResource<t_descriptor>, public Resource
{
 public:
   RenderResource& operator=(const RenderResource& p_other) = delete;
   RenderResource(const RenderResource& p_other) = delete;
   RenderResource& operator=(RenderResource&& p_other) = delete;
   RenderResource(RenderResource&& p_other) = delete;

 protected:
   RenderResource(t_descriptor&& p_desc) : DescriptorResource<t_descriptor>(std::move(p_desc)), Resource()
   {
   }

   virtual ~RenderResource() override = default;
};

}; // namespace GHI

}; // namespace Render
