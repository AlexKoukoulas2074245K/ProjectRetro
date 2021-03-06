//
//  TextureLoader.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef TextureLoader_h
#define TextureLoader_h

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "IResourceLoader.h"

#include <memory>
#include <SDL_stdinc.h>
#include <set>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

struct SDL_Surface;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class TextureLoader final: public IResourceLoader
{
    friend class ResourceLoadingService;

public:
    void VInitialize() override;
    std::unique_ptr<IResource> VCreateAndLoadResource(const std::string& path) const override;

private:
    TextureLoader() = default;

    std::set<Uint32> ExtractColorSet(SDL_Surface* const) const;
};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* TextureLoader_h */
