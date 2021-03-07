#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>

namespace Render
{
// Instance of the reference
template <typename t_Resource>
class ResourceUse
{
   friend class ResourceRef<t_Resource>;

 public:
   t_Resource* operator->() const
   {
      *m_descriptorPool.get();
   }

   t_Resource* Get() const
   {
      *m_descriptorPool.get();
   }

 private:
   ResourceUse() = delete;
   ResourceUse& operator=(const ResourceUse& p_other) = delete;
   ResourceUse(const ResourceUse& p_other) = delete;
   ResourceUse& operator=(ResourceUse&& p_other) = delete;
   ResourceUse(ResourceUse&& p_other) = delete;

   ResourceUse(eastl::weak_ptr<t_Resource*> p_sharedWeakRef)
   {
      m_resourceRef = p_sharedWeakRef.lock();
   }

   ~ResourceUse()
   {
      m_resourceRef = nullptr;
   }

   eastl::shared_ptr<t_Resource*> m_resourceRef = nullptr;
};

// Resource reference acquired from the RenderResource. Holds a weak_ptr to the shared_ptr reference. Can be used to be stored for
// longer use
template <typename t_Resource>
class ResourceRef
{
   friend class RenderResource<ResourceRef>;

 public:
   ResourceUse Lock() const
   {
      return ResourceUse(m_resourceWeakRef);
   }

 private:
   ResourceRef() = delete;
   ResourceRef(eastl::shared_ptr<t_Resource*> p_sharedRef)
   {
      m_resourceWeakRef = eastl::weak_ptr<t_Resource*>(p_sharedRef);
   }

   eastl::weak_ptr<t_Resource*> m_resourceWeakRef;
};

// Wrapper around the unique_ptr type to add a custom deleter for all render resources
template <typename t_Resource>
class ResourceUniqueRef
{
   friend class ResourceUniqueRef<t_Resource>;

   // Deleter that is used to check if the shared_ref is being used. If the shared_ref is still being used, it will flag it for
   // removal, and will be removed when the shared_ref is finished. If the shared_ref isn't used, it will call the DeleteResource on
   // the instance
   class ResourceDeleter
   {
      ResourceDeleter() = default;

      template <typename U>
      ResourceDeleter(const ResourceDeleter<U>&, typename eastl::enable_if<is_convertible<U*, T*>::value>::type* = 0)
      {
      }

      void operator()(t_Resource* p_resource) const
      {
         if (p_resource->m_resourceRef.use_count() > 1)
         {
            p_resource->m_deleteFlag = true;
         }
         else
         {
            p_resource->DeleteResource();
         }
      }
   };

 public:
   // Gets the pointer
   t_Resource* operator->()
   {
      m_resourceUnique->get();
   }

   // Returns the unique_ptr
   eastl::unique_ptr<t_Resource, ResourceDeleter>& Get() const
   {
      m_resourceUnique;
   }

 private:
   ResourceUniqueRef() = delete;
   ResourceUniqueRef& operator=(const ResourceUniqueRef& p_other) = delete;
   ResourceUniqueRef(const ResourceUniqueRef& p_other) = delete;

   ResourceUniqueRef(t_Resource* p_resource)
   {
      m_resourceUnique = eastl::unique_ptr<t_Resource>(p_resource);
   }

   ResourceUniqueRef& operator=(ResourceUniqueRef&& p_other)
   {
      m_resourceUnique = eastl::move(p_other.m_resourceUnique);
   }

   ResourceUniqueRef(ResourceUniqueRef&& p_other)
   {
      m_resourceUnique = eastl::move(p_other.m_resourceUnique);
   }

   eastl::unique_ptr<t_Resource, ResourceDeleter> m_resourceUnique;
};

// Base class of a Render Resource. Adds a method to create the instance of a resource, and will create a shared_ptr reference of
// the instance for other objects to reference.
template <typename t_Resource, typename t_Descriptor>
class RenderResource
{
   friend struct ResourceUniqueRef<t_Resource>::ResourceDeleter;

   struct ReferenceDeleter
   {
      void operator()(t_Resource* p_resourceRef) const
      {
         delete p_resourceRef;

         if (p_resourceRef->m_deleteFlag)
         {
            DeleteResource();
         }
      }
   };

 public:
   RenderResource() = delete;
   RenderResource& operator=(const RenderResource& p_other) = delete;
   RenderResource(const RenderResource& p_other) = delete;
   RenderResource& operator=(RenderResource&& p_other) = delete;
   RenderResource(RenderResource&& p_other) = delete;

   static ResourceUniqueRef<t_Resource> CreateInstance(t_Descriptor&& p_desc)
   {
      ResourceUniqueRef resource(new t_Resource(eastl::move(p_desc)));
      resource->InitializeInternal();
      return eastl::move(resource);
   }

   ResourceRef<t_Resource> GetReference()
   {
      return ResourceRef(m_resourceRef);
   }

 protected:
   RenderResource()
   {
      m_resourceRef = eastl::shared_ptr<t_Resource*>(new t_Resource*(this), ReferenceDeleter());
   }

 private:
   virtual void InitializeInternal() override
   {
   }

   virtual void DeleteResource() override
   {
      delete this;
   }

   eastl::shared_ptr<t_Resource*> m_resourceRef = nullptr;
   bool m_deleteFlag = false;
};

} // namespace Render
