#pragma once

#include <atomic>
//#include <type_traits>

#include <Std/intrusive_ptr.h>
#include <std/shared_ptr.h>
#include <Std/vector.h>
#include <Std/string.h>
#include <Std/string_view.h>

#include <ResourceDeleterInterface.h>
#include <ResourceTrackerInterface.h>

namespace Render
{

template <typename T>
void intrusive_ptr_add_ref(const T* p)
{
   const_cast<T*>(p)->AddRef();
}

template <typename T>
void intrusive_ptr_release(const T* p)
{
   const_cast<T*>(p)->Release();
}

template <typename t_resource>
class RenderResource;

class ResourceBase;

// ----------- Ptr -----------

template <typename T>
using Ptr = Std::intrusive_ptr<T>;

template <typename T>
using ConstPtr = Std::intrusive_ptr<const T>;

// ----------- Resource -----------

class Resource
{
 public:
   Resource()
   {
      ResourceTrackerInterface::Get()->Track(this);
   }

   virtual ~Resource()
   {
      ResourceTrackerInterface::Get()->Untrack(this);
   }

   void SetName(Std::string_view p_name)
   {
      m_name = p_name;
   }

   Std::string_view GetName() const
   {
      return m_name;
   }

 private:
   Std::string m_name;
};

// ----------- RenderResource -----------

template <typename t_resource>
class RenderResource : public Resource
{
   template <typename T>
   friend void eastl::intrusive_ptr_add_ref(T* p);

   template <typename T>
   friend void eastl::intrusive_ptr_release(T* p);

   template <typename T>
   friend void Render::intrusive_ptr_add_ref(const T* p);

   template <typename T>
   friend void Render::intrusive_ptr_release(const T* p);

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
      return resource;
   }

 public:
   RenderResource() = default;
   virtual ~RenderResource() override = default;

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

         ResourceDeleterInterface::Get()->QueueForDeletion(this);
      }
   }

 private:
   std::atomic_uint m_refCount = 0u;
};

} // namespace Render
