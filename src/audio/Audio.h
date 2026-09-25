#ifndef AUDIO_H
#define AUDIO_H

#include <windows.h>
#include <mmsystem.h>

class AudioManager {
public:
    static void init();
    static void cleanup();
    
    // MP3 Soundtrack control
    static void playSong();
    static void pauseSong();
    static void stopSong();
    static void restartSong();

    // SFX
    static void playChant(float tempoMultiplier);
    static void stopChant();
    static void playGunshot();
    static void playTimerBeep();
    static void playHeadTurnClick();
    static void playEliminationSound();
    static void playWinMusic();
    static void playGameOverMusic();
    static void stopAll();
};

#endif // AUDIO_H
