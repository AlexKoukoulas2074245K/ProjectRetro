//
//  ResourceLoadingService.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef ResourceLoadingService_h
#define ResourceLoadingService_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "../common/utils/StringUtils.h"

#include <memory>
#include <string>        
#include <unordered_map>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using ResourceId = unsigned int;
class IResource;
class IResourceLoader;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct ResourceIdHasher
{
    std::size_t operator()(const ResourceId& key) const
    {
        return static_cast<std::size_t>(key);
    }
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class ResourceLoadingService final
{
public:
    static const std::string RES_ROOT;
    static const std::string RES_ATLASES_ROOT;
    static const std::string RES_DATA_ROOT;
    static const std::string ENCOUNTER_DATA_ROOT;
    static const std::string RES_LEVELS_ROOT;    
    static const std::string RES_MODELS_ROOT;
    static const std::string RES_MUSIC_ROOT;
    static const std::string RES_SFX_ROOT;
    static const std::string RES_SHADERS_ROOT;
    static const std::string RES_TEXTURES_ROOT;        
    
    static ResourceLoadingService& GetInstance();

    ~ResourceLoadingService();
    ResourceLoadingService(const ResourceLoadingService&) = delete;
    ResourceLoadingService(ResourceLoadingService&&) = delete;
    const ResourceLoadingService& operator = (const ResourceLoadingService&) = delete;
    ResourceLoadingService& operator = (ResourceLoadingService&&) = delete;

    // Initializes loaders for different types of assets. 
    // Should be called after the SDL/GL context has been initialized
    void InitializeResourceLoaders();
    
    // Calculates a resource id from a given path
    ResourceId GetResourceIdFromPath(const std::string& resourcePath);

    // Loads a single or number of resources based on the relative path(s) supplied
    // respectively. Both full paths, relative paths including the Resource Root, and relative
    // paths excluding the Resource Root are supported
    ResourceId LoadResource(const std::string& resourcePath);
    void LoadResources(const std::vector<std::string>& resourcePaths);
    
    // Checks whether the resource with the given path exists
    bool DoesResourceExist(const std::string& resourcePath) const;
    
    // Checks whether the given resource has been loaded.
    // Both full paths, relative paths including the Resource Root, and relative
    // paths not including the Resource Root are supported
    bool HasLoadedResource(const std::string& resourcePath) const;
    
    // Unloads the specified resource. Any subsequent calls to get that 
    // resource will need to be preceeded by another Load to get the resource 
    // back to the map of resources held by this service
    void UnloadResource(const std::string& resourcePath);
    void UnloadResource(const ResourceId resourceId);
    
    // Both full paths, relative paths including the Resource Root, and relative
    // paths not including the Resource Root are supported
    template<class ResourceType>
    inline ResourceType& GetResource(const std::string& resourcePath)
    {
        return static_cast<ResourceType&>(GetResource(resourcePath));
    }

    template<class ResourceType>
    inline ResourceType& GetResource(const ResourceId resourceId)
    {
        return static_cast<ResourceType&>(GetResource(resourceId));
    }

    // Opens a file based on the relative file path given, and writes the string str to it
    void WriteStringToFile(const std::string& str, const std::string& relativeFilePath);

    // Checks whether a file with the given relative file path exists
    bool DoesFileExist(const std::string& relativeFilePath);
    
private:    
    ResourceLoadingService() = default;

    IResource& GetResource(const std::string& resourceRelativePath);
    IResource& GetResource(const ResourceId resourceId);    
    void LoadResourceInternal(const std::string& resourceRelativePath, const ResourceId resourceId);
   
    // Strips the leading RES_ROOT from the resourcePath given, if present
    std::string AdjustResourcePath(const std::string& resourcePath) const;
    
private:
    std::unordered_map<ResourceId, std::unique_ptr<IResource>, ResourceIdHasher> mResourceMap;
    std::unordered_map<StringId, IResourceLoader*, StringIdHasher> mResourceExtensionsToLoadersMap;
    std::vector<std::unique_ptr<IResourceLoader>> mResourceLoaders;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* ResourceLoadingService_h */
