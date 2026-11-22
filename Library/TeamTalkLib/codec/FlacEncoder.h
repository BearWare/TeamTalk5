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

#ifndef FLACENCODER_H
#define FLACENCODER_H

#include "mystd/MyStd.h"

#include <FLAC/stream_encoder.h>

#include <cstddef>
#include <cstdint>
#include <vector>

class FlacEncode : private NonCopyable
{
public:
    FlacEncode() = default;

    bool Open(int sample_rate, int channels, int compression_level, int frame_size);
    void Close();

    int Encode(const short* input_buffer, int input_samples,
               char* output_buffer, int output_bufsize);

private:
    FLAC__StreamEncoder* CreateEncoder();
    static FLAC__StreamEncoderWriteStatus WriteCallback(const FLAC__StreamEncoder* encoder,
                                                        const FLAC__byte buffer[], size_t bytes,
                                                        uint32_t samples, uint32_t current_frame,
                                                        void* client_data);

    bool m_open = false;
    int m_samplerate = 0;
    int m_channels = 0;
    int m_compressionlevel = 0;
    int m_framesize = 0;
    std::vector<FLAC__int32> m_input;
    char* m_output = nullptr;
    int m_output_size = 0;
    int m_output_length = 0;
    bool m_output_overflow = false;
};

#endif
