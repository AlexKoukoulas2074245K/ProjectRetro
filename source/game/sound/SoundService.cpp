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
#include "../resources/SfxResource.h"

#include <cassert>
#include <fstream>
#include <SDL_mixer.h>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

const std::string SoundService::MUSIC_FILE_EXTENSION = ".ogg";
const std::string SoundService::SFX_FILE_EXTENSION   = ".wav";

const int SoundService::SOUND_FREQUENCY                  = 44100;
const int SoundService::HARDWARE_CHANNELS                = 2;
const int SoundService::CHUNK_SIZE_IN_BYTES              = 1024;
const int SoundService::FADE_OUT_DURATION_IN_MILISECONDS = 1000;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

static void OnMusicFinishedHook()
{
    SoundService::GetInstance().OnMusicFinished();
}

static void OnMusicIntroFinishedHook()
{
    SoundService::GetInstance().OnMusicIntroFinished();
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

void SoundService::PlaySfx(const StringId sfxName, const bool overrideCurrentPlaying /* true */)
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    const auto sfxFilePath = ResourceLoadingService::RES_SFX_ROOT + sfxName.GetString();
    auto sfxFilePathWithExtension = sfxFilePath + SFX_FILE_EXTENSION;

    if (resourceLoadingService.HasLoadedResource(sfxFilePathWithExtension) == false)
    {
        Log(LogType::WARNING, "Sfx file %s requested not preloaded", sfxFilePathWithExtension.c_str());
        resourceLoadingService.LoadResource(sfxFilePathWithExtension);
    }

    auto& sfxResource = resourceLoadingService.GetResource<SfxResource>(sfxFilePathWithExtension);

    if (overrideCurrentPlaying || Mix_Playing(1) == false)
    {
        mLastPlayedSfxName = sfxName;
        Mix_PlayChannel(1, sfxResource.GetSdlSfxHandle(), 0);        
    }    
}

void SoundService::PlayMusic(const StringId musicTrackName, const bool fadeOutEnabled /* true */)
{    
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();

    const auto musicFilePath        = ResourceLoadingService::RES_MUSIC_ROOT + musicTrackName.GetString();    
    auto musicFilePathWithExtension = musicFilePath + MUSIC_FILE_EXTENSION;    

    if (resourceLoadingService.HasLoadedResource(musicFilePathWithExtension) == false)
    {
        Log(LogType::WARNING, "Music file %s requested not preloaded", musicFilePathWithExtension.c_str());
        resourceLoadingService.LoadResource(musicFilePathWithExtension);
    }

    // If music track has an intro part, the intro 
    // gets played first, and the core part is saved
    const auto hasIntro = HasIntro(musicFilePath);
    if (hasIntro)
    {        
        musicFilePathWithExtension = musicFilePath + "_intro" + MUSIC_FILE_EXTENSION;
        if (resourceLoadingService.HasLoadedResource(musicFilePathWithExtension) == false)
        {
            Log(LogType::WARNING, "Music intro file %s requested not preloaded", musicFilePathWithExtension.c_str());
            resourceLoadingService.LoadResource(musicFilePathWithExtension);
        }

        mCoreMusicTrackResourceId = resourceLoadingService.GetResourceIdFromPath(musicFilePath + MUSIC_FILE_EXTENSION);
    }
    else
    {
        mCoreMusicTrackResourceId = 0;
    }
    
    auto& musicResource = resourceLoadingService.GetResource<MusicResource>(musicFilePathWithExtension);
    
    if (mCurrentlyPlayingMusicResourceId == 0 || fadeOutEnabled == false)
    {
        mCurrentlyPlayingMusicResourceId = resourceLoadingService.GetResourceIdFromPath(musicFilePathWithExtension);

        if (hasIntro)
        {   
            Mix_HookMusicFinished(fadeOutEnabled ? OnMusicFinishedHook : OnMusicIntroFinishedHook);               
            Mix_PlayMusic(musicResource.GetSdlMusicHandle(), 0);
        }
        else
        {            
            Mix_PlayMusic(musicResource.GetSdlMusicHandle(), -1);
        }               

        if (mMusicVolumePriorToSilence == -1)
        {
            mMusicVolumePriorToSilence = Mix_VolumeMusic(-1);
        }
        Mix_VolumeMusic(mMusicVolumePriorToSilence);
    }
    else
    {        
        mQueuedMusicResourceId = resourceLoadingService.GetResourceIdFromPath(musicFilePathWithExtension);
        Mix_FadeOutMusic(FADE_OUT_DURATION_IN_MILISECONDS);
        Mix_HookMusicFinished(OnMusicFinishedHook);
    }    
}

void SoundService::SilenceMusic()
{
    mMusicVolumePriorToSilence = Mix_VolumeMusic(-1);
    Mix_VolumeMusic(0);
}

void SoundService::OnMusicFinished()
{
    assert(mQueuedMusicResourceId != 0 && "No queued music to play");

    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    auto& musicResource = resourceLoadingService.GetResource<MusicResource>(mQueuedMusicResourceId);

    mCurrentlyPlayingMusicResourceId = mQueuedMusicResourceId;
    if (mCoreMusicTrackResourceId != 0)
    {
        Mix_HookMusicFinished(OnMusicIntroFinishedHook);
        Mix_PlayMusic(musicResource.GetSdlMusicHandle(), 0);
    }
    else
    {
        Mix_PlayMusic(musicResource.GetSdlMusicHandle(), -1);
    }        

    Mix_VolumeMusic(mMusicVolumePriorToSilence);
}

void SoundService::OnMusicIntroFinished()
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    auto& musicResource = resourceLoadingService.GetResource<MusicResource>(mCoreMusicTrackResourceId);

    mCurrentlyPlayingMusicResourceId = mCoreMusicTrackResourceId;
    Mix_PlayMusic(musicResource.GetSdlMusicHandle(), -1);

    Mix_VolumeMusic(mMusicVolumePriorToSilence);
}

bool SoundService::IsPlayingSfx() const
{
    return Mix_Playing(1) != 0;
}

StringId SoundService::GetLastPlayedSfxName() const
{
    return mLastPlayedSfxName;
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

bool SoundService::HasIntro(const std::string& musicTrackPath) const
{
    std::ifstream file(musicTrackPath + "_intro" + MUSIC_FILE_EXTENSION);
    return file.good();
}

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
