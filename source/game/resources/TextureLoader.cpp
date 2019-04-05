//
//  TextureLoader.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/03/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "TextureLoader.h"
#include "TextureResource.h"
#include "../common/utils/Logging.h"
#include "../common/utils/MessageBox.h"
#include "../rendering/opengl/Context.h"

#include <fstream>     // ifstream
#include <SDL_image.h>
#include <SDL.h>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void TextureLoader::VInitialize()
{
    SDL_version imgCompiledVersion;
    SDL_IMAGE_VERSION(&imgCompiledVersion);
    
    const auto* imgLinkedVersion = IMG_Linked_Version();
    
    const auto imgMajorVersionConsistency = imgCompiledVersion.major == imgLinkedVersion->major;
    const auto imgMinorVersionConsistency = imgCompiledVersion.minor == imgLinkedVersion->minor;
    const auto imgPatchConsistency = imgCompiledVersion.patch == imgLinkedVersion->patch;
    const auto imgVersionConsistency = imgMajorVersionConsistency && imgMinorVersionConsistency && imgPatchConsistency;
    
    const auto sdlImageInitFlags = IMG_INIT_PNG;
    if (!imgVersionConsistency || IMG_Init(sdlImageInitFlags) != sdlImageInitFlags)
    {
        ShowMessageBox(MessageBoxType::ERROR, "SDL_image", "SDL_image did not initialize properly");        
    }
    
    Log(LogType::INFO, "Successfully initialized SDL_image version %d.%d.%d", imgCompiledVersion.major, imgCompiledVersion.minor, imgCompiledVersion.patch);        
}

std::unique_ptr<IResource> TextureLoader::VCreateAndLoadResource(const std::string& resourcePath) const
{
    std::ifstream file(resourcePath);
    
    if (!file.good())
    {
        ShowMessageBox(MessageBoxType::ERROR, "File could not be found", resourcePath.c_str());
        return nullptr;
    }
    
    auto* sdlSurface = IMG_Load(resourcePath.c_str());
    if (!sdlSurface)
    {
        ShowMessageBox(MessageBoxType::ERROR, "SDL_image could not load texture", IMG_GetError());
        return nullptr;
    }
    
    SDL_LockSurface(sdlSurface);
    const auto hasTransparentPixels = HasTransparentPixels(sdlSurface);
    SDL_UnlockSurface(sdlSurface);

    GLuint glTextureId;
    GL_CHECK(glGenTextures(1, &glTextureId));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, glTextureId));
    
    int mode;
    switch (sdlSurface->format->BytesPerPixel)
    {
        case 4:
            mode = GL_RGBA;
            break;
        case 3:
            mode = GL_RGB;
            break;
        default:
            throw std::runtime_error("Image with unknown channel profile");
            break;
    }
    
#ifdef _WIN32
    const auto textureFormat = GL_RGBA;
#else
    const auto textureFormat = GL_BGRA;
#endif
    
    GL_CHECK(glTexImage2D
    (
        GL_TEXTURE_2D,
        0,
        mode,
        sdlSurface->w,
        sdlSurface->h,
        0,
        textureFormat,
        GL_UNSIGNED_BYTE,
        sdlSurface->pixels
     ));
    
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    
    return std::unique_ptr<IResource>(new TextureResource(sdlSurface, glTextureId, hasTransparentPixels));
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool TextureLoader::HasTransparentPixels(SDL_Surface* const sdlSurface) const
{    
    auto* pixels = (Uint32*)sdlSurface->pixels;
    for (int y = 0; y < sdlSurface->h; ++y)
    {
        for (int x = 0; x < sdlSurface->w; ++x)
        {                                  
            auto pixel = pixels[y * sdlSurface->w + x];
            if (pixel == 0)
            {                
                return true;
            }
        }
    }

    return false;    
}