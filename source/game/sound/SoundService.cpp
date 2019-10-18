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

const int SoundService::SFX_CHANNEL_NUMBER               = 1;
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

static void OnSfxFinishedHook(const int channel)
{
    if (channel == SoundService::SFX_CHANNEL_NUMBER)
    {
        SoundService::GetInstance().OnSfxFinished();
    }
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

void SoundService::PlaySfx(const StringId sfxName, const bool overrideCurrentPlaying /* true */, const bool shouldMuteMusicWhilePlaying /* false */)
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
        Mix_PlayChannel(SFX_CHANNEL_NUMBER, sfxResource.GetSdlSfxHandle(), 0);
        
        if (shouldMuteMusicWhilePlaying)
        {
            MuteMusic();
            Mix_ChannelFinished(OnSfxFinishedHook);
        }
        else
        {
            Mix_ChannelFinished(nullptr);
        }
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
            Mix_HookMusicFinished(OnMusicIntroFinishedHook);               
            Mix_PlayMusic(musicResource.GetSdlMusicHandle(), 0);
        }
        else
        {            
            Mix_PlayMusic(musicResource.GetSdlMusicHandle(), -1);
        }               

        if (!mAllAudioDisabled)
        {
            if (mMusicVolumePriorToMuting == -1)
            {
                mMusicVolumePriorToMuting = Mix_VolumeMusic(-1);
            }
            
            Mix_VolumeMusic(mMusicVolumePriorToMuting);
        }
    }
    else
    {        
        mQueuedMusicResourceId = resourceLoadingService.GetResourceIdFromPath(musicFilePathWithExtension);
        Mix_FadeOutMusic(FADE_OUT_DURATION_IN_MILISECONDS);
        Mix_HookMusicFinished(OnMusicFinishedHook);
    }    
}

void SoundService::MuteMusic()
{
    if (!mAllAudioDisabled)
    {
        mMusicVolumePriorToMuting = Mix_VolumeMusic(-1);
        Mix_VolumeMusic(0);
    }    
}

void SoundService::UnmuteMusic()
{
    if (!mAllAudioDisabled)
    {
        Mix_VolumeMusic(mMusicVolumePriorToMuting);
    }
    
}

void SoundService::MuteSfx()
{
    if (!mAllAudioDisabled)
    {
        mSfxVolumePriorToMuting = Mix_Volume(SFX_CHANNEL_NUMBER, -1);
        Mix_Volume(SFX_CHANNEL_NUMBER, 0);
    }    
}

void SoundService::UnmuteSfx()
{
    if (!mAllAudioDisabled)
    {
        Mix_Volume(SFX_CHANNEL_NUMBER, mMusicVolumePriorToMuting);
    }
}

void SoundService::ToggleAudioOnOff()
{
    mAllAudioDisabled = !mAllAudioDisabled;
    if (!mAllAudioDisabled)
    {
        Mix_VolumeMusic(mMusicVolumePriorToMuting);
        Mix_Volume(SFX_CHANNEL_NUMBER, mMusicVolumePriorToMuting);
    }
    else
    {
        mMusicVolumePriorToMuting = Mix_VolumeMusic(-1);
        Mix_VolumeMusic(0);
        mSfxVolumePriorToMuting = Mix_Volume(SFX_CHANNEL_NUMBER, -1);
        Mix_Volume(SFX_CHANNEL_NUMBER, 0);
    }
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

    if (!mAllAudioDisabled)
    {
        Mix_VolumeMusic(mMusicVolumePriorToMuting);
    }
}

void SoundService::OnMusicIntroFinished()
{
    auto& resourceLoadingService = ResourceLoadingService::GetInstance();
    auto& musicResource = resourceLoadingService.GetResource<MusicResource>(mCoreMusicTrackResourceId);

    mCurrentlyPlayingMusicResourceId = mCoreMusicTrackResourceId;
    Mix_PlayMusic(musicResource.GetSdlMusicHandle(), -1);

    if (!mAllAudioDisabled)
    {
        Mix_VolumeMusic(mMusicVolumePriorToMuting);
    }
}

void SoundService::OnSfxFinished()
{
    UnmuteMusic();
}

bool SoundService::IsPlayingMusic() const
{
    return Mix_PlayingMusic() != 0;
}

bool SoundService::IsPlayingSfx() const
{
    return Mix_Playing(SFX_CHANNEL_NUMBER) != 0;
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
