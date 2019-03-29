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

ResourceId ResourceLoadingService::LoadResource(const std::string& resourceRelativePath)
{
    const auto resourceId = GetStringHash(resourceRelativePath);
    
    if (mResourceMap.count(resourceId))
    {
        Log(LogType::WARNING, "Resource %s already loaded", resourceRelativePath.c_str());
        return resourceId;
    }
    else
    {
        LoadResourceInternal(resourceRelativePath, resourceId);
        return resourceId;
    }
}

void ResourceLoadingService::LoadResources(const std::vector<std::string>& resourceRelativePaths)
{
    for (const auto path: resourceRelativePaths)
    {
        LoadResource(path);
    }
}

void ResourceLoadingService::UnloadResource(const std::string& resourceRelativePath)
{
    const auto resourceId = GetStringHash(resourceRelativePath);
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

IResource& ResourceLoadingService::GetResource(const std::string& resourceRelativePath)
{
    const auto resourceId = GetStringHash(resourceRelativePath);
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

void ResourceLoadingService::LoadResourceInternal(const std::string& resourceRelativePath, const ResourceId resourceId)
{
    const auto resourceFileExtension = GetFileExtension(resourceRelativePath);
    
    auto loadedResource = mResourceExtensionsToLoadersMap[resourceFileExtension]->VCreateAndLoadResource(RES_ROOT + resourceRelativePath);
    
    mResourceMap[resourceId] = std::move(loadedResource);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////