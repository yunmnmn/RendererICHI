#pragma once

#include <EASTL/intrusive_ptr.h>

#include <std/shared_ptr.h>

#include <Std/vector.h>
#include <atomic>

namespace Render
{

template <typename t_Resource>
class RenderResource;

template <typename t_Resource>
class ResourceRef
{
   friend RenderResource<t_Resource>;

 public:
   ResourceRef() = default;

   ResourceRef(const ResourceRef& p_resourceRef)
   {
      m_resource = p_resourceRef.m_resource;
   }

   template <typename U>
   ResourceRef(const ResourceRef<U>& p_resourceRef,
               typename eastl::enable_if<eastl::is_convertible<U*, t_Resource*>::value>::type* = 0)
   {
      m_resource = eastl::intrusive_ptr<U>(p_resourceRef.Get());
   }

   ResourceRef(ResourceRef&& p_other)
   {
      m_resource = eastl::move(p_other.m_resource);
   }

   ResourceRef& operator=(ResourceRef&& p_other)
   {
      m_resource = eastl::move(p_other.m_resource);
      return *this;
   }

   ResourceRef& operator=(const ResourceRef& p_other)
   {
      m_resource = p_other.m_resource;
      return *this;
   }

   bool operator==(const ResourceRef& p_other) const
   {
      return (Get() == p_other.Get());
   }

   ~ResourceRef()
   {
      m_resource.reset();
   }

   // Returns whether the resource was set or not (nullptr)
   bool IsInitialized() const
   {
      return m_resource.get() != nullptr;
   }

   t_Resource* operator->()
   {
      return m_resource.get();
   }

   const t_Resource* operator->() const
   {
      return m_resource.get();
   }

   t_Resource* Get()
   {
      return m_resource.get();
   }

   const t_Resource* Get() const
   {
      return m_resource.get();
   }

 private:
   ResourceRef(t_Resource* p_resource)
   {
      m_resource = eastl::intrusive_ptr<t_Resource>(p_resource);
   }

   eastl::intrusive_ptr<t_Resource> m_resource;
};

class RenderResourceBase
{
   template <typename T>
   friend void eastl::intrusive_ptr_add_ref(T* p);

   template <typename T>
   friend void eastl::intrusive_ptr_release(T* p);

 public:
   virtual ~RenderResourceBase()
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

// Base class of a Render Resource. Adds a method to create the instance of a resource, and will create a shared_ptr reference of
// the instance for other objects to reference.
template <typename t_Resource>
class RenderResource : public RenderResourceBase
{
 public:
   RenderResource& operator=(const RenderResource& p_other) = delete;
   RenderResource(const RenderResource& p_other) = delete;
   RenderResource& operator=(RenderResource&& p_other) = delete;
   RenderResource(RenderResource&& p_other) = delete;

   template <typename t_Descriptor>
   static ResourceRef<t_Resource> CreateInstance(t_Descriptor&& p_desc)
   {
      t_Resource* resourceNative = new t_Resource(eastl::move(p_desc));
      ResourceRef<t_Resource> resource(resourceNative);
      return eastl::move(resource);
   }

 protected:
   RenderResource()
   {
   }

 private:
};

} // namespace Render
