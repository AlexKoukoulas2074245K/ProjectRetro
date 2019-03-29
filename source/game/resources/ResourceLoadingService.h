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

#include <string>        
#include <vector>        
#include <memory>        
#include <unordered_map>      

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
    friend class App;
public:
    static const std::string RES_ROOT;
    static const std::string RES_SHADERS_ROOT;
    
    static ResourceLoadingService& GetInstance();

    ~ResourceLoadingService();
    ResourceLoadingService(const ResourceLoadingService&) = delete;
    ResourceLoadingService(ResourceLoadingService&&) = delete;
    const ResourceLoadingService& operator = (const ResourceLoadingService&) = delete;
    ResourceLoadingService& operator = (ResourceLoadingService&&) = delete;

    // Initializes loaders for different types of assets. 
    // Should be called after the SDL/GL context has been initialized
    void InitializeResourceLoaders();
    
    // Loads a single or number of resouces base on the relative path(s) supplied
    // respectively
    ResourceId LoadResource(const std::string& resourcePath);
    void LoadResources(const std::vector<std::string>& resourcePaths);
    
    // Unloads the specified resource. Any subsequent calls to get that 
    // resource will need to be preceeded by another Load to get the resource 
    // back to the map of resources held by this service
    void UnloadResource(const std::string& resourcePath);
    void UnloadResource(const ResourceId resourceId);
          
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

private:    
    ResourceLoadingService();

    IResource& GetResource(const std::string& resourceRelativePath);
    IResource& GetResource(const ResourceId resourceId);
    void MapResourceExtensionsToLoaders();
    void LoadResourceInternal(const std::string& resourceRelativePath, const ResourceId resourceId);
   
    std::string AdjustResourcePath(const std::string& resourcePath) const;
    
private:
    std::unordered_map<ResourceId, std::unique_ptr<IResource>, ResourceIdHasher> mResourceMap;
    std::unordered_map<std::string, IResourceLoader*> mResourceExtensionsToLoadersMap;
    
    std::unique_ptr<IResourceLoader> mDataFileLoader;
    std::unique_ptr<IResourceLoader> mTextureLoader;  
    std::unique_ptr<IResourceLoader> mShaderLoader;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* ResourceLoadingService_h */
