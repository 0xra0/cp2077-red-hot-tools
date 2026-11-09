#include "ResourcePathRegistry.hpp"
#include "App/Project.hpp"
#include "Red/SharedStorage.hpp"

namespace
{
constexpr auto SharedName = Red::CName("ResourcePathRegistryV3" BUILD_SUFFIX);
}

App::ResourcePathRegistry::ResourcePathRegistry(const std::filesystem::path& aPreloadPath)
{
    s_instance = Red::AcquireSharedInstance<SharedName, SharedInstance>();
    s_preloadPath = aPreloadPath;
}

void App::ResourcePathRegistry::OnBootstrap()
{
    std::filesystem::path preloadPath;

    {
        std::unique_lock lock(s_instance->m_lock);

        if (!s_instance->m_initialized)
        {
            s_instance->m_initialized = true;
            s_instance->m_map.reserve(400000);

            HookAfter<Raw::ResourcePath::Create>(&OnCreatePath);
        }

        if (!s_instance->m_preloaded && !s_preloadPath.empty() && std::filesystem::exists(s_preloadPath))
        {
            s_instance->m_preloaded = true;
            preloadPath = s_preloadPath;
        }
    }

    if (!preloadPath.empty())
    {
        std::thread([preloadPath = std::move(preloadPath)]() {
            LogInfo("[ResourcePathRegistry] Loading metadata...");

            // Parse without holding the shared registry lock. The old code
            // kept the exclusive lock for the entire 100k+ line file load,
            // stalling every game thread that created a resource path.
            Core::Map<Red::ResourcePath, std::string> preloadMap;
            preloadMap.reserve(160000);

            std::ifstream f(preloadPath);
            std::string path;
            while (std::getline(f, path))
            {
                const auto hash = Red::ResourcePath::HashSanitized(path.data());
                preloadMap.emplace(hash, std::move(path));
            }

            size_t mapSize = 0;
            {
                std::scoped_lock lock(s_instance->m_lock);

                for (auto& [hash, path] : preloadMap)
                {
                    if (!s_instance->m_map.contains(hash))
                        s_instance->m_map.emplace(hash, std::move(path));
                }

                mapSize = s_instance->m_map.size();
            }

            LogInfo("[ResourcePathRegistry] Loaded {} predefined hashes.", mapSize);
        }).detach();
    }
}

void App::ResourcePathRegistry::OnCreatePath(Red::ResourcePath* aPath, Red::StringView* aPathStr)
{
    if (!aPath || !*aPath || !aPathStr || !*aPathStr)
        return;

    // ResourcePath::Create is a very hot multi-threaded path. Most resources
    // are already present in the preloaded hash map, so never take the
    // exclusive lock just to overwrite the same string again.
    {
        std::shared_lock lock(s_instance->m_lock);
        if (s_instance->m_map.contains(*aPath))
            return;
    }

    std::string path{aPathStr->Data(), aPathStr->Length()};

    std::scoped_lock lock(s_instance->m_lock);
    if (!s_instance->m_map.contains(*aPath))
        s_instance->m_map.emplace(*aPath, std::move(path));
}

std::string App::ResourcePathRegistry::ResolvePath(Red::ResourcePath aPath)
{
    if (!aPath)
        return {};

    std::shared_lock _(s_instance->m_lock);
    const auto& it = s_instance->m_map.find(aPath);

    if (it == s_instance->m_map.end())
        return {};

    return it.value();
}

std::string App::ResourcePathRegistry::ResolvePathOrHash(Red::ResourcePath aPath)
{
    auto str = ResolvePath(aPath);

    if (str.empty())
    {
        str = std::to_string(aPath.hash);
    }

    return str;
}

Red::ResourcePath App::ResourcePathRegistry::RegisterPath(const std::string& aPathStr)
{
    if (aPathStr.empty())
        return {};

    auto path = Red::ResourcePath(aPathStr.data());

    RegisterPath(path, aPathStr);

    return path;
}

void App::ResourcePathRegistry::RegisterPath(Red::ResourcePath aPath, const std::string& aPathStr)
{
    if (!aPath)
        return;

    {
        std::shared_lock _(s_instance->m_lock);
        if (s_instance->m_map.contains(aPath))
            return;
    }

    {
        std::scoped_lock _(s_instance->m_lock);
        s_instance->m_map[aPath] = aPathStr;
    }
}
