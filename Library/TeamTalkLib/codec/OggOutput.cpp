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

#include "OggOutput.h"
#include <assert.h>
#include <myace/MyACE.h>

OggOutput::OggOutput()
    : m_os()
{
}

OggOutput::~OggOutput()
{
    Close();
}

bool OggOutput::Open(int stream_id)
{
    Close();
    return ogg_stream_init(&m_os, stream_id)>=0;
}

void OggOutput::Close()
{
    ogg_stream_clear(&m_os);
}

int OggOutput::PutPacket(const ogg_packet& op)
{
    return ogg_stream_packetin(&m_os, const_cast<ogg_packet*>(&op));
}

int OggOutput::GetPageOut(ogg_page& og)
{
    return ogg_stream_pageout(&m_os,&og);
}

int OggOutput::FlushPageOut(ogg_page& og)
{
    return ogg_stream_flush(&m_os,&og);
}

OggInput::OggInput()
    : m_os()
{
    Close();
}

OggInput::~OggInput()
{
    Close();
}

bool OggInput::Open(const ogg_page& og)
{
    return ogg_stream_init(&m_os, ogg_page_serialno(&og)) >= 0;
}

void OggInput::Close()
{
    ogg_stream_clear(&m_os);
    m_os = {};
    m_ready = false;
}

int OggInput::PutPage(ogg_page& og)
{
    assert(ogg_stream_check(&m_os) == 0);

    if (ogg_page_serialno(&og) != m_os.serialno)
        ogg_stream_reset_serialno(&m_os, ogg_page_serialno(&og));

    return ogg_stream_pagein(&m_os, &og);
}

int OggInput::GetPacket(ogg_packet& op)
{
    return ogg_stream_packetout(&m_os, &op);
}


OggFile::OggFile()
{
}

OggFile::~OggFile()
{
    Close();
}

bool OggFile::NewFile(const ACE_TString& filename)
{
    ACE_FILE_Connector con;
    if (con.connect(m_file, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC) < 0)
        return false;

    return true;
}

bool OggFile::Open(const ACE_TString& filename)
{
    ACE_FILE_Connector con;
    if (con.connect(m_file, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDONLY) < 0)
    {
        return false;
    }
    
    return ogg_sync_init(&m_state) == 0;
}

void OggFile::Close()
{
    m_file.close();

    if (ogg_sync_check(&m_state) == 0)
    {
        ogg_sync_clear(&m_state);
    }
}

int OggFile::ReadOggPage(ogg_page& og)
{
    assert(m_file.get_handle() != ACE_INVALID_HANDLE);
    assert(ogg_sync_check(&m_state) == 0);
    
    int pages;
    while ((pages = ogg_sync_pageout(&m_state, &og)) != 1)
    {
        auto SIZE = 0x1000;
        auto buffer = ogg_sync_buffer(&m_state, SIZE);
        auto ret = m_file.recv(buffer, SIZE);
        if (ret > 0)
        {
            ret = ogg_sync_wrote(&m_state, ret);
            assert(ret == 0);
        }
        else break;
    }
    
    return pages;
}

int OggFile::WriteOggPage(const ogg_page& og)
{
    assert(m_file.get_handle() != ACE_INVALID_HANDLE);

    auto bytes_out = m_file.send(og.header, og.header_len);
    if (bytes_out > 0)
    {
        auto ret = m_file.send(og.body, og.body_len);
        if (ret > 0)
            bytes_out += ret;
        else
            bytes_out = -1;
    }

    return int(bytes_out);
}

#if defined(ENABLE_SPEEX)

#include "SpeexDecoder.h"
#include "SpeexEncoder.h"

SpeexOgg::SpeexOgg()
{
    Reset();
}

SpeexOgg::~SpeexOgg()
{
    Close();
}

void SpeexOgg::Reset()
{
    m_ogg.Close();
    m_lookahead = 0;
    m_counter = 0;
    m_frame_size = 0;
    m_last_packetno = 0;
    m_msec_per_packet = 0;
    m_last_timestamp = 0;
}

bool SpeexOgg::Open(const SpeexHeader& spx_header,
                    spx_int32_t lookahead)
{
    ogg_packet op;
    const char comment[]= {4, 0, 0, 0, 'B', 'E', 'A', 'R', 0, 0, 0, 0};
    int packet_size, ret;

    if(!m_ogg.Open('S'))
        goto error;

    op.packet = (unsigned char *)speex_header_to_packet(const_cast<SpeexHeader*>(&spx_header), 
                                                        &packet_size);
    op.bytes = packet_size;
    op.b_o_s = 1;
    op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 0;
    ret = m_ogg.PutPacket(op);
    assert(ret>=0);
    speex_header_free(op.packet);
    if(ret < 0)
        goto error;

    op.packet = (unsigned char*)comment;
    op.bytes = sizeof(comment);
    op.b_o_s = 0;
    op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 0;
    ret = m_ogg.PutPacket(op);
    assert(ret>=0);
    if(ret < 0)
        goto error;

    m_msec_per_packet = (spx_header.frame_size * 1000) / spx_header.rate;
    m_msec_per_packet *= spx_header.frames_per_packet;
    if(!m_msec_per_packet)
        goto error;

    

    m_lookahead = lookahead;
    m_counter = 0;
    m_frame_size = spx_header.frame_size;

    return true;

error:
    Close();
    return false;
}

void SpeexOgg::Close()
{
    Reset();
}

int SpeexOgg::PutEncoded(const char* enc_data, int len, 
                         unsigned short packetno, 
                         unsigned int timestamp, bool last)
{
    if(m_counter != 0)
    {
        //insert silence (if any)
        m_counter += speex_packet_jump(m_msec_per_packet,
                                       m_last_timestamp,
                                       timestamp);

        short diff = (short)packetno - (short)m_last_packetno;
        if(diff>0)
            m_counter += diff;
    }
    else
        m_counter++;

    m_last_timestamp = timestamp;
    m_last_packetno = packetno;

    ogg_int64_t total_samples = (m_counter) * m_frame_size + m_frame_size;

    ogg_packet op;
    op.packet = (unsigned char*)enc_data;
    op.bytes = len;
    op.b_o_s = 0;
    op.e_o_s = (long)last;
    //do not modify unless also changing 'speex_granule_time(..)'
    op.granulepos = (m_counter)*m_frame_size - m_lookahead;
    if(op.granulepos > total_samples)
        op.granulepos = total_samples;

    op.packetno = m_counter+1;

    return m_ogg.PutPacket(op);
}

double speex_granule_time(const SpeexHeader& spx_header,
                          spx_int32_t lookahead, ogg_int64_t granpos)
{
    if(spx_header.rate == 0)
        return 0.0;
    double total_samples = (double)granpos + lookahead;
    return total_samples / (double)spx_header.rate;
}

int speex_packet_jump(int msec_per_packet,
                      unsigned int last_timestamp,
                      unsigned int cur_timestamp)
{
    int time_diff = (int)cur_timestamp - (int)last_timestamp;
    int packet_diff = time_diff / msec_per_packet;
    if(time_diff >= 250)
    {
        MYTRACE(ACE_TEXT("Time diff skipped: %u\n"), time_diff);
        return packet_diff;
    }
    return 0;
}

int SpeexOgg::GetPageOut(ogg_page& og)
{
    return m_ogg.GetPageOut(og);
}

int SpeexOgg::FlushPageOut(ogg_page& og)
{
    return m_ogg.FlushPageOut(og);
}



SpeexFile::SpeexFile()
: m_initial_packet(true)
{
    Close();
}

SpeexFile::~SpeexFile()
{
    Close();
}

bool SpeexFile::Open(const ACE_TString& filename,
                     int bandmode,
                     bool vbr)
{
    switch(bandmode)
    {
    case SPEEX_MODEID_NB :
        speex_init_header(&m_spx_header, 8000, 1, 
            speex_lib_get_mode(SPEEX_MODEID_NB));
        break;
    case SPEEX_MODEID_WB :
        speex_init_header(&m_spx_header, 16000, 1, 
            speex_lib_get_mode(SPEEX_MODEID_WB));
        break;
    case SPEEX_MODEID_UWB :
        speex_init_header(&m_spx_header, 32000, 1, 
            speex_lib_get_mode(SPEEX_MODEID_UWB));
        break;
    default:
        assert(0);
        return false;
    }

    m_spx_header.frames_per_packet = 1;
    m_spx_header.nb_channels = 1;
    m_spx_header.vbr = vbr;

    SpeexEncoder enc;
    if(!enc.Initialize(bandmode, 2, 5))
    {
        Close();
        return false;
    }

    spx_int32_t lookahead = 0;
    enc.GetOption(SPEEX_GET_LOOKAHEAD, lookahead);

    if(!m_speex.Open(m_spx_header, lookahead))
    {
        Close();
        return false;
    }
    if (!m_ogg.NewFile(filename))
    {
        Close();
        return false;
    }

    //write remaining audio headers
    int ret;
    while(m_speex.FlushPageOut(m_aud_page)>0)
    {
        ret = m_ogg.WriteOggPage(m_aud_page);
        assert(ret>=0);
    }

    return true;
}

void SpeexFile::Close()
{
    // don't flush if we never wrote anything
    if (!m_initial_packet)
    {
        // write remaining audio
        int ret;
        while(m_speex.FlushPageOut(m_aud_page)>0)
        {
            ret = m_ogg.WriteOggPage(m_aud_page);
            assert(ret>=0);
        }
    }

    m_speex.Close();
    m_ogg.Close();
    m_initial_packet = true;
    m_timestamp = m_packet_no = 0;
}

int SpeexFile::WriteEncoded(const char* enc_data, int len, 
                            bool last)
{
    m_speex.PutEncoded(enc_data, len, m_packet_no++, m_timestamp, last);
    m_timestamp += 20;

    int ret;
    if(m_initial_packet)
    {
        while((ret = m_speex.FlushPageOut(m_aud_page))>0)
        {
            ret = m_ogg.WriteOggPage(m_aud_page);
            assert(ret>=0);
        }
        m_initial_packet = false;
    }
    else
    {
        while((ret = m_speex.GetPageOut(m_aud_page))>0)
        {
            ret = m_ogg.WriteOggPage(m_aud_page);
            assert(ret>=0);
        }
    }
    return ret;
}

SpeexEncFile::SpeexEncFile()
{
}

bool SpeexEncFile::Open(const ACE_TString& filename,
                        int bandmode, int complexity, float vbr_quality,
                        int bitrate, int maxbitrate, bool dtx)
{
    if(!m_encoder.Initialize(bandmode, complexity, vbr_quality, bitrate, maxbitrate, dtx))
        return false;
    if(!m_file.Open(filename, bandmode, bitrate || maxbitrate))
    {
        Close();
        return false;
    }
    m_buffer.resize(0xFFF);
    return true;
}

void SpeexEncFile::Close()
{
    m_encoder.Close();
    m_file.Close();
}

int SpeexEncFile::Encode(const short* samples, bool last/*=false*/)
{
    int ret = m_encoder.Encode(samples, &m_buffer[0], int(m_buffer.size()));
    if(ret>0)
        return m_file.WriteEncoded(&m_buffer[0], ret, last);
    return 0;
}

#endif /* ENABLE_SPEEX */

#if defined(ENABLE_OPUSTOOLS)

OpusFile::OpusFile()
{
    Close();
}

OpusFile::~OpusFile()
{
    Close();
}

bool OpusFile::NewFile(const ACE_TString& filename,
                       int channels, int samplerate,
                       int framesize)
{
    // Make the stream (ogg stream 'serialno') as unique as possible on the same machine by using the timestamp as id.
    // Facilitates the subsequent handling of the recording with tooling like oggz-merge to mux the recordings.
    // If we use the same stream for every recording then muxing of recording always involves rewritting the id.
    if (!m_oggout.Open(GETTIMESTAMP()) || !m_oggfile.NewFile(filename))
    {
        Close();
        return false;
    }

    m_frame_size = framesize;

    m_header.preskip = 3840;  // 80 ms @ 48kHz as per rfc7845 - 5.1 bullet 4
    m_header.channels = channels;
    m_header.channel_mapping = 0;
    m_header.input_sample_rate = samplerate;
    m_header.gain = 0;
    m_header.nb_streams = 1;
    m_header.nb_coupled = channels == 2? 1 : 0;

    unsigned char header_data[276];
    int packet_size = opus_header_to_packet(&m_header, header_data, sizeof(header_data));
    ogg_packet op = {};
    op.packet = header_data;
    op.bytes = packet_size;
    op.b_o_s = 1;
    op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 0;

    m_oggout.PutPacket(op);

    unsigned char comment_data[] = {'O', 'p', 'u', 's', 'T', 'a', 'g', 's',
                                    /* Vendor String Length */
                                    0x0, 0x0, 0x0, 0x0,
                                    /* User Comment List Length */
                                    0x0, 0x0, 0x0, 0x0};
    op = {};
    op.packet = comment_data;
    op.bytes = sizeof(comment_data);
    op.b_o_s = op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 1;

    m_oggout.PutPacket(op);

    ogg_page og;
    int ret;
    while((ret = m_oggout.FlushPageOut(og))>0)
    {
        m_oggfile.WriteOggPage(og);
    }
    m_packet_no = 2;
    return ret >= 0;
}

bool OpusFile::OpenFile(const ACE_TString& filename)
{
    if (!m_oggfile.Open(filename))
        return false;

    ogg_page og;
    if (m_oggfile.ReadOggPage(og) != 1 || !m_oggin.Open(og) || m_oggin.PutPage(og) != 0)
    {
        Close();
        return false;
    }

    int ret;
    m_packet_no = -1;
    ogg_packet op = {};
    while (true)
    {
        if (m_oggin.GetPacket(op) == 1)
        {
            if (op.b_o_s && op.bytes >= 8 && memcmp(op.packet, "OpusHead", 8) == 0 &&
                opus_header_parse(op.packet, op.bytes, &m_header) == 1)
            {
                // found opus header
                m_packet_no = 0;
            }
            else if (m_packet_no == 0)
            {
                // skip comments
                ++m_packet_no;
                break;
            }
        }

        ret = m_oggfile.ReadOggPage(og);
        if (ret > 0)
        {
            ret = m_oggin.PutPage(og);
            assert(ret == 0);
        }
        else if (ret == 0)
            break; // no page - eof
        else if (ret < 0)
            break; // file read error
    }

    // packet number 2 is where the first encoded OPUS data exists
    if (m_packet_no == 1)
        return true;

    Close();

    return false;
}

void OpusFile::Close()
{
    m_oggfile.Close();
    m_oggout.Close();
    m_oggin.Close();

    m_frame_size = 0;
    m_granule_pos = m_packet_no = 0;
    m_header = {};
}

int OpusFile::GetSampleRate() const
{
    return m_header.input_sample_rate;
}

int OpusFile::GetChannels() const
{
    return m_header.channels;
}

int OpusFile::WriteEncoded(const char* enc_data, int enc_len, bool last)
{
    ogg_packet op = {};
    op.packet = reinterpret_cast<unsigned char*>(const_cast<char*>(enc_data));
    op.bytes = enc_len;
    op.b_o_s = 0;
    op.e_o_s = last? 1 : 0;
    // As per rfc7845, the granule on the first data page must already
    // include the samples in the page that's written.
    // So, advance the granule before writing.
    // https://tools.ietf.org/html/rfc7845#page-6
    m_granule_pos += m_frame_size * 48000 / m_header.input_sample_rate;
    op.granulepos = m_granule_pos;
    op.packetno = m_packet_no++;

    m_oggout.PutPacket(op);

    ogg_page og;
    int ret;
    while((ret = m_oggout.FlushPageOut(og)) > 0)
    {
        ret = m_oggfile.WriteOggPage(og);
        assert(ret >= 0);
    }
    assert(ret >= 0);

    return ret;
}

const unsigned char* OpusFile::ReadEncoded(int& bytes, ogg_int64_t* sampleoffset /*= nullptr*/)
{
    ogg_packet op;
    ogg_page og;

    while (m_oggin.GetPacket(op) != 1)
    {
        int ret = m_oggfile.ReadOggPage(og);
        if (ret > 0)
        {
            ret = m_oggin.PutPage(og);
            assert(ret == 0);
        }
        else if (ret == 0)
            return nullptr; // no page - eof
        else if (ret < 0)
            return nullptr; // file read error
    }

    ++m_packet_no;
    bytes = op.bytes;
    return op.packet;
}

#endif /* ENABLE_OPUSTOOLS */

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)

OpusEncFile::OpusEncFile()
{
}

bool OpusEncFile::Open(const ACE_TString& filename, int channels, 
                       int samplerate, int framesize, int app)
{
    if(!m_encoder.Open(samplerate, channels, app))
        return false;

    if (!m_file.NewFile(filename, channels, samplerate, framesize))
    {
        Close();
        return false;
    }
    m_buffer.resize(0xFFF);
    return true;
}

void OpusEncFile::Close()
{
    m_file.Close();
    m_encoder.Close();
}

int OpusEncFile::Encode(const short* input_buffer, int input_samples,
                        bool last)
{
    int ret = m_encoder.Encode(input_buffer, input_samples,
                               &m_buffer[0], int(m_buffer.size()));
    if(ret > 0)
        return m_file.WriteEncoded(&m_buffer[0], ret, last);

    return 0;
}

#endif
