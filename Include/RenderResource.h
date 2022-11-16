#pragma once

#include <atomic>
//#include <type_traits>

#include <Std/intrusive_ptr.h>
#include <std/shared_ptr.h>
#include <Std/vector.h>

#include <ForwardDeclare.h>

namespace Render
{

template <typename t_Resource>
class RenderResource;

class ResourceBase;

// ----------- Ptr -----------

template <typename T>
using Ptr = Std::intrusive_ptr<T>;

template <typename T>
using ConstPtr = Std::intrusive_ptr<const T>;


// ----------- RenderResource -----------

// Base class of a Render Resource. Adds a method to create the instance of a resource, and will create a shared_ptr reference of
// the instance for other objects to reference.
template <typename t_resource>
class RenderResource
{
   template<typename T>
   friend void eastl::intrusive_ptr_add_ref(T* p);

   template <typename T>
   friend void eastl::intrusive_ptr_release(T* p);

 public:
   using ResourceType = t_resource;

   RenderResource& operator=(const RenderResource& p_other) = delete;
   RenderResource(const RenderResource& p_other) = delete;
   RenderResource& operator=(RenderResource&& p_other) = delete;
   RenderResource(RenderResource&& p_other) = delete;

   template <typename t_Descriptor>
   static Ptr<t_resource> CreateInstance(t_Descriptor&& p_desc)
   {
      t_resource* resourceNative = new t_resource(eastl::move(p_desc));
      Ptr<t_resource> resource(resourceNative);
      return eastl::move(resource);
   }

 public:
   RenderResource()
   {
   }

   virtual ~RenderResource()
   {
   }

 protected:
   virtual void ReleaseInternal()
   {
   }

 private:
   void AddRef()
   {
      m_refCount++;
   }

   void Release()
   {
      m_refCount--;

      if (m_refCount == 0u)
      {
         ReleaseInternal();
         delete this;
      }
   }

   std::atomic_uint m_refCount = 0u;
};

} // namespace Render
