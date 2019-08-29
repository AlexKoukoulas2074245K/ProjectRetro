//
//  MusicLoader.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/08/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "MusicLoader.h"
#include "MusicResource.h"
#include "../common/utils/Logging.h"
#include "../common/utils/OSMessageBox.h"

#include <fstream>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int MusicLoader::SOUND_FREQUENCY     = 44100;
const int MusicLoader::HARDWARE_CHANNELS   = 2;
const int MusicLoader::CHUNK_SIZE_IN_BYTES = 1024;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

void MusicLoader::VInitialize()
{
    SDL_version mixerCompiledVersion;
    SDL_MIXER_VERSION(&mixerCompiledVersion);
    
    const auto* imgLinkedVersion = Mix_Linked_Version();
    
    const auto mixerMajorVersionConsistency = mixerCompiledVersion.major == imgLinkedVersion->major;
    const auto mixerMinorVersionConsistency = mixerCompiledVersion.minor == imgLinkedVersion->minor;
    const auto mixerPatchConsistency        = mixerCompiledVersion.patch == imgLinkedVersion->patch;
    const auto mixerVersionConsistency      = mixerMajorVersionConsistency && mixerMinorVersionConsistency && mixerPatchConsistency;
    
    const auto sdlMixerInitFlags = MIX_INIT_OGG;
    if (!mixerVersionConsistency || Mix_Init(sdlMixerInitFlags) != sdlMixerInitFlags)
    {
        ShowMessageBox(MessageBoxType::ERROR, "SDL_Mixer", "SDL_Mixer was not initialize properly");        
    }
    
    if (Mix_OpenAudio(SOUND_FREQUENCY, MIX_DEFAULT_FORMAT, HARDWARE_CHANNELS, CHUNK_SIZE_IN_BYTES) < 0)
    {
        ShowMessageBox(MessageBoxType::ERROR, "SDL_Mixer", "SDL_Mixer was not initialize properly");
    }

    Log(LogType::INFO, "Successfully initialized SDL_Mixer version %d.%d.%d", mixerCompiledVersion.major, mixerCompiledVersion.minor, mixerCompiledVersion.patch);        
}

std::unique_ptr<IResource> MusicLoader::VCreateAndLoadResource(const std::string& resourcePath) const
{       
    std::ifstream file(resourcePath);

    if (!file.good())
    {
        ShowMessageBox(MessageBoxType::ERROR, "File could not be found", resourcePath.c_str());
        return nullptr;
    }

    auto* loadedMusic = Mix_LoadMUS(resourcePath.c_str());
    if (!loadedMusic)
    {
        ShowMessageBox(MessageBoxType::ERROR, "SDL_mixer could not load music", Mix_GetError());
        return nullptr;
    }

    return std::unique_ptr<IResource>(new MusicResource(loadedMusic));
}
