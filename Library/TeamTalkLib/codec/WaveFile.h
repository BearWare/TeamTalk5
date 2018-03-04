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

#include <ace/FILE_IO.h>
#include <ace/SString.h>
#include <ace/Bound_Ptr.h>
#include <ace/Null_Mutex.h>

class WaveFile
{
public:
    WaveFile();
    ~WaveFile();

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
    bool WriteHeaderLength();
    ACE_FILE_IO m_wavfile;
    ACE_TString m_filepath;
    int m_channels;
};

typedef ACE_Strong_Bound_Ptr< WaveFile, ACE_Null_Mutex > wavefile_t;

#endif
