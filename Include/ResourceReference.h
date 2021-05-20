#pragma once

#include <EASTL/weak_ptr.h>
#include <EASTL/span.h>

#include <std/shared_ptr.h>

#include <std/vector.h>

namespace Render
{
// Forward declare
template <typename>
class ResourceRef;

template <typename>
class ResourceUse;

// Wrapper around the shared_ptr type to only allow a single instance
template <typename t_Resource>
class ResourceUniqueRef
{
 public:
   // Gets the pointer
   t_Resource* operator->()
   {
      return m_resourceUnique.get();
   }

   t_Resource* Get()
   {
      return m_resourceUnique.get();
   }

   const t_Resource* Get() const
   {
      return m_resourceUnique.get();
   }

   // Returns the Resource Reference
   ResourceRef<t_Resource> GetResourceReference() const
   {
      return ResourceRef<t_Resource>(m_resourceUnique);
   }

   ResourceUniqueRef<t_Resource>& operator=(ResourceUniqueRef<t_Resource>&& p_other)
   {
      m_resourceUnique = eastl::move(p_other.m_resourceUnique);
      return *this;
   }

   ResourceUniqueRef(t_Resource* p_resource)
   {
      Render::RendererDeleter<t_Resource> deleter;
      Render::RendererEastlAllocator allocator;
      m_resourceUnique = Render::shared_ptr<t_Resource>(p_resource, deleter, allocator);
   }

   ResourceUniqueRef(ResourceUniqueRef&& p_other)
   {
      m_resourceUnique = eastl::move(p_other.m_resourceUnique);
   }

   ResourceUniqueRef()
   {
      m_resourceUnique = nullptr;
   }

 private:
   ResourceUniqueRef& operator=(const ResourceUniqueRef<t_Resource>& p_other) = delete;
   ResourceUniqueRef(const ResourceUniqueRef<t_Resource>& p_other) = delete;

   Render::shared_ptr<t_Resource> m_resourceUnique;
};

// Resource reference acquired from the RenderResource. Holds a weak_ptr to the shared_ptr. Can be used to be stored for
// longer use
class RenderResourceBase;
template <typename t_Resource>
class ResourceRef
{
   friend class ResourceUniqueRef<t_Resource>;

 public:
   ResourceRef() = default;

   template <typename U>
   ResourceRef(const ResourceRef<U>& p_resourceRef,
               typename eastl::enable_if<eastl::is_convertible<U*, t_Resource*>::value>::type* = 0)
   {
      m_resourceWeakRef = eastl::weak_ptr<U>(p_resourceRef.GetWeakReference());
   }

   ~ResourceRef()
   {
      m_resourceWeakRef.reset();
   }

   ResourceUse<t_Resource> Lock() const
   {
      return ResourceUse<t_Resource>(m_resourceWeakRef);
   }

   // Returns true if the weak reference is not nullptr, and has a use count higher than 0
   bool Alive() const
   {
      return !m_resourceWeakRef.expired();
   }

   bool AliveRecursive() const
   {
      if (!m_resourceWeakRef.expired())
      {
         ResourceUse<t_Resource> resourceUse(m_resourceWeakRef);
         eastl::span<const ResourceRef<RenderResourceBase>> dependencies = resourceUse->GetDependencies();
         for (const ResourceRef<RenderResourceBase>& dependency : dependencies)
         {
            if (!dependency.AliveRecursive())
            {
               return false;
            }
         }

         return true;
      }

      return false;
   }

   eastl::weak_ptr<t_Resource> GetWeakReference() const
   {
      return m_resourceWeakRef;
   }

 private:
   ResourceRef(const Render::shared_ptr<t_Resource>& p_sharedRef)
   {
      m_resourceWeakRef = eastl::weak_ptr<t_Resource>(p_sharedRef);
   }

   // TODO: fix
   eastl::weak_ptr<t_Resource> m_resourceWeakRef;
};

// Instance of the reference
template <typename t_Resource>
class ResourceUse
{
   friend class ResourceRef<t_Resource>;

 public:
   t_Resource* operator->() const
   {
      return m_resourceRef.get();
   }

   t_Resource* Get() const
   {
      return m_resourceRef.get();
   }

   ~ResourceUse()
   {
      m_resourceRef = nullptr;
   }

 private:
   ResourceUse() = delete;
   ResourceUse& operator=(const ResourceUse& p_other) = delete;
   ResourceUse(const ResourceUse& p_other) = delete;
   ResourceUse& operator=(ResourceUse&& p_other) = delete;
   ResourceUse(ResourceUse&& p_other) = delete;

   ResourceUse(eastl::weak_ptr<t_Resource> p_sharedWeakRef)
   {
      m_resourceRef = p_sharedWeakRef.lock();
   }

   Render::shared_ptr<t_Resource> m_resourceRef = nullptr;
};

class RenderResourceBase
{
 public:
   void AddDependency(ResourceRef<RenderResourceBase> p_resource)
   {
      if (p_resource.Alive())
      {
         m_dependencies.push_back(p_resource);
      }
   }

   eastl::span<const ResourceRef<RenderResourceBase>> GetDependencies() const
   {
      return eastl::span<const ResourceRef<RenderResourceBase>>(m_dependencies);
   }

 protected:
   Render::vector<ResourceRef<RenderResourceBase>> m_dependencies;
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
   static ResourceUniqueRef<t_Resource> CreateInstance(t_Descriptor&& p_desc)
   {
      t_Resource* resourceNative = new t_Resource(eastl::move(p_desc));
      ResourceUniqueRef<t_Resource> resource(resourceNative);
      resource.Get()->m_resourceRef = resource.GetResourceReference();
      // resource->InitializeInternal();
      return eastl::move(resource);
   }

   ResourceRef<t_Resource> GetReference() const
   {
      return m_resourceRef;
   }

 protected:
   RenderResource()
   {
   }

 private:
   ResourceRef<t_Resource> m_resourceRef;

   // virtual void InitializeInternal() override
   //{
   //}

   // virtual void DeleteResource() override
   //{
   //   delete this;
   //}
};

} // namespace Render
