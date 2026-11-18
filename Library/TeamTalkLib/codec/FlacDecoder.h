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

#ifndef FLACDECODER_H
#define FLACDECODER_H

#include "mystd/MyStd.h"

#include <FLAC/stream_decoder.h>

#include <cstddef>

class FlacDecode : private NonCopyable
{
public:
    FlacDecode() = default;
    ~FlacDecode();

    bool Open(int sample_rate, int channels);
    void Close();

    int Decode(const char* input_buffer, int input_bufsize,
               short* output_buffer, int output_samples);

private:
    static FLAC__StreamDecoderReadStatus ReadCallback(const FLAC__StreamDecoder* decoder,
                                                      FLAC__byte buffer[], size_t* bytes,
                                                      void* client_data);
    static FLAC__StreamDecoderWriteStatus WriteCallback(const FLAC__StreamDecoder* decoder,
                                                        const FLAC__Frame* frame,
                                                        const FLAC__int32* const buffer[],
                                                        void* client_data);
    static void ErrorCallback(const FLAC__StreamDecoder* decoder,
                              FLAC__StreamDecoderErrorStatus status,
                              void* client_data);

    FLAC__StreamDecoder* m_decoder = nullptr;
    int m_samplerate = 0;
    int m_channels = 0;
    const char* m_input = nullptr;
    int m_input_size = 0;
    int m_input_pos = 0;
    short* m_output = nullptr;
    int m_output_samples = 0;
    int m_decoded_samples = 0;
    bool m_error = false;
};

#endif
