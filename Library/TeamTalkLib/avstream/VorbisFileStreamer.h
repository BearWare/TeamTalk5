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

#ifndef VORBISFILESTREAMER_H
#define VORBISFILESTREAMER_H

#include "MediaStreamer.h"
#include "AudioResampler.h"

#include "codec/OggFileIO.h"

#include <ace/SString.h>

bool GetVorbisFileMediaFileProp(const ACE_TString& filename, MediaFileProp& mfp);

class VorbisFileStreamer : public MediaFileStreamer
{
public:
    VorbisFileStreamer(const ACE_TString& filename, const MediaStreamOutput& out_prop);
    ~VorbisFileStreamer() override;

protected:
    void Run() override;

private:
    VorbisDecFile m_decoder;
    audio_resampler_t m_resampler;
};

#endif // VORBISFILESTREAMER_H
