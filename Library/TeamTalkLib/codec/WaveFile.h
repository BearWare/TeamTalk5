/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if !defined(WAVEFILE_H)
#define WAVEFILE_H

#include <memory>

#include <ace/FILE_IO.h>
#include <ace/SString.h>

#include "MediaUtil.h"

#if defined(WIN32)
#include <Mmreg.h>
#else
struct WAVEFORMATEX
{
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
    uint16_t cbSize;
} __attribute__((packed));
#endif

#define SIZEOF_WAVEFORMATEX 16

bool WriteWaveFileHeader(ACE_FILE_IO& file, const media::AudioFormat& fmt);
bool WriteWaveFileHeader(ACE_FILE_IO& file, const WAVEFORMATEX* waveformat, int len);
bool UpdateWaveFileHeader(ACE_FILE_IO& file);

class WaveFile
{
public:
    WaveFile(const WaveFile&) = delete;
    WaveFile();
    virtual ~WaveFile();

    bool NewFile(const ACE_TString& filename, const WAVEFORMATEX* waveformat, int len);
    bool AppendData(const void* data, int len);
private:
    ACE_FILE_IO m_wavfile;
};

typedef std::shared_ptr< WaveFile > wavefile_t;

class WavePCMFile
{
public:
    WavePCMFile(const WavePCMFile&) = delete;
    WavePCMFile();
    ~WavePCMFile();

    bool NewFile(const ACE_TString& filename, int samplerate, int channels);
    bool OpenFile(const ACE_TString& filename, bool readonly);
    void Close();
    bool AppendSamples(const short* buffer, int samples_len);
    const ACE_TString& Path() const { return m_filepath; }
    const ACE_TString FileName() const; 

    bool SeekSamplesBegin();
    bool SeekSamplesEnd();
    int ReadSamples(short* buffer, int buffer_len);
    int GetSampleRate();
    int GetSamplesCount();
    int GetChannels();

private:
    bool Valid();
    int WriteData(const void* data, int len);
    bool WriteHeader(int samplerate, int channels);
    ACE_FILE_IO m_wavfile;
    ACE_TString m_filepath;
    int m_channels;
};

typedef std::shared_ptr< WavePCMFile > wavepcmfile_t;

#endif
