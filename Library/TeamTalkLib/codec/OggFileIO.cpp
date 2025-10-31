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

#include "OggFileIO.h"

#include <cassert>

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

void OggInput::Reset()
{
    assert(ogg_stream_check(&m_os) == 0);

    ogg_stream_reset(&m_os);
}

int OggInput::PutPage(ogg_page& og)
{
    assert(ogg_stream_check(&m_os) == 0);

    if (ogg_page_serialno(&og) != m_os.serialno)
        ogg_stream_reset_serialno(&m_os, ogg_page_serialno(&og));

    return ogg_stream_pagein(&m_os, &og);
}

int OggInput::GetPacket(ogg_packet& op, bool peek/* = false*/)
{
    return peek ? ogg_stream_packetpeek(&m_os, &op) : ogg_stream_packetout(&m_os, &op);
}


OggFile::~OggFile()
{
    Close();
}

bool OggFile::NewFile(const ACE_TString& filename)
{
    return m_file.NewFile(filename);
}

bool OggFile::Open(const ACE_TString& filename)
{
    if (!m_file.Open(filename))
        return false;
    
    return ogg_sync_init(&m_state) == 0;
}

void OggFile::Close()
{
    m_file.Close();

    if (ogg_sync_check(&m_state) == 0)
    {
        ogg_sync_clear(&m_state);
    }
    m_last_gp = -1;
}

int OggFile::ReadOggPage(ogg_page& og)
{
    assert(ogg_sync_check(&m_state) == 0);
    
    int pages = 0;
    while ((pages = ogg_sync_pageout(&m_state, &og)) != 1)
    {
        auto SIZE = 0x1000;
        auto *buffer = ogg_sync_buffer(&m_state, SIZE);
        auto ret = m_file.Read(buffer, SIZE);
        if (ret > 0)
        {
            ret = ogg_sync_wrote(&m_state, long(ret));
            assert(ret == 0);
        }
        else break;
    }

    if (pages > 0)
    {
        // store for seek
        auto gp = ogg_page_granulepos(&og);
        if (gp >= 0)
            m_last_gp = gp;
    }

    return pages;
}

int OggFile::WriteOggPage(const ogg_page& og)
{
    auto bytes_out = m_file.Write(reinterpret_cast<const char*>(og.header), og.header_len);
    if (bytes_out > 0)
    {
        m_last_gp = ogg_page_granulepos(&og);
        auto ret = m_file.Write(reinterpret_cast<const char*>(og.body), og.body_len);
        if (ret > 0)
            bytes_out += ret;
    }

    return int(bytes_out);
}

bool OggFile::Seek(ogg_int64_t granulepos, ogg_page& og)
{
    if (!m_file.Seek(0, std::ios_base::beg))
        return false;

    ogg_sync_reset(&m_state);
    
    while (ReadOggPage(og) > 0)
    {
        auto gp = ogg_page_granulepos(&og);
        if (gp >= granulepos)
            return true;
    }

    return false;
}

ogg_int64_t OggFile::LastGranulePos()
{
    auto origin_gp = m_last_gp;
    ogg_page og;
    if (!Seek(0, og))
        return -1;

    while(ReadOggPage(og) > 0);
    auto lastgp = m_last_gp;
    if (!Seek(origin_gp, og))
        return 0;

    return lastgp;
}

ogg_int64_t OggFile::CurrentGranulePos() const
{
    return m_last_gp;
}

bool OggFile::SeekLog2(ogg_int64_t granulepos, ogg_page& og)
{
    assert(ogg_sync_check(&m_state) == 0);

    if (!m_file.Seek(0, std::ios_base::end))
        return false;

    const auto FILESIZE = m_file.Tell();
    auto half = FILESIZE / 2;
    auto nextpos = half;
    auto closestpos = FILESIZE * 0;

    ogg_int64_t gp = 0;
    do
    {
        if (half == 0)
            break;

        if (!m_file.Seek(nextpos, std::ios_base::beg))
            return false;

        if (!SyncPage())
            break;

        if (ReadOggPage(og) != 1)
            break;

        closestpos = nextpos;

        //std::cout << "New pos: " << closestpos << "/" << FILESIZE << std::endl;

        gp = ogg_page_granulepos(&og);

        half = std::abs(half / 2);
        half *= (granulepos > gp ? 1 : -1);
        nextpos += half;

        assert(closestpos >= 0);
        assert(closestpos < FILESIZE);
    }
    while (granulepos != gp);

    if (closestpos <= 0)
        return false;

    m_file.Seek(closestpos, std::ios_base::beg);

    return SyncPage() && ReadOggPage(og) > 0;
}

ogg_int64_t OggFile::LastGranulePosLog2()
{
    assert(ogg_sync_check(&m_state) == 0);

    const auto ORIGIN = m_file.Tell();

    if (!m_file.Seek(0, std::ios_base::end))
        return -1;

    ogg_int64_t gp = -1;
    if (SyncPage())
    {
        ogg_page og;
        while (ReadOggPage(og) > 0)
        {
            gp = ogg_page_granulepos(&og);
        }
        ogg_sync_reset(&m_state);
    }

    // rewind to origin
    m_file.Seek(ORIGIN, std::ios_base::beg);
    return gp;
}


bool OggFile::SyncPage()
{
    assert(ogg_sync_check(&m_state) == 0);

    const auto SIZE = 0x1000;
    const auto ORIGIN = m_file.Tell();

    // first seek forwards
    long skip = 0;
    int64_t n_read = 0;
    do
    {
        ogg_sync_reset(&m_state);
        auto *buffer = ogg_sync_buffer(&m_state, SIZE);
        n_read = m_file.Read(buffer, long(SIZE));
        if (n_read > 0)
        {
            int const ret = ogg_sync_wrote(&m_state, long(n_read));
            assert(ret == 0);
        }
        else break;

        ogg_page og;
        skip = ogg_sync_pageseek(&m_state, &og);
    } while (skip == 0);

    // now try backwards if failed
    if (skip == 0)
    {
        if (!m_file.Seek(ORIGIN, std::ios_base::beg))
            return false;

        auto backwards = ORIGIN;

        do
        {
            if (backwards == 0)
                return false; // already at begining last time

            ogg_sync_reset(&m_state);
            backwards = std::max(backwards - SIZE, int64_t(0));
            if (!m_file.Seek(backwards, std::ios_base::beg))
                return false;

            auto *buffer = ogg_sync_buffer(&m_state, SIZE);
            n_read = m_file.Read(buffer, SIZE);
            if (n_read > 0)
            {
                int const ret = ogg_sync_wrote(&m_state, long(n_read));
                assert(ret == 0);
            }
            else break;

            ogg_page og;
            skip = ogg_sync_pageseek(&m_state, &og);
        } while (skip == 0);
    }

    auto offset = -n_read;
    if (skip < 0)
        offset += skip;

    if (!m_file.Seek(offset, std::ios_base::cur))
        return false;

    ogg_sync_reset(&m_state);
    return true;
}

#if defined(ENABLE_SPEEX)

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
    int packet_size;
    int ret;

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
    if(m_msec_per_packet == 0)
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

double speex_granule_time(const SpeexHeader& spx_header,
                          spx_int32_t lookahead, ogg_int64_t granpos);

int speex_packet_jump(int msec_per_packet,
                      unsigned int last_timestamp,
                      unsigned int cur_timestamp);

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

        short const diff = (short)packetno - (short)m_last_packetno;
        if(diff>0)
            m_counter += diff;
    }
    else
        m_counter++;

    m_last_timestamp = timestamp;
    m_last_packetno = packetno;

    ogg_int64_t const total_samples = ((m_counter) * m_frame_size) + m_frame_size;

    ogg_packet op;
    op.packet = (unsigned char*)enc_data;
    op.bytes = len;
    op.b_o_s = 0;
    op.e_o_s = (long)last;
    //do not modify unless also changing 'speex_granule_time(..)'
    op.granulepos = (m_counter)*m_frame_size - m_lookahead;
    op.granulepos = std::min(op.granulepos, total_samples);

    op.packetno = m_counter+1;

    return m_ogg.PutPacket(op);
}

double speex_granule_time(const SpeexHeader& spx_header,
                          spx_int32_t lookahead, ogg_int64_t granpos)
{
    if(spx_header.rate == 0)
        return 0.0;
    double const total_samples = (double)granpos + lookahead;
    return total_samples / (double)spx_header.rate;
}

int speex_packet_jump(int msec_per_packet,
                      unsigned int last_timestamp,
                      unsigned int cur_timestamp)
{
    int const time_diff = (int)cur_timestamp - (int)last_timestamp;
    int const packet_diff = time_diff / msec_per_packet;
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
    m_spx_header.vbr = static_cast<spx_int32_t>(vbr);

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
    int ret = 0;
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
        int ret = 0;
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

    int ret = 0;
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

bool SpeexEncFile::Open(const ACE_TString& filename,
                        int bandmode, int complexity, float vbr_quality,
                        int bitrate, int maxbitrate, bool dtx)
{
    if(!m_encoder.Initialize(bandmode, complexity, vbr_quality, bitrate, maxbitrate, dtx))
        return false;
    if(!m_file.Open(filename, bandmode, (bitrate != 0) || (maxbitrate != 0)))
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
    int const ret = m_encoder.Encode(samples, m_buffer.data(), int(m_buffer.size()));
    if(ret>0)
        return m_file.WriteEncoded(m_buffer.data(), ret, last);
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
                       int  /*framesize*/)
{
    // Make the stream (ogg stream 'serialno') as unique as possible on the same machine by using the timestamp as id.
    // Facilitates the subsequent handling of the recording with tooling like oggz-merge to mux the recordings.
    // If we use the same stream for every recording then muxing of recording always involves rewritting the id.
    if (!m_oggout.Open(GETTIMESTAMP()) || !m_oggfile.NewFile(filename))
    {
        Close();
        return false;
    }

    m_header.preskip = 3840;  // 80 ms @ 48kHz as per rfc7845 - 5.1 bullet 4
    m_header.channels = channels;
    m_header.channel_mapping = 0;
    m_header.input_sample_rate = samplerate;
    m_header.gain = 0;
    m_header.nb_streams = 1;
    m_header.nb_coupled = channels == 2? 1 : 0;

    unsigned char header_data[276];
    int const packet_size = opus_header_to_packet(&m_header, header_data, sizeof(header_data));
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
    int ret = 0;
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
    else
    {
        // first store duration of file. Needed by GetTotalSamples()
        m_last_granule_pos = m_oggfile.LastGranulePos();
        // rewind and find first page (again)
        ogg_page og;
        if (m_last_granule_pos < 0 || !m_oggfile.Seek(0, og))
        {
            Close();
            return false;
        }

        if (!m_oggin.Open(og) || m_oggin.PutPage(og) != 0)
        {
            Close();
            return false;
        }
    }

    int ret = 0;
    m_packet_no = -1;
    while (true)
    {
        ogg_packet op;
        if (m_oggin.GetPacket(op) == 1)
        {
            if ((op.b_o_s != 0) && op.bytes >= 8 && memcmp(op.packet, "OpusHead", 8) == 0 &&
                opus_header_parse(op.packet, op.bytes, &m_header) == 1)
            {
                // found opus header
                m_packet_no = op.packetno;
            }
            else if (m_packet_no == 0)
            {
                // skip comments
                m_packet_no = op.packetno;
                break;
            }
        }

        ogg_page og;
        ret = m_oggfile.ReadOggPage(og);
        if (ret > 0)
        {
            ret = m_oggin.PutPage(og);
            assert(ret == 0);
            if (ret != 0)
                break;
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

    m_last_granule_pos = m_granule_pos = m_packet_no = 0;
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

int OpusFile::WriteEncoded(const char* enc_data, int enc_len, int framesize, bool last)
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
    m_granule_pos += framesize * 48000 / m_header.input_sample_rate;
    op.granulepos = m_granule_pos;
    op.packetno = m_packet_no++;

    m_oggout.PutPacket(op);

    ogg_page og;
    int ret = 0;
    while((ret = m_oggout.FlushPageOut(og)) > 0)
    {
        ret = m_oggfile.WriteOggPage(og);
        assert(ret >= 0);
    }
    assert(ret >= 0);

    return ret;
}

const unsigned char* OpusFile::ReadEncoded(int& bytes, ogg_int64_t* sampleduration /*= nullptr*/)
{
    ogg_packet op;
    ogg_page og;
    op.packet = nullptr;

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

    if (op.packet != nullptr)
    {
        m_packet_no = op.packetno; // unused by decoder
        m_granule_pos = op.granulepos; // unused by decoder
        bytes = op.bytes;

        if (sampleduration != nullptr)
        {
            if (op.granulepos >= 0)
                *sampleduration = (op.granulepos / (48000 / m_header.input_sample_rate));
            else
                *sampleduration = -1;
        }
    }

    return op.packet;
}

bool OpusFile::Seek(ogg_int64_t samplesoffset)
{
    ogg_page og;
    ogg_int64_t const granulepos = samplesoffset * (48000 / m_header.input_sample_rate);

    // special handling of "granulepos == 0" because we need to
    // skip "OpusHead" packet #0 and "OpusTags packet #1 in OggFile.
    if (granulepos == 0)
    {
        if (!m_oggfile.Seek(0, og)) //skip 1 "OpusHead"
            return false;

        if (m_oggfile.ReadOggPage(og) != 1)  //skip 1 "OpusTags"
            return false;

        // flush whatever we already have
        m_oggin.Reset();

        return true;
    }

    if (m_oggfile.Seek(granulepos, og))
    {
        // flush whatever we already have
        m_oggin.Reset();

        return true;
    }
    return false;
}

ogg_int64_t OpusFile::GetTotalSamples() const
{
    if (m_last_granule_pos >= 0)
        return m_last_granule_pos / (48000 / m_header.input_sample_rate);
    return -1;
}

ogg_int64_t OpusFile::GetSamplesPosition() const
{
    auto gp = m_oggfile.CurrentGranulePos();
    if (gp >= 0)
    {
        return gp / (48000 / m_header.input_sample_rate);
    }
    return -1;
}

#endif /* ENABLE_OPUSTOOLS */

#if defined(ENABLE_OPUSTOOLS) && defined(ENABLE_OPUS)

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

int OpusEncFile::Encode(const short* input_buffer, int input_samples, bool last)
{
    assert(input_buffer);
    int const ret = m_encoder.Encode(input_buffer, input_samples,
                               m_buffer.data(), int(m_buffer.size()));
    if (ret > 0)
        return m_file.WriteEncoded(m_buffer.data(), ret, input_samples, last);

    return ret;
}

bool OpusDecFile::Open(const ACE_TString& filename)
{
    if (m_file.OpenFile(filename) && m_decoder.Open(m_file.GetSampleRate(), m_file.GetChannels()))
        return true;

    Close();
    return false;
}

void OpusDecFile::Close()
{
    m_file.Close();
    m_decoder.Close();
    m_samples_decoded = 0;
}

int OpusDecFile::GetSampleRate() const
{
    return m_file.GetSampleRate();
}

int OpusDecFile::GetChannels() const
{
    return m_file.GetChannels();
}

int OpusDecFile::Decode(short* input_buffer, int input_samples)
{
    int bytes = 0;
    ogg_int64_t samples_read = 0;
    const auto *opusbuf = m_file.ReadEncoded(bytes, &samples_read);
    if (opusbuf == nullptr)
        return 0;

    int const samples = m_decoder.Decode(reinterpret_cast<const char*>(opusbuf), bytes, input_buffer, input_samples);
    if (samples > 0)
        m_samples_decoded += samples;
    return samples;
}

bool OpusDecFile::Seek(uint32_t offset_msec)
{
    double const offset_sec = offset_msec / 1000.;
    if (m_file.Seek(ogg_int64_t(GetSampleRate() * offset_sec)))
    {
        auto samples = m_file.GetSamplesPosition();
        if (samples >= 0)
            m_samples_decoded = samples;

        return true;
    }

    return false;
}

uint32_t OpusDecFile::GetDurationMSec()
{
    auto samplestotal = m_file.GetTotalSamples();
    if (samplestotal >= 0)
    {
        samplestotal *= 1000;
        return uint32_t(samplestotal / GetSampleRate());
    }
    return 0;
}

uint32_t OpusDecFile::GetElapsedMSec() const
{
    return uint32_t((m_samples_decoded * 1000) / GetSampleRate());
}

#endif /* ENABLE_OPUSTOOLS && ENABLE_OPUS */

#if defined(ENABLE_VORBIS)

VorbisFile::VorbisFile()
{
}

VorbisFile::~VorbisFile()
{
    Close();
}

bool VorbisFile::OpenFile(const ACE_TString& filename)
{
    Close();

#ifdef UNICODE
    FILE* fp = _wfopen(filename.c_str(), L"rb");
#else
    FILE* fp = fopen(filename.c_str(), "rb");
#endif

    if (!fp)
        return false;

    if (ov_open(fp, &m_vf, NULL, 0) < 0)
    {
        fclose(fp);
        return false;
    }

    vorbis_info* vi = ov_info(&m_vf, -1);
    if (!vi)
    {
        Close();
        return false;
    }

    m_samplerate = vi->rate;
    m_channels = vi->channels;
    m_opened = true;

    return true;
}

void VorbisFile::Close()
{
    if (m_opened)
    {
        ov_clear(&m_vf);
        m_opened = false;
    }
    m_samplerate = 0;
    m_channels = 0;
}

uint32_t VorbisFile::GetDurationMSec() const
{
    if (!m_opened)
        return 0;

    double duration = ov_time_total(const_cast<OggVorbis_File*>(&m_vf), -1);
    if (duration < 0)
        return 0;

    return uint32_t(duration * 1000.0);
}

bool VorbisDecFile::Open(const ACE_TString& filename)
{
    if (!m_file.OpenFile(filename))
    {
        Close();
        return false;
    }

    m_samples_decoded = 0;
    return true;
}

void VorbisDecFile::Close()
{
    m_file.Close();
    m_samples_decoded = 0;
}

int VorbisDecFile::GetSampleRate() const
{
    return m_file.GetSampleRate();
}

int VorbisDecFile::GetChannels() const
{
    return m_file.GetChannels();
}

int VorbisDecFile::Decode(short* output_buffer, int output_samples)
{
    if (!m_file.IsOpen())
        return 0;

    int total_samples = 0;
    int bytes_to_read = output_samples * GetChannels() * sizeof(short);

    while (bytes_to_read > 0)
    {
        int bitstream;
        long bytes_read = ov_read(&m_file.GetVorbisFile(),
                                  reinterpret_cast<char*>(output_buffer) + total_samples * GetChannels() * sizeof(short),
                                  bytes_to_read,
                                  0,  // little endian
                                  2,  // 16-bit samples
                                  1,  // signed
                                  &bitstream);

        if (bytes_read <= 0)
            break;  // End of file or error

        int samples_read = bytes_read / (GetChannels() * sizeof(short));
        total_samples += samples_read;
        bytes_to_read -= bytes_read;
        m_samples_decoded += samples_read;
    }

    return total_samples;
}

bool VorbisDecFile::Seek(uint32_t offset_msec)
{
    if (!m_file.IsOpen())
        return false;

    double offset_sec = offset_msec / 1000.0;
    if (ov_time_seek(&m_file.GetVorbisFile(), offset_sec) == 0)
    {
        m_samples_decoded = static_cast<ogg_int64_t>(offset_sec * GetSampleRate());
        return true;
    }

    return false;
}

uint32_t VorbisDecFile::GetDurationMSec()
{
    return m_file.GetDurationMSec();
}

uint32_t VorbisDecFile::GetElapsedMSec() const
{
    if (GetSampleRate() == 0)
        return 0;
    return uint32_t((m_samples_decoded * 1000) / GetSampleRate());
}

#endif /* ENABLE_VORBIS */
