#include "ObjectRegistry.hpp"
#include "Red/ISerializable.hpp"

namespace
{
constexpr auto MaxNumObjects = 0x350000;

// Shrink when the registry reaches this size. It grows when most objects are still alive,
// otherwise every new handle would rescan millions of entries under the exclusive lock.
size_t s_shrinkThreshold = MaxNumObjects - 1;

Red::ClassLocator<Red::IScriptable> s_IScriptableType;
}

void App::ObjectRegistry::OnBootstrap()
{
    s_objects.reserve(MaxNumObjects);

    Hook<Raw::ISerializable::CreateHandle>(&OnCreateHandle);
}

void* App::ObjectRegistry::OnCreateHandle(Red::Handle<Red::ISerializable>& aHandle, Red::ISerializable* aObject)
{
    auto isNewHandle = aObject && !aObject->ref.refCount;

    Raw::ISerializable::CreateHandle(aHandle, aObject);

    if (isNewHandle)
    {
        std::unique_lock lock(s_registryLock);
        s_objects.emplace_back(aHandle);

        if (s_objects.size() >= s_shrinkThreshold)
        {
            Shrink();
        }
    }

    return &aHandle;
}

void App::ObjectRegistry::CollectSerializables(Red::DynArray<Red::WeakHandle<Red::ISerializable>>& aOut)
{
    std::shared_lock lock(s_registryLock);

    for (const auto& weak : s_objects)
    {
        if (!weak.Expired())
        {
            aOut.EmplaceBack(weak);
        }
    }
}

void App::ObjectRegistry::CollectScriptables(Red::DynArray<Red::WeakHandle<Red::IScriptable>>& aOut)
{
    std::shared_lock lock(s_registryLock);

    for (const auto& weak : s_objects)
    {
        if (!weak.Expired() && weak.instance->GetType()->IsA(s_IScriptableType))
        {
            aOut.EmplaceBack(*reinterpret_cast<const Red::WeakHandle<Red::IScriptable>*>(&weak));
        }
    }
}

void App::ObjectRegistry::CreateSnapshot()
{
    std::shared_lock lock1(s_registryLock);
    std::unique_lock lock2(s_snapshotLock);

    s_snapshot.clear();

    for (const auto& weak : s_objects)
    {
        if (weak.Expired())
            continue;

        if (!weak.instance->GetType()->IsA(s_IScriptableType))
            continue;

        auto cls = weak.instance->GetType();

        if (!cls->holderSize)
            continue;

        auto it = s_snapshot.find(cls->GetName());

        if (it != s_snapshot.end())
            continue;

        auto& desc = s_snapshot.insert({cls->name, {}}).first.value();

        Red::DynArray<Red::CProperty*> props;
        cls->GetProperties(props);

        for (const auto& prop : props)
        {
            if (prop->flags.inValueHolder)
            {
                desc.props.emplace_back(prop->name, prop->type, prop->valueOffset);
            }
        }
    }
}

void App::ObjectRegistry::RestoreSnapshot()
{
    std::unique_lock lock1(s_registryLock);
    std::unique_lock lock2(s_snapshotLock);

    for (auto& weak : s_objects)
    {
        if (weak.Expired())
            continue;

        if (!weak.instance->GetType()->IsA(s_IScriptableType))
            continue;

        auto instance = reinterpret_cast<Red::IScriptable*>(weak.instance);
        auto oldHolder = instance->valueHolder;

        if (!oldHolder)
            continue;

        auto cls = instance->GetType();
        auto it = s_snapshot.find(cls->GetName());

        if (it == s_snapshot.end())
            continue;

        instance->valueHolder = nullptr;
        auto newHolder = instance->GetValueHolder();

        for (auto& oldProp : it.value().props)
        {
            auto newProp = cls->GetProperty(oldProp.name);

            if (newProp && newProp->flags.inValueHolder && newProp->type == oldProp.type)
            {
                newProp->type->Assign(GetValuePtr(newHolder, newProp->valueOffset),
                                      GetValuePtr(oldHolder, oldProp.valueOffset));
            }

            oldProp.type->Destruct(GetValuePtr(oldHolder, oldProp.valueOffset));
        }
    }

    s_snapshot.clear();

    Shrink();
}

void App::ObjectRegistry::Shrink()
{
    const auto before = s_objects.size();
    auto [b, e] = std::ranges::remove_if(s_objects, [](auto& x) { return x.Expired(); });
    s_objects.erase(b, e);

    const auto threshold = std::max<size_t>(MaxNumObjects - 1, s_objects.size() * 2);
    if (threshold != s_shrinkThreshold)
    {
        LogInfo("ObjectRegistry: {} of {} objects alive, next shrink at {}.", s_objects.size(), before, threshold);
        s_shrinkThreshold = threshold;
    }
}
