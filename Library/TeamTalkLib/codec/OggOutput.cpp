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

OggFile::~OggFile()
{
    Close();
}

bool OggFile::Open(const ACE_TString& filename)
{
    ACE_FILE_Connector con;
    if(con.connect(m_out_file, ACE_FILE_Addr(filename.c_str()),
        0, ACE_Addr::sap_any, 0, O_RDWR | O_CREAT | O_TRUNC) < 0)
        return false;

    return true;
}

void OggFile::Close()
{
    m_out_file.close();
}

int OggFile::WriteOggPage(const ogg_page& og)
{
    assert(m_out_file.get_handle() != ACE_INVALID_HANDLE);

    ssize_t bytes_out = -1;
    bytes_out = m_out_file.send(og.header, og.header_len);
    if(bytes_out>0)
        bytes_out += m_out_file.send(og.body, og.body_len);

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
    if(!m_ogg.Open(filename))
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

extern "C" {
#include <opus_header.h>
}

OpusFile::OpusFile()
{
    Close();
}

bool OpusFile::Open(const ACE_TString& filename,
                    int channels, int samplerate,
                    int framesize)
{

    if(!m_ogg.Open('S') || !m_oggfile.Open(filename))
    {
        Close();
        return false;
    }

    m_samplerate = samplerate;
    m_frame_size = framesize;
    OpusHeader header = {};
    header.preskip = 0;
    header.channels = channels;
    header.channel_mapping = 0;
    header.input_sample_rate = samplerate;
    header.gain = 0;
    header.nb_streams = 1;
    header.nb_coupled = channels == 2? 1 : 0;

    unsigned char header_data[276];
    int packet_size = opus_header_to_packet(&header, header_data, sizeof(header_data));
    ogg_packet op = {};
    op.packet = header_data;
    op.bytes = packet_size;
    op.b_o_s = 1;
    op.e_o_s = 0;
    op.granulepos = 0;
    op.packetno = 0;

    m_ogg.PutPacket(op);

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

    m_ogg.PutPacket(op);

    ogg_page og;
    int ret;
    while((ret = m_ogg.FlushPageOut(og))>0)
    {
        m_oggfile.WriteOggPage(og);
    }
    m_packet_no = 2;
    return ret >= 0;
}

void OpusFile::Close()
{
    m_oggfile.Close();
    m_ogg.Close();
    m_samplerate = m_frame_size = 0;
    m_granule_pos = m_packet_no = 0;
}

int OpusFile::WriteEncoded(const char* enc_data, int enc_len, bool last)
{
    ogg_packet op = {};
    op.packet = reinterpret_cast<unsigned char*>(const_cast<char*>(enc_data));
    op.bytes = enc_len;
    op.b_o_s = 0;
    op.e_o_s = last? 1 : 0;
    op.granulepos = m_granule_pos;
    op.packetno = m_packet_no++;
    m_granule_pos += m_frame_size * 48000 / m_samplerate;

    m_ogg.PutPacket(op);

    ogg_page og;
    int ret;
    while((ret = m_ogg.FlushPageOut(og)) > 0)
    {
        ret = m_oggfile.WriteOggPage(og);
    }
    assert(ret >= 0);

    return ret;
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

    if(!m_file.Open(filename, channels, samplerate, framesize))
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
