#include "audio/Audio.h"
#include <vector>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <windows.h>
#include <mmsystem.h>

namespace {
    const int SAMPLE_RATE = 22050;
    const float PI = 3.14159265358979323846f;

    struct WavBuffer {
        std::vector<char> data;
    };

    WavBuffer chantNormal;
    WavBuffer gunshotSound;
    WavBuffer timerBeepSound;
    WavBuffer headTurnSound;
    WavBuffer eliminationSound;
    WavBuffer winSound;
    WavBuffer gameOverSound;
    WavBuffer glassShatterSound;
    WavBuffer glassStepSound;
    WavBuffer fallWhooshSound;

    bool mp3Loaded = false;
    char foundMp3Path[MAX_PATH] = {0};

    // Builds a standard 16-bit mono RIFF WAV in memory
    WavBuffer createWav(const std::vector<short>& samples) {
        WavBuffer buf;
        int numSamples = (int)samples.size();
        int dataSize = numSamples * sizeof(short);
        int totalSize = 36 + dataSize;

        buf.data.resize(44 + dataSize);
        char* p = buf.data.data();

        memcpy(p, "RIFF", 4);
        memcpy(p + 4, &totalSize, 4);
        memcpy(p + 8, "WAVE", 4);

        memcpy(p + 12, "fmt ", 4);
        int fmtChunkSize = 16;
        short audioFormat = 1;
        short numChannels = 1;
        int sampleRate = SAMPLE_RATE;
        int byteRate = sampleRate * numChannels * 2;
        short blockAlign = numChannels * 2;
        short bitsPerSample = 16;

        memcpy(p + 16, &fmtChunkSize, 4);
        memcpy(p + 20, &audioFormat, 2);
        memcpy(p + 22, &numChannels, 2);
        memcpy(p + 24, &sampleRate, 4);
        memcpy(p + 28, &byteRate, 4);
        memcpy(p + 32, &blockAlign, 2);
        memcpy(p + 34, &bitsPerSample, 2);

        memcpy(p + 36, "data", 4);
        memcpy(p + 40, &dataSize, 4);
        memcpy(p + 44, samples.data(), dataSize);

        return buf;
    }

    void appendDollNote(std::vector<short>& out, float freq, float durationSec, float attackSec = 0.04f, float decaySec = 0.08f) {
        int count = (int)(durationSec * SAMPLE_RATE);
        int attackCount = (int)(attackSec * SAMPLE_RATE);
        int decayCount = (int)(decaySec * SAMPLE_RATE);
        if (attackCount < 1) attackCount = 1;
        if (decayCount < 1) decayCount = 1;

        double phase = 0.0;
        double vibratoPhase = 0.0;

        for (int i = 0; i < count; ++i) {
            float env = 1.0f;
            if (i < attackCount) {
                float x = (float)i / (float)attackCount;
                env = 0.5f * (1.0f - cosf(PI * x)); // Smooth raised-cosine attack (zero clicks)
            } else if (i > count - decayCount) {
                float x = (float)(count - i) / (float)decayCount;
                env = 0.5f * (1.0f - cosf(PI * x)); // Smooth raised-cosine release
            }

            // Continuous phase accumulation: eliminates FM derivative glitching
            float vibrato = 1.0f + 0.012f * (float)sin(vibratoPhase);
            vibratoPhase += (2.0 * PI * 5.0) / (double)SAMPLE_RATE;
            if (vibratoPhase > 2.0 * PI) vibratoPhase -= 2.0 * PI;

            phase += (2.0 * PI * (double)(freq * vibrato)) / (double)SAMPLE_RATE;
            if (phase > 2.0 * PI) phase -= 2.0 * PI;

            // Warm music-box chime timbre: fundamental + gentle harmonics
            float s = 0.72f * (float)sin(phase)
                    + 0.22f * (float)sin(2.0 * phase)
                    + 0.06f * (float)sin(3.0 * phase);

            short sample = (short)(s * env * 21000.0f);
            out.push_back(sample);
        }
        // Seamless smooth 15ms silence between syllables
        int gap = (int)(0.015f * SAMPLE_RATE);
        for (int i = 0; i < gap; ++i) out.push_back(0);
    }

    WavBuffer generateFallbackChant() {
        std::vector<short> samples;
        appendDollNote(samples, 392.0f, 0.40f); // Mu
        appendDollNote(samples, 392.0f, 0.40f); // gung
        appendDollNote(samples, 440.0f, 0.48f); // hwa
        appendDollNote(samples, 392.0f, 0.42f); // kko
        appendDollNote(samples, 329.6f, 0.45f); // chi
        appendDollNote(samples, 392.0f, 0.35f); // pi
        appendDollNote(samples, 392.0f, 0.35f); // eot
        appendDollNote(samples, 440.0f, 0.40f); // seum
        appendDollNote(samples, 392.0f, 0.38f); // ni
        appendDollNote(samples, 329.6f, 0.85f, 0.04f, 0.25f); // da
        return createWav(samples);
    }

    WavBuffer generateGunshot() {
        std::vector<short> samples;
        float duration = 0.45f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float noise = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
            float crack = noise * expf(-t * 50.0f);
            float lowBoom = sinf(2.0f * PI * 85.0f * t) * expf(-t * 10.0f);
            float s = crack * 0.75f + lowBoom * 0.50f;
            if (s > 1.0f) s = 1.0f;
            if (s < -1.0f) s = -1.0f;
            samples.push_back((short)(s * 28000.0f));
        }
        return createWav(samples);
    }

    WavBuffer generateTimerBeep() {
        std::vector<short> samples;
        float duration = 0.06f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float env = 1.0f - (t / duration);
            float s = sinf(2.0f * PI * 1300.0f * t) * env;
            samples.push_back((short)(s * 16000.0f));
        }
        return createWav(samples);
    }

    WavBuffer generateHeadTurnClick() {
        std::vector<short> samples;
        float duration = 0.20f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            // Clean crisp mechanical doll rotation sound: dual click + gentle servo hum
            float click1 = expf(-t * 90.0f) * sinf(2.0f * PI * 1700.0f * t);
            float t2 = t - 0.07f;
            float click2 = (t2 > 0.0f) ? (expf(-t2 * 110.0f) * sinf(2.0f * PI * 2100.0f * t2)) : 0.0f;
            float servo = 0.12f * sinf(2.0f * PI * 320.0f * t) * (1.0f - t / duration);
            float s = click1 * 0.70f + click2 * 0.60f + servo;
            if (s > 1.0f) s = 1.0f;
            if (s < -1.0f) s = -1.0f;
            samples.push_back((short)(s * 22000.0f));
        }
        return createWav(samples);
    }

    WavBuffer generateEliminationSound() {
        std::vector<short> samples;
        float duration = 0.40f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float env = expf(-t * 7.0f);
            float s = (sinf(2.0f * PI * 380.0f * t) * 0.65f + sinf(2.0f * PI * 190.0f * t) * 0.35f) * env;
            samples.push_back((short)(s * 22000.0f));
        }
        return createWav(samples);
    }

    WavBuffer generateWinMusic() {
        std::vector<short> samples;
        float notes[] = { 523.25f, 659.25f, 783.99f, 1046.50f };
        for (int n = 0; n < 4; ++n) {
            float dur = (n == 3) ? 0.9f : 0.22f;
            int count = (int)(dur * SAMPLE_RATE);
            for (int i = 0; i < count; ++i) {
                float t = (float)i / SAMPLE_RATE;
                float env = 1.0f - (t / dur) * 0.6f;
                float s = (sinf(2.0f * PI * notes[n] * t) + 0.3f * sinf(2.0f * PI * notes[n] * 2.0f * t)) * env;
                samples.push_back((short)(s * 22000.0f));
            }
        }
        return createWav(samples);
    }

    WavBuffer generateGameOverMusic() {
        std::vector<short> samples;
        float notes[] = { 392.0f, 311.13f, 293.66f, 261.63f };
        for (int n = 0; n < 4; ++n) {
            float dur = (n == 3) ? 1.0f : 0.35f;
            int count = (int)(dur * SAMPLE_RATE);
            for (int i = 0; i < count; ++i) {
                float t = (float)i / SAMPLE_RATE;
                float env = expf(-t * 2.0f);
                float s = (sinf(2.0f * PI * notes[n] * t) + 0.4f * sinf(2.0f * PI * notes[n] * 0.5f * t)) * env;
                samples.push_back((short)(s * 24000.0f));
            }
        }
        return createWav(samples);
    }

    WavBuffer generateGlassShatter() {
        std::vector<short> samples;
        float duration = 1.15f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float env = expf(-t * 3.8f);
            float noise = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * expf(-t * 6.0f);
            
            // Sparkling crystalline resonant rings
            float ring1 = sinf(2.0f * PI * 3350.0f * t) * expf(-t * 4.2f);
            float ring2 = sinf(2.0f * PI * 4820.0f * t) * expf(-t * 5.0f);
            float ring3 = sinf(2.0f * PI * 6180.0f * t) * expf(-t * 7.5f);
            float lowCrack = sinf(2.0f * PI * 180.0f * t) * expf(-t * 14.0f);

            float s = noise * 0.45f + ring1 * 0.25f + ring2 * 0.20f + ring3 * 0.15f + lowCrack * 0.35f;
            s *= env;
            if (s > 1.0f) s = 1.0f;
            if (s < -1.0f) s = -1.0f;
            samples.push_back((short)(s * 28000.0f));
        }
        return createWav(samples);
    }

    WavBuffer generateGlassStep() {
        std::vector<short> samples;
        float duration = 0.18f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float env = expf(-t * 22.0f);
            float tap = sinf(2.0f * PI * 1380.0f * t) * 0.6f + sinf(2.0f * PI * 2750.0f * t) * 0.3f;
            float thud = sinf(2.0f * PI * 240.0f * t) * 0.5f;
            float s = (tap + thud) * env;
            samples.push_back((short)(s * 25000.0f));
        }
        return createWav(samples);
    }

    WavBuffer generateFallWhoosh() {
        std::vector<short> samples;
        float duration = 1.35f;
        int count = (int)(duration * SAMPLE_RATE);
        for (int i = 0; i < count; ++i) {
            float t = (float)i / SAMPLE_RATE;
            float env = (t < 0.2f) ? (t / 0.2f) : expf(-(t - 0.2f) * 2.2f);
            float freq = 850.0f * expf(-t * 1.6f) + 140.0f;
            float scream = sinf(2.0f * PI * freq * t) + 0.35f * sinf(2.0f * PI * freq * 1.5f * t);
            float wind = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * 0.3f;
            float s = (scream * 0.7f + wind) * env;
            if (s > 1.0f) s = 1.0f;
            if (s < -1.0f) s = -1.0f;
            samples.push_back((short)(s * 25000.0f));
        }
        return createWav(samples);
    }

    void loadMP3() {
        char exePath[MAX_PATH] = {0};
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        char exeDir[MAX_PATH] = {0};
        strcpy(exeDir, exePath);
        char* lastSlash = strrchr(exeDir, '\\');
        if (!lastSlash) lastSlash = strrchr(exeDir, '/');
        if (lastSlash) *(lastSlash + 1) = '\0';

        char testPath1[MAX_PATH], testPath2[MAX_PATH];
        sprintf(testPath1, "%ssquid_song.mp3", exeDir);
        sprintf(testPath2, "%s..\\bin\\squid_song.mp3", exeDir);

        const char* candidatePaths[] = {
            testPath1,
            testPath2,
            "squid_song.mp3",
            "bin\\squid_song.mp3",
            "bin/squid_song.mp3",
            "src\\bgmusic\\squid_song.mp3",
            "src/bgmusic/squid_song.mp3"
        };

        char fullPath[MAX_PATH] = {0};
        bool fileExists = false;

        for (int i = 0; i < 7; ++i) {
            if (GetFullPathNameA(candidatePaths[i], MAX_PATH, fullPath, NULL) > 0) {
                FILE* f = fopen(fullPath, "rb");
                if (f) {
                    fclose(f);
                    fileExists = true;
                    strcpy(foundMp3Path, fullPath);
                    break;
                }
            }
        }

        // Search directory if not found in list
        if (!fileExists) {
            WIN32_FIND_DATAA fd;
            HANDLE hFind = FindFirstFileA("src\\bgmusic\\*.mp3", &fd);
            if (hFind != INVALID_HANDLE_VALUE) {
                char relPath[MAX_PATH];
                sprintf(relPath, "src\\bgmusic\\%s", fd.cFileName);
                GetFullPathNameA(relPath, MAX_PATH, foundMp3Path, NULL);
                fileExists = true;
                FindClose(hFind);
            }
        }

        if (fileExists) {
            mciSendStringA("close squid_song", NULL, 0, NULL);

            char shortPath[MAX_PATH] = {0};
            if (GetShortPathNameA(foundMp3Path, shortPath, MAX_PATH) > 0) {
                strcpy(foundMp3Path, shortPath);
            }

            char cmd[512];
            sprintf(cmd, "open \"%s\" type mpegvideo alias squid_song", foundMp3Path);
            MCIERROR err = mciSendStringA(cmd, NULL, 0, NULL);
            if (err != 0) {
                sprintf(cmd, "open \"%s\" alias squid_song", foundMp3Path);
                err = mciSendStringA(cmd, NULL, 0, NULL);
            }

            if (err == 0) {
                mp3Loaded = true;
            }
        }
    }
}

void AudioManager::init() {
    chantNormal      = generateFallbackChant();
    gunshotSound     = generateGunshot();
    timerBeepSound   = generateTimerBeep();
    headTurnSound    = generateHeadTurnClick();
    eliminationSound = generateEliminationSound();
    winSound         = generateWinMusic();
    gameOverSound    = generateGameOverMusic();
    glassShatterSound= generateGlassShatter();
    glassStepSound   = generateGlassStep();
    fallWhooshSound  = generateFallWhoosh();

    loadMP3();
}

void AudioManager::cleanup() {
    stopAll();
    if (mp3Loaded) {
        mciSendStringA("close squid_song", NULL, 0, NULL);
    }
}

void AudioManager::playSong() {
    if (mp3Loaded) {
        mciSendStringA("play squid_song from 0", NULL, 0, NULL);
    } else {
        PlaySoundA((LPCSTR)chantNormal.data.data(), NULL, SND_MEMORY | SND_ASYNC);
    }
}

void AudioManager::pauseSong() {
    if (mp3Loaded) {
        mciSendStringA("pause squid_song", NULL, 0, NULL);
    } else {
        PlaySoundA(NULL, NULL, 0);
    }
}

void AudioManager::stopSong() {
    if (mp3Loaded) {
        mciSendStringA("stop squid_song", NULL, 0, NULL);
    } else {
        PlaySoundA(NULL, NULL, 0);
    }
}

void AudioManager::restartSong() {
    if (mp3Loaded) {
        mciSendStringA("stop squid_song", NULL, 0, NULL);
        mciSendStringA("play squid_song from 0", NULL, 0, NULL);
    } else {
        PlaySoundA((LPCSTR)chantNormal.data.data(), NULL, SND_MEMORY | SND_ASYNC);
    }
}

void AudioManager::playChant(float tempoMultiplier) {
    restartSong();
}

void AudioManager::stopChant() {
    stopSong();
}

void AudioManager::playGunshot() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)gunshotSound.data.data(), NULL, flags);
}

void AudioManager::playTimerBeep() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)timerBeepSound.data.data(), NULL, flags);
}

void AudioManager::playHeadTurnClick() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)headTurnSound.data.data(), NULL, flags);
}

void AudioManager::playEliminationSound() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)eliminationSound.data.data(), NULL, flags);
}

void AudioManager::playWinMusic() {
    stopSong();
    PlaySoundA((LPCSTR)winSound.data.data(), NULL, SND_MEMORY | SND_ASYNC);
}

void AudioManager::playGameOverMusic() {
    stopSong();
    PlaySoundA((LPCSTR)gameOverSound.data.data(), NULL, SND_MEMORY | SND_ASYNC);
}

void AudioManager::playGlassShatter() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)glassShatterSound.data.data(), NULL, flags);
}

void AudioManager::playGlassStep() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)glassStepSound.data.data(), NULL, flags);
}

void AudioManager::playFallWhoosh() {
    DWORD flags = SND_MEMORY | SND_ASYNC | (mp3Loaded ? 0 : SND_NOSTOP);
    PlaySoundA((LPCSTR)fallWhooshSound.data.data(), NULL, flags);
}

void AudioManager::stopAll() {
    stopSong();
    PlaySoundA(NULL, NULL, 0);
}
