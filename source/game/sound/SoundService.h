//
//  SoundService.h
//  ProjectRetro
//
//  Created by Alex Koukoulas on 29/08/2019.
//


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#ifndef SoundService_h
#define SoundService_h

#include "../common/utils/StringUtils.h"

#include <functional>

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

using MusicResourceId = unsigned int;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

class SoundService final
{    
    friend class App;

public:
    static SoundService& GetInstance();

    ~SoundService();
    SoundService(const SoundService&) = delete;
    SoundService(SoundService&&) = delete;
    const SoundService& operator = (const SoundService&) = delete;
    SoundService& operator = (SoundService&&) = delete;

    void InitializeSdlMixer() const;
    
    void PlaySfx(const StringId sfxName, const bool overrideCurrentPlaying = true, const bool shouldMuteMusic = false);
    void PlayMusic(const StringId musicTrackName, const bool fadeOutEnabled = true);
    void MuteMusic();
    void UnmuteMusic();

    void OnMusicFinished();
    void OnMusicIntroFinished();    
    void OnSfxFinished();
    
    bool IsPlayingSfx() const;
    StringId GetLastPlayedSfxName() const;
    
private:    
    SoundService() = default;    
    
    bool HasIntro(const std::string& musicTrackPath) const;
    
    static const std::string MUSIC_FILE_EXTENSION;
    static const std::string SFX_FILE_EXTENSION;

    static const int SOUND_FREQUENCY;
    static const int HARDWARE_CHANNELS;
    static const int CHUNK_SIZE_IN_BYTES;
    static const int FADE_OUT_DURATION_IN_MILISECONDS;

    MusicResourceId mCurrentlyPlayingMusicResourceId = 0;
    MusicResourceId mQueuedMusicResourceId = 0;
    MusicResourceId mCoreMusicTrackResourceId = 0;    

    StringId mLastPlayedSfxName    = StringId();
    int mMusicVolumePriorToMuting = -1;

};

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

#endif /* SoundService_h */
