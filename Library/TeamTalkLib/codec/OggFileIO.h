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

#ifndef OGGOUTPUT_H
#define OGGOUTPUT_H

#include "myace/MyACE.h"
#include "mystd/MyStd.h"

#include <ogg/ogg.h>

#include <ace/SString.h>
#include <cstdint>

#include <memory>
#include <vector>

class OggOutput : private NonCopyable
{
public:
    OggOutput();
    ~OggOutput();
    bool Open(int stream_id);
    void Close();

    int PutPacket(const ogg_packet& op);
    int GetPageOut(ogg_page& og);
    int FlushPageOut(ogg_page& og);

protected:
    ogg_stream_state m_os;
};

class OggInput : private NonCopyable
{
public:
    OggInput();
    ~OggInput();

    bool Open(const ogg_page& og);
    void Close();
    void Reset();

    int PutPage(ogg_page& og);
    int GetPacket(ogg_packet& op, bool peek = false);

private:
    ogg_stream_state m_os;
    bool m_ready;
};

class OggFile : private NonCopyable
{
public:
    OggFile() = default;
    ~OggFile();
    
    // write
    bool NewFile(const ACE_TString& filename);
    // read only
    bool Open(const ACE_TString& filename);
    
    void Close();

    int ReadOggPage(ogg_page& og);
    int WriteOggPage(const ogg_page& og);

    bool Seek(ogg_int64_t granulepos, ogg_page& og);
    ogg_int64_t LastGranulePos();
    ogg_int64_t CurrentGranulePos() const;

    bool SeekLog2(ogg_int64_t granulepos, ogg_page& og);
    ogg_int64_t LastGranulePosLog2();

private:
    bool SyncPage();
    MyFile m_file;
    ogg_sync_state m_state = {};
    ogg_int64_t m_last_gp = -1;
};

#if defined(ENABLE_SPEEX)

#include <speex/speex_header.h>

class SpeexOgg : private NonCopyable
{
public:
    SpeexOgg();
    ~SpeexOgg();

    bool Open(const SpeexHeader& spx_header,
              spx_int32_t lookahead);
    void Close();

    int PutEncoded(const char* enc_data, int len, 
                   unsigned short packetno,
                   unsigned int timestamp,
                   bool last);

    int GetPageOut(ogg_page& og);
    int FlushPageOut(ogg_page& og);

private:
    void Reset();
    OggOutput m_ogg;
    spx_int32_t m_lookahead;
    ogg_int64_t m_counter;
    int m_frame_size;
    int m_msec_per_packet;
    unsigned short m_last_packetno;
    unsigned int m_last_timestamp;
};


class SpeexFile : private NonCopyable
{
public:
    SpeexFile();
    ~SpeexFile();
    bool Open(const ACE_TString& filename,
              int bandmode, bool vbr);

    void Close();

    int WriteEncoded(const char* enc_data, int len, 
                     bool last);

private:
    SpeexOgg m_speex;
    OggFile m_ogg;
    SpeexHeader m_spx_header;
    bool m_initial_packet;
    unsigned short m_packet_no;
    unsigned int m_timestamp;
    ogg_page m_aud_page;
};

using speexfile_t = std::shared_ptr< SpeexFile >;

#include "SpeexEncoder.h"

class SpeexEncFile : private NonCopyable
{
public:
    SpeexEncFile() = default;

    bool Open(const ACE_TString& filename,
              int bandmode, int complexity, float vbr_quality,
              int bitrate, int maxbitrate, bool dtx);
    void Close();
    int Encode(const short* samples, bool last=false);

private:
    SpeexEncoder m_encoder;
    SpeexFile m_file;
    std::vector<char> m_buffer;
};

using speexencfile_t = std::shared_ptr< SpeexEncFile >;

#endif /* ENABLE_SPEEX */

#if defined(ENABLE_OPUSTOOLS)

extern "C" {
#include <opus_header.h>
}

class OpusFile : private NonCopyable
{
public:
    OpusFile();
    ~OpusFile();

    bool NewFile(const ACE_TString& filename,
                int channels, int samplerate, int framesize);
    bool OpenFile(const ACE_TString& filename);
    void Close();

    int GetSampleRate() const;
    int GetChannels() const;

    int WriteEncoded(const char* enc_data, int enc_len, int framesize, bool last);

    const unsigned char* ReadEncoded(int& bytes, ogg_int64_t* sampleduration = nullptr);

    bool Seek(ogg_int64_t samplesoffset);

    ogg_int64_t GetTotalSamples() const;
    ogg_int64_t GetSamplesPosition() const;

private:
    OggInput m_oggin;
    OggOutput m_oggout;
    OggFile m_oggfile;
    OpusHeader m_header = {};

    // for encoding
    ogg_int64_t m_granule_pos, m_packet_no;
    // for decoding
    ogg_int64_t m_last_granule_pos;
};

using opusfile_t = std::shared_ptr< OpusFile >;

#endif /* ENABLE_OPUSTOOLS */

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)

#include "OpusEncoder.h"

class OpusEncFile : private NonCopyable
{
public:
    OpusEncFile() = default;

    bool Open(const ACE_TString& filename, int channels, 
              int samplerate, int framesize, int app);
    void Close();
    int Encode(const short* input_buffer, int input_samples, bool last);

    OpusEncode& GetEncoder() { return m_encoder; }

private:
    OpusEncode m_encoder;
    OpusFile m_file;
    std::vector<char> m_buffer;
};

using opusencfile_t = std::shared_ptr< OpusEncFile >;

#include "OpusDecoder.h"

class OpusDecFile : private NonCopyable
{
public:
    bool Open(const ACE_TString& filename);
    void Close();

    int GetSampleRate() const;
    int GetChannels() const;
    int Decode(short* input_buffer, int input_samples);
    bool Seek(uint32_t offset_msec);
    uint32_t GetDurationMSec();
    uint32_t GetElapsedMSec() const;
private:
    OpusDecode m_decoder;
    OpusFile m_file;
    ogg_int64_t m_samples_decoded = 0;
};

using opusdecfile_t = std::shared_ptr< OpusDecFile >;

#endif


#endif
