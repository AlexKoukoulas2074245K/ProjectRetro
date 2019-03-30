//
//  ResourceLoadingService.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "ResourceLoadingService.h"
#include "DataFileLoader.h"
#include "IResource.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"
#include "../common_utils/FileUtils.h"
#include "../common_utils/Logging.h"
#include "../common_utils/MessageBox.h"
#include "../common_utils/StringUtils.h"
#include "../common_utils/TypeTraits.h"

#include <cassert>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
const std::string ResourceLoadingService::RES_ROOT = "../res/";
#else
const std::string ResourceLoadingService::RES_ROOT = "../../res/";
#endif

const std::string ResourceLoadingService::RES_SHADERS_ROOT = RES_ROOT + "shaders/";

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ResourceLoadingService& ResourceLoadingService::GetInstance()
{
    static ResourceLoadingService instance;
    return instance;
}

ResourceLoadingService::~ResourceLoadingService()
{
}

void ResourceLoadingService::InitializeResourceLoaders()
{
    // No make unique due to constructing the loaders with their private constructors
    // via friendship
    mResourceLoaders.push_back(std::unique_ptr<TextureLoader>(new TextureLoader));
    mResourceLoaders.push_back(std::unique_ptr<DataFileLoader>(new DataFileLoader));
    mResourceLoaders.push_back(std::unique_ptr<ShaderLoader>(new ShaderLoader));
    
    // Map resource extensions to loaders
    mResourceExtensionsToLoadersMap[StringId("png")]  = mResourceLoaders[0].get();
    mResourceExtensionsToLoadersMap[StringId("json")] = mResourceLoaders[1].get();
    mResourceExtensionsToLoadersMap[StringId("vs")]   = mResourceLoaders[2].get();
    mResourceExtensionsToLoadersMap[StringId("fs")]   = mResourceLoaders[2].get();
    
    for (auto& resourceLoader: mResourceLoaders)
    {
        resourceLoader->VInitialize();
    }
}

ResourceId ResourceLoadingService::LoadResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = GetStringHash(adjustedPath);
    
    if (mResourceMap.count(resourceId))
    {
        Log(LogType::WARNING, "Resource %s already loaded", adjustedPath.c_str());
        return resourceId;
    }
    else
    {
        LoadResourceInternal(adjustedPath, resourceId);
        return resourceId;
    }
}

void ResourceLoadingService::LoadResources(const std::vector<std::string>& resourcePaths)
{
    for (const auto path: resourcePaths)
    {
        LoadResource(path);
    }
}

bool ResourceLoadingService::HasLoadedResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = GetStringHash(adjustedPath);
    
    return mResourceMap.count(resourceId) != 0;
}

void ResourceLoadingService::UnloadResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = GetStringHash(adjustedPath);
    mResourceMap.erase(resourceId);
}

void ResourceLoadingService::UnloadResource(const ResourceId resourceId)
{
    mResourceMap.erase(resourceId);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

ResourceLoadingService::ResourceLoadingService()
{
    InitializeResourceLoaders();
}

IResource& ResourceLoadingService::GetResource(const std::string& resourcePath)
{
    const auto adjustedPath = AdjustResourcePath(resourcePath);
    const auto resourceId = GetStringHash(adjustedPath);
    return GetResource(resourceId);
}

IResource& ResourceLoadingService::GetResource(const ResourceId resourceId)
{
    if (mResourceMap.count(resourceId))
    {
        return *mResourceMap[resourceId];
    }

    assert(false && "Resource could not be found");
    return *mResourceMap[resourceId];
}

void ResourceLoadingService::LoadResourceInternal(const std::string& resourcePath, const ResourceId resourceId)
{
    // Get resource extension
    const auto resourceFileExtension = GetFileExtension(resourcePath);
    
    // Pick appropriate loader
    auto& selectedLoader = mResourceExtensionsToLoadersMap.at(StringId(GetFileExtension(resourcePath)));
    auto loadedResource = selectedLoader->VCreateAndLoadResource(RES_ROOT + resourcePath);
    
    assert(loadedResource != nullptr && "No loader was able to load resource");
    mResourceMap[resourceId] = std::move(loadedResource);
}

std::string ResourceLoadingService::AdjustResourcePath(const std::string& resourcePath) const
{    
    return !StringStartsWith(resourcePath, RES_ROOT) ? resourcePath : resourcePath.substr(RES_ROOT.size(), resourcePath.size() - RES_ROOT.size());
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
