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
#include "TextureLoader.h"
#include "DataFileLoader.h"
#include "ShaderLoader.h"
#include "IResource.h"
#include "../common_utils/TypeTraits.h"
#include "../common_utils/Logging.h"
#include "../common_utils/FileUtils.h"
#include "../common_utils/MessageBox.h"
#include "../common_utils/StringUtils.h"

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
    mTextureLoader->VInitialize();
    mDataFileLoader->VInitialize();     
    mShaderLoader->VInitialize();
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
    : mDataFileLoader(new DataFileLoader)
    , mTextureLoader(new TextureLoader)
    , mShaderLoader(new ShaderLoader)

{
    MapResourceExtensionsToLoaders();
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

    Log(LogType::ERROR, "Resource not loaded when requested. Aborting");
    assert(false);
    return *mResourceMap[resourceId];
}

void ResourceLoadingService::MapResourceExtensionsToLoaders()
{
    mResourceExtensionsToLoadersMap["png"]  = mTextureLoader.get();
    mResourceExtensionsToLoadersMap["json"] = mDataFileLoader.get();
    mResourceExtensionsToLoadersMap["vs"]   = mShaderLoader.get();
    mResourceExtensionsToLoadersMap["fs"]   = mShaderLoader.get();
}

void ResourceLoadingService::LoadResourceInternal(const std::string& resourcePath, const ResourceId resourceId)
{
    const auto resourceFileExtension = GetFileExtension(resourcePath);
    
    auto loadedResource = mResourceExtensionsToLoadersMap[resourceFileExtension]->VCreateAndLoadResource(RES_ROOT + resourcePath);
    
    mResourceMap[resourceId] = std::move(loadedResource);
}

std::string ResourceLoadingService::AdjustResourcePath(const std::string& resourcePath) const
{    
    return !StringStartsWith(resourcePath, RES_ROOT) ? resourcePath : resourcePath.substr(RES_ROOT.size(), resourcePath.size() - RES_ROOT.size());
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
