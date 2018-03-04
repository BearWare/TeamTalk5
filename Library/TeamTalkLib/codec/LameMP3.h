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

#ifndef LAMEMP3_H
#define LAMEMP3_H

#include <ace/OS.h>

#include "BladeMP3EncDLL.h"

#include <ace/SString.h>
#include <ace/FILE_IO.h>
#include <ace/Bound_Ptr.h>
#include <ace/Null_Mutex.h>

#include <vector>

class LameMP3
{
public:
    LameMP3();
    ~LameMP3();
    bool NewFile(const ACE_TString& filename, int samplerate, 
                 int channels, int bitrate);
    void Close();
    ACE_TString GetFileName() const;

    int Encode(const short* samples, int samples_count);

    static bool CanLoad();

private:
    ACE_FILE_IO m_outfile;
    HBE_STREAM m_hMp3Stream;
    DWORD m_dwInSamples, m_dwChannels;
    std::vector<BYTE> m_out_mp3data;
};

typedef ACE_Strong_Bound_Ptr< LameMP3, ACE_Null_Mutex > lame_mp3file_t;

#endif

