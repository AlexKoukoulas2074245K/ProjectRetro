//
//  SoundService.cpp
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/08/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#include "SoundService.h"
#include "../common/utils/Logging.h"
#include "../common/utils/OSMessageBox.h"
#include "../resources/MusicResource.h"
#include "../resources/ResourceLoadingService.h"

#include <cassert>
#include <fstream>
#include <SDL_mixer.h>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const int SoundService::SOUND_FREQUENCY = 44100;
const int SoundService::HARDWARE_CHANNELS = 2;
const int SoundService::CHUNK_SIZE_IN_BYTES = 1024;
const int SoundService::FADE_IN_OUT_TOTAL_DURATION_IN_MILISECONDS = 1000;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static void OnMusicFinishedHook()
{
    SoundService::GetInstance().OnMusicFinished();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

SoundService& SoundService::GetInstance()
{
    static SoundService instance;
    return instance;
}

SoundService::~SoundService()
{
    Mix_HookMusicFinished(nullptr);
}

void SoundService::InitializeSdlMixer() const
{
    SDL_version mixerCompiledVersion;
    SDL_MIXER_VERSION(&mixerCompiledVersion);

    const auto* imgLinkedVersion = Mix_Linked_Version();

    const auto mixerMajorVersionConsistency = mixerCompiledVersion.major == imgLinkedVersion->major;
    const auto mixerMinorVersionConsistency = mixerCompiledVersion.minor == imgLinkedVersion->minor;
    const auto mixerPatchConsistency = mixerCompiledVersion.patch == imgLinkedVersion->patch;
    const auto mixerVersionConsistency = mixerMajorVersionConsistency && mixerMinorVersionConsistency && mixerPatchConsistency;

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

void SoundService::PlayMusic(const StringId musicTrackName)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    const auto musicFilePath = ResourceLoadingService::RES_MUSIC_ROOT + musicTrackName.GetString();
    
    if (resourceLoadingService.HasLoadedResource(musicFilePath) == false)
    {
        Log(LogType::WARNING, "Music file %s requested not preloaded", musicFilePath.c_str());
        resourceLoadingService.LoadResource(musicFilePath);
    }

    auto& musicResource = resourceLoadingService.GetResource<MusicResource>(musicFilePath);
    
    if (mCurrentlyPlayingMusicResourceId == 0)
    {
        mCurrentlyPlayingMusicResourceId = resourceLoadingService.GetResourceIdFromPath(musicFilePath);
        Mix_PlayMusic(musicResource.GetSdlMusicHandle(), -1);
    }
    else
    {        
        mQueuedMusicResourceId = resourceLoadingService.GetResourceIdFromPath(musicFilePath);
        Mix_FadeOutMusic(FADE_IN_OUT_TOTAL_DURATION_IN_MILISECONDS/2);
        Mix_HookMusicFinished(OnMusicFinishedHook);
    }    
}

void SoundService::OnMusicFinished()
{
    assert(mQueuedMusicResourceId != 0 && "No queued music to play");

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    auto& musicResource = resourceLoadingService.GetResource<MusicResource>(mQueuedMusicResourceId);

    Mix_FadeInMusic(musicResource.GetSdlMusicHandle(), -1, FADE_IN_OUT_TOTAL_DURATION_IN_MILISECONDS/2);
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool SoundService::HasIntro(const std::string& musicTrackPath) const
{
    std::ifstream file(musicTrackPath + "_intro");
    return file.good();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
