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

#include "PacketHelper.h"
#include <teamtalk/ttassert.h>
#include <myace/MyACE.h>

using namespace std;

#define MAX_PACKETS_ON_WIRE 16

namespace teamtalk {

audiopackets_t BuildAudioPackets(uint16_t src_userid,
                                 uint32_t time,
                                 uint8_t streamid,
                                 uint16_t packet_no,
                                 const char* enc_data,
                                 uint16_t enc_length,
                                 const std::vector<int>* enc_frame_sizes)
{
    if(enc_frame_sizes)
    {
        int h_len = AUDIOPACKET_TYPICAL_VBR_HEADER_SIZE;
        int p_len = h_len + enc_length;
        if(enc_frame_sizes->size() % 2 == 1)
            p_len += int((enc_frame_sizes->size()*12 / 8) + 1);
        else
            p_len += int(enc_frame_sizes->size()*12 / 8);

        MYTRACE(ACE_TEXT("Packet prediction for %d is %d bytes\n"), 
            packet_no, p_len);
    }
    else
    {
        int h_len = AUDIOPACKET_TYPICAL_CBR_HEADER_SIZE;
        int p_len = h_len + enc_length;
        MYTRACE(ACE_TEXT("Packet prediction for %d is %d bytes\n"), 
            packet_no, p_len);
    }
    return audiopackets_t();
}

audiopackets_t BuildAudioFragments(const AudioPacket& in_packet,
                                   uint16_t max_chunk_size)
{
    TTASSERT(in_packet.Finalized());
    audiopackets_t result;
    uint16_t enc_size = 0;
    const char* ptr = in_packet.GetEncodedAudio(enc_size);
    if(enc_size <= max_chunk_size)
        return result;

    int fragments = enc_size / max_chunk_size;
    if(enc_size % max_chunk_size != 0)
        fragments++;

    if(fragments >= 0xFF)
        return result;

    AudioPacket* p;
    uint8_t stream_id = in_packet.GetStreamID();

    //WARNING: if FieldPacket is changed these additional fields must be 
    //added here.
    uint16_t dest_userid = in_packet.GetDestUserID();
    uint16_t chanid = in_packet.GetChannel();

    uint8_t u8_fragments = fragments;
    if(in_packet.HasFrameSizes())
    {
        //only fragment 0 has framesize info
        std::vector<uint16_t> frame_sizes = in_packet.GetEncodedFrameSizes();
        ACE_NEW_RETURN(p, AudioPacket(in_packet.GetKind(),
                                      in_packet.GetSrcUserID(), 
                                      in_packet.GetTime(),
                                      stream_id,
                                      in_packet.GetPacketNumber(),
                                      (uint8_t)0, &u8_fragments,
                                      ptr, max_chunk_size,
                                      &frame_sizes), 
                                      result);
    }
    else
    {
        ACE_NEW_RETURN(p, AudioPacket(in_packet.GetKind(),
                                      in_packet.GetSrcUserID(), 
                                      in_packet.GetTime(),
                                      stream_id,
                                      in_packet.GetPacketNumber(),
                                      (uint8_t)0, &u8_fragments,
                                      ptr, max_chunk_size), 
                                      result);
    }
    result.push_back(audiopacket_t(p));

    if(dest_userid)
        p->SetDestUser(dest_userid);
    p->SetChannel(chanid);

    uint16_t copied = max_chunk_size;

    for(uint8_t i=1;i<(uint8_t)fragments;i++)
    {
        uint16_t copy_size = (copied + max_chunk_size > enc_size)?
            enc_size-copied : max_chunk_size;

        ACE_NEW_RETURN(p, AudioPacket(in_packet.GetKind(),
                                      in_packet.GetSrcUserID(), 
                                      in_packet.GetTime(),
                                      stream_id,
                                      in_packet.GetPacketNumber(),
                                      i, NULL,
                                      &ptr[copied], copy_size), 
                                      result);
        copied += copy_size;
        result.push_back(audiopacket_t(p));

        if(dest_userid)
            p->SetDestUser(dest_userid);
        p->SetChannel(chanid);
    }
    assert(copied == enc_size);

    return result;
}

audiopacket_t ReassembleAudioPacket(const audiofragments_t& fragments,
                                    const AudioCodec& codec)
{
    audiofragments_t::const_iterator ii = fragments.find(0);
    if(ii == fragments.end())
        return audiopacket_t();
    
    TTASSERT(ii->second->HasFragments());
    TTASSERT(ii->second->GetPacketNumber() == 0);

    uint8_t frag_no = 0, frag_cnt = 0;
    uint16_t packet_no = ii->second->GetPacketNumberAndFragNo(frag_no, &frag_cnt);

    //MYTRACE_COND(fragments.size() != frag_cnt,
    //             ACE_TEXT("Ejected packet %d due to missing fragments. Had %d needed %d\n"),
    //             packet_no, fragments.size(), frag_cnt);
    if(fragments.size() != frag_cnt)
        return audiopacket_t();

    uint16_t available = 0;
    ii = fragments.begin();
    while(ii != fragments.end())
    {
        uint16_t tmp = 0;
        ii->second->GetEncodedAudio(tmp);
        available += tmp;
        ii++;
    }

    uint16_t enc_size = 0;
    vector<uint16_t> frame_sizes;
    ii = fragments.find(0);
    if(ii->second->HasFrameSizes())
    {
        frame_sizes = ii->second->GetEncodedFrameSizes();
        enc_size = SumFrameSizes(frame_sizes);
        if(available != enc_size)
            return audiopacket_t();
    }
    else
        enc_size = available;

    uint16_t copied = 0, pktenc_len = 0;
    frag_no = 0;
    std::vector<char> buf((size_t)enc_size);
    ii = fragments.find(0);
    while(ii != fragments.end())
    {
        const char* ptr = ii->second->GetEncodedAudio(pktenc_len);
        if(!ptr || !pktenc_len || (size_t)copied+(size_t)pktenc_len>buf.size())
            break;
        memcpy(&buf[copied], ptr, pktenc_len);
        copied += pktenc_len;
        ii = fragments.find(++frag_no);
    }
    if(copied != enc_size)
        return audiopacket_t();

    ii = fragments.find(0);
    const AudioPacket& frag0 = *ii->second;
    AudioPacket* audpkt;
    if(frame_sizes.size())
        ACE_NEW_RETURN(audpkt, AudioPacket(frag0.GetKind(),
                                           frag0.GetSrcUserID(), //VBR
                                           frag0.GetTime(), 
                                           frag0.GetStreamID(), 
                                           packet_no, 
                                           &buf[0], uint16_t(buf.size()),
                                           frame_sizes), 
                                           audiopacket_t());
    else
        ACE_NEW_RETURN(audpkt, AudioPacket(frag0.GetKind(),
                                           frag0.GetSrcUserID(), 
                                           frag0.GetTime(), 
                                           frag0.GetStreamID(), 
                                           packet_no, 
                                           &buf[0], uint16_t(buf.size())),
                                           audiopacket_t());
    //WARNING: if FieldPacket is changed these additional fields must be 
    //added here.
    uint16_t dest_userid = frag0.GetDestUserID();
    uint16_t chanid = frag0.GetChannel();
    if(dest_userid)
        audpkt->SetDestUser(dest_userid);
    if(chanid)
        audpkt->SetChannel(chanid);

    return audiopacket_t(audpkt);
}

std::vector<uint16_t> GetAudioPacketFrameSizes(const AudioPacket& packet,
                                               const AudioCodec& codec)
{
    std::vector<uint16_t> frame_sizes;
    if(packet.HasFrameSizes())
        frame_sizes = packet.GetEncodedFrameSizes();
    else if(GetAudioCodecFramesPerPacket(codec)>1)
        frame_sizes.assign(GetAudioCodecFramesPerPacket(codec), 
                           GetAudioCodecEncFrameSize(codec));
    else
    {
        uint16_t enc_len;
        if(packet.GetEncodedAudio(enc_len))
            frame_sizes.push_back(enc_len);
    }
    return frame_sizes;
}


videopackets_t BuildVideoPackets(uint8_t kind,
                                 uint16_t src_userid,
                                 uint32_t time,
                                 uint16_t max_chunk_size,
                                 uint8_t streamid,
                                 uint32_t packet_no,
                                 uint16_t* width,
                                 uint16_t* height,
                                 const char* enc_data,
                                 uint32_t enc_len)
{
    videopackets_t result;
    uint16_t fragno = 0;
    uint16_t fragcnt = (uint16_t)(enc_len / max_chunk_size);
    if(enc_len % max_chunk_size)
        fragcnt++;

    TTASSERT(enc_len / max_chunk_size < 0xFFFF);
    if(enc_len / max_chunk_size >= 0xFFFF)
        return result;

    if(fragcnt > 1)
    {
        const char* payload_ptr = enc_data;
        VideoPacket* vp;
        ACE_NEW_RETURN(vp, VideoPacket(kind, src_userid, time, streamid, 
                                       packet_no, width, height, payload_ptr,
                                       max_chunk_size, fragcnt),
                       result);
        if(vp)
            result.push_back(vp);

        for(fragno=1;fragno<fragcnt-1;fragno++)
        {
            payload_ptr += max_chunk_size;
            ACE_NEW_NORETURN(vp, VideoPacket(kind, src_userid, time, streamid, 
                                             packet_no, payload_ptr,
                                             max_chunk_size, fragno));
            if(vp)
                result.push_back(vp);
            else
            {
                for(size_t i=0;i<result.size();i++)
                    delete result[i];
                result.clear();
                return result;
            }
        }
        payload_ptr += max_chunk_size;
        uint16_t remaining = (uint16_t)(enc_len - (fragcnt - 1) * max_chunk_size);
        ACE_NEW_NORETURN(vp, VideoPacket(kind, src_userid, time, streamid, 
                                         packet_no, payload_ptr, 
                                         remaining, fragno));
        if(vp)
            result.push_back(vp);
        else
        {
            for(size_t i=0;i<result.size();i++)
                delete result[i];
            result.clear();
            return result;
        }
    }
    else
    {
        VideoPacket* vp;
        ACE_NEW_NORETURN(vp, VideoPacket(kind, src_userid, time, streamid, 
                                         packet_no, width, height, enc_data,
                                         enc_len));
        if(vp)
            result.push_back(vp);
    }
    return result;

}

bool ReassembleVideoPackets(const video_fragments_t& fragments,
                            const VideoPacket& packet,
                            std::vector<char>& enc_frame)
{
    uint16_t fragno = packet.GetFragmentNo();
    //find out if we can reassemble the packet
    if(fragments.find(fragno) != fragments.end())
        return false; //retransmission, so return

    //find out if we have all the fragments
    uint16_t fragcnt = 0;
    video_fragments_t::const_iterator frag_ite = fragments.find(0);
    if(frag_ite != fragments.end())
        fragcnt = frag_ite->second->GetFragmentCount();
    else if(packet.GetFragmentNo() == 0)
        fragcnt = packet.GetFragmentCount();

    if(fragcnt == 0 || fragments.size()+1 < fragcnt)
        return false; //not all fragments have been received

    uint16_t frag_size = 0;
    uint32_t frame_size = 0;
    for(uint16_t i=0;i<fragcnt;i++)
    {
        frag_ite = fragments.find(i);
        if(frag_ite != fragments.end())
        {
            if(!frag_ite->second->GetEncodedData(frag_size))
                return false;
            frame_size += frag_size;
        }
        else if(i == packet.GetFragmentNo())
        {
            if(!packet.GetEncodedData(frag_size))
                return false;
            frame_size += frag_size;
        }
        else
            return false;
    }
    enc_frame.resize(frame_size);
    uint32_t copied = 0;
    for(uint16_t i=0;i<fragcnt;i++)
    {
        frag_ite = fragments.find(i);
        const char* data;
        if(frag_ite != fragments.end())
        {
            data = frag_ite->second->GetEncodedData(frag_size);
            ACE_OS::memcpy(&enc_frame[copied], data, frag_size);
            copied += frag_size;
        }
        else if(i == packet.GetFragmentNo())
        {
            data = packet.GetEncodedData(frag_size);
            ACE_OS::memcpy(&enc_frame[copied], data, frag_size);
            copied += frag_size;
        }
        else
            return false;
    }
    assert(enc_frame.size() == copied);
    return true;
}

void UpdateBlocksCRC(const map_blocks_t& blocks,
                     const std::set<uint16_t>& dirty_blocks,
                     map_block_crc_t& block_crcs,
                     map_crc_blocks_t& crc_blocks)
{
    set<uint16_t>::const_iterator si=dirty_blocks.begin();
    for(;si!=dirty_blocks.end();si++)
    {
        uint32_t crc32;
        map_block_crc_t::iterator bci = block_crcs.find(*si);
        if(bci != block_crcs.end())
        {
            //remove old CRC32 from 'crc_blocks'
            map_crc_blocks_t::iterator cbi = crc_blocks.find(bci->second);
            if(cbi != crc_blocks.end())
            {
                cbi->second.erase(*si);
                if(cbi->second.empty())
                    crc_blocks.erase(cbi);
            }
            //store new CRC32 value in 'crc_blocks'
            map_blocks_t::const_iterator bi = blocks.find(*si);
            TTASSERT(bi!=blocks.end());
            crc32 = ACE::crc32(&bi->second[0], bi->second.size());
            block_crcs[*si] = crc32;
        }
        else
        {
            //store block's CRC32 in 'block_crcs' and 'crc_blocks'
            map_blocks_t::const_iterator bi = blocks.find(*si);
            TTASSERT(bi!=blocks.end()); //this should never happen since a block is reported dirty which is not in the list of blocks
            if(bi!=blocks.end())
            {
                crc32 = ACE::crc32(&bi->second[0], bi->second.size());
                block_crcs[*si] = crc32;
            }
            else continue;
        }

        //search for duplicate block
        map_crc_blocks_t::iterator cbi = crc_blocks.find(crc32);
        if(cbi != crc_blocks.end())
        {
            //it's a duplicate
            TTASSERT(cbi->second.size());
            uint16_t dup_blockno = *cbi->second.begin();
            TTASSERT(dup_blockno != *si);
            cbi->second.insert(*si);
        }
        else
        {
            set<uint16_t> blocknums;
            blocknums.insert(*si);
            crc_blocks[crc32] = blocknums;
        }
        TTASSERT(crc_blocks.size() <= blocks.size());
    }
    TTASSERT(crc_blocks.size() <= blocks.size());
}

void DuplicateBlocks(const std::set<uint16_t>& dirty_blocks,
                     const map_block_crc_t& block_crcs,
                     const map_crc_blocks_t& crc_blocks,
                     map_dup_blocks_t& dup_blocks,
                     std::set<uint16_t>& ignore_blocks)
{
    set<uint16_t>::const_iterator si=dirty_blocks.begin();
    for(;si!=dirty_blocks.end();si++)
    {
        ACE_UINT32 crc32;
        map_block_crc_t::const_iterator bci = block_crcs.find(*si);
        TTASSERT(bci != block_crcs.end());
        if(bci != block_crcs.end())
            crc32 = bci->second;
        else
            continue;

        //search for duplicate block
        map_crc_blocks_t::const_iterator cbi = crc_blocks.find(crc32);
        if(cbi != crc_blocks.end())
        {
            TTASSERT(cbi->second.size());
            if(cbi->second.empty())
                continue;

            //ensure it's not ourself or a dirty block
            uint16_t dup_blockno = 0xFFFF;
            if(*si != *cbi->second.begin() &&
               dup_blocks.find(*si) == dup_blocks.end())
                dup_blockno = *cbi->second.begin();
            else
                continue;

            //it's a duplicate
            TTASSERT(dup_blockno != *si);
            TTASSERT(dup_blockno != 0xFFFF);
            map_dup_blocks_t::iterator dbi = dup_blocks.find(dup_blockno);
            if(dbi != dup_blocks.end())
                dbi->second.insert(*si);
            else
            {
                set<uint16_t> dups;
                dups.insert(*si);
                dup_blocks[dup_blockno] = dups;
            }

            ignore_blocks.insert(*si);
        }
    }
}

void InsertDuplicateBlocks(const map_dup_blocks_t& dup_blocks,
                           map_blocks_t& blocks, 
                           std::set<uint16_t>& updated_blocks)
{
    map_dup_blocks_t::const_iterator dbi = dup_blocks.begin();
    for(;dbi!=dup_blocks.end();dbi++)
    {
        map_blocks_t::iterator bi = blocks.find(dbi->first);
        //MYTRACE(ACE_TEXT("%d => "), dbi->first);
        TTASSERT(bi != blocks.end());
        if(bi != blocks.end())
        {
            set<uint16_t>::const_iterator ii = dbi->second.begin();
            for(;ii!=dbi->second.end();ii++)
            {
                //MYTRACE(ACE_TEXT("%d, "), *ii);
                blocks[*ii] = bi->second;
                updated_blocks.insert(*ii);
            }
        }
        //MYTRACE(ACE_TEXT("\n"));
    }
}

bool ExtractBlockRange(const std::set<uint16_t>& blocknums, 
                       std::set<uint16_t>& result_range)
{
    if(blocknums.size() < 2)
        return false;

    std::set<uint16_t>::const_iterator bi = blocknums.begin();
    uint16_t start_no = *bi;
    uint16_t count = 1;
    bi++;
    while(bi != blocknums.end())
    {
        if(start_no + count == *bi)
        {
            count++;
            bi++;
        }
        else if(count > 1)
            break;
        else
        {
            start_no = *bi;
            count = 1;
            bi++;
        }
    }

    while(count--)
        result_range.insert(start_no++);

    return !result_range.empty();
}

desktoppackets_t BuildDesktopPackets(bool new_session,
                                     uint16_t src_userid,
                                     uint32_t time,
                                     uint16_t max_chunk_size,
                                     uint16_t max_payload_size,
                                     const DesktopWindow& dwnd,
                                     const map_blocks_t& blocks,
                                     const map_dup_blocks_t& dup_blocks,
                                     const std::set<uint16_t>* inc_blocks/* = NULL*/,
                                     const std::set<uint16_t>* ignore_blocks/* = NULL*/)
{
    desktoppackets_t result;
    uint16_t packet_index = 0;
    int packets_size = 0;

    map_block_t send_blocks;
    block_frags_t send_frags;
    map_dup_blocks_t send_dup_blocks(dup_blocks); //TODO: fragment if too many dup single-blocks

    map_blocks_t::const_iterator ii = blocks.begin();
    for(;ii != blocks.end();ii++)
    {
        if(inc_blocks && inc_blocks->find(ii->first) == inc_blocks->end())
            continue;
        if(ignore_blocks && ignore_blocks->find(ii->first) != ignore_blocks->end())
            continue;

        if(ii->second.size() <= max_chunk_size) //block is small enough
        {
            desktop_block db;
            db.block_data = &ii->second[0];
            db.block_size = uint16_t(ii->second.size());

            send_blocks[ii->first] = db;
        }
        else //fragment the block
        {
            size_t n_frags = (ii->second.size() / max_chunk_size) +
                ((ii->second.size() % max_chunk_size) ? 1 : 0);
            assert(n_frags <= 0xFF);
            uint8_t frag_cnt = uint8_t(n_frags);

            for(uint8_t i=0;i<frag_cnt;i++)
            {
                block_fragment bf;
                bf.block_no = ii->first;
                bf.frag_no = i;
                bf.frag_cnt = frag_cnt;
                bf.frag_data = &ii->second[max_chunk_size*i];
                if(i<frag_cnt-1)
                    bf.frag_size = max_chunk_size;
                else
                    bf.frag_size = (ii->second.size() % max_chunk_size)?
                    ii->second.size() % max_chunk_size : max_chunk_size;
                assert(bf.frag_size);

                send_frags.push_back(bf);
            }
            //MYTRACE(ACE_TEXT("Block #%d fragmented into %d\n"), ii->first, frag_cnt);
        }
    }

    assert(send_blocks.size() || send_frags.size() || send_dup_blocks.size());

    map_block_t packet_blocks;
    block_frags_t packet_frags;
    mmap_dup_blocks_t packet_dup_blocks;

    //MYTRACE(ACE_TEXT("Update %u:%d: "), time, packet_index);
    int send_queue_bytes = SelectDesktopBlocks(new_session, send_blocks, 
                                               send_frags, send_dup_blocks,
                                               packet_blocks, packet_frags,
                                               packet_dup_blocks,
                                               max_payload_size);
    TTASSERT(send_queue_bytes>0);
    //MYTRACE(ACE_TEXT("\nTotal: %d\n"), send_queue_bytes);
    if(new_session)
    {
        DesktopPacket* p;
        ACE_NEW_NORETURN(p, DesktopPacket(src_userid, time,
                                          dwnd.session_id,
                                          dwnd.width, dwnd.height, 
                                          (uint8_t)dwnd.rgb_mode,
                                          packet_index++,
                                          0,//specified in UpdatePacketCount()
                                          packet_blocks, packet_frags,
                                          packet_dup_blocks));

        result.push_back(desktoppacket_t(p));
        packets_size += p->GetPacketSize();
    }
    else
    {
        DesktopPacket* p;
        ACE_NEW_NORETURN(p, DesktopPacket(src_userid, time,
                                          dwnd.session_id,
                                          packet_index++,
                                          0, //specified in UpdatePacketCount()
                                          packet_blocks, 
                                          packet_frags,
                                          packet_dup_blocks));

        result.push_back(desktoppacket_t(p));
        packets_size += p->GetPacketSize();
    }

    packet_blocks.clear();
    packet_frags.clear();
    packet_dup_blocks.clear();

    while(send_blocks.size() || send_frags.size() || send_dup_blocks.size())
    {
        //MYTRACE(ACE_TEXT("Update %u:%d: "), time, packet_index);
        send_queue_bytes = SelectDesktopBlocks(false, send_blocks, send_frags,
                                               send_dup_blocks, packet_blocks, 
                                               packet_frags, packet_dup_blocks,
                                               max_payload_size);
        //MYTRACE(ACE_TEXT("\nTotal: %d\n"), send_queue_bytes);
        TTASSERT(send_queue_bytes>0);
        TTASSERT(send_queue_bytes <= max_chunk_size + FIELDHEADER_PAYLOAD);

        DesktopPacket* p;
        ACE_NEW_NORETURN(p, DesktopPacket(src_userid, time,
                                          dwnd.session_id,
                                          packet_index++,
                                          0, //specified in UpdatePacketCount()
                                          packet_blocks, 
                                          packet_frags,
                                          packet_dup_blocks));

        packet_blocks.clear();
        packet_frags.clear();
        packet_dup_blocks.clear();

        result.push_back(desktoppacket_t(p));
        packets_size += p->GetPacketSize();
    }

    //now specify packet count for update
    desktoppackets_t::const_iterator dpi = result.begin();
    while(dpi != result.end())
    {
        (*dpi)->UpdatePacketCount(packet_index);
        TTASSERT((*dpi)->GetPacketSize() <= MAX_PACKET_SIZE);
        dpi++;
    }

    return result;
}

int SelectDesktopBlocks(bool initial_desktoppacket, 
                        map_block_t& send_blocks,
                        block_frags_t& send_frags,
                        map_dup_blocks_t& send_dup_blocks,
                        map_block_t& packet_blocks,
                        block_frags_t& packet_frags,
                        mmap_dup_blocks_t& packet_dup_blocks,
                        uint16_t max_payload_size)
{
    assert(packet_blocks.empty());
    assert(packet_frags.empty());
    assert(packet_dup_blocks.empty());

    int send_queue_bytes = 0, old_fields_usage = 0, new_fields_usage, data_usage = 0, fields_usage = 0;
    int blocks_cnt = 0, frags_cnt = 0, range_cnt = 0, single_blocks_cnt = 0;

    send_queue_bytes += DESKTOPPACKET_SESSIONUSAGE(initial_desktoppacket);
    fields_usage += DESKTOPPACKET_SESSIONUSAGE(initial_desktoppacket);

    //select the blocks which can fit in a packet
    map_block_t::iterator blocks_ite = send_blocks.begin();
    while(blocks_ite != send_blocks.end())
    {
        old_fields_usage = DESKTOPPACKET_DATAUSAGE(blocks_cnt, frags_cnt);
        new_fields_usage = DESKTOPPACKET_DATAUSAGE(blocks_cnt+1, frags_cnt);
        int diff_usage = new_fields_usage - old_fields_usage;
        TTASSERT(diff_usage>0);
        if(send_queue_bytes + blocks_ite->second.block_size + diff_usage 
           <= max_payload_size)
        {
            packet_blocks[blocks_ite->first] = blocks_ite->second;
            send_queue_bytes += blocks_ite->second.block_size;
            send_queue_bytes += diff_usage;
            data_usage += blocks_ite->second.block_size;
            fields_usage += diff_usage;
            blocks_cnt++;
            send_blocks.erase(blocks_ite++);
        }
        else
            blocks_ite++;
    }

    //select the fragments which can fit in a packet
    block_frags_t::iterator frags_ite = send_frags.begin();
    while(frags_ite != send_frags.end())
    {
        old_fields_usage = DESKTOPPACKET_DATAUSAGE(blocks_cnt, frags_cnt);
        new_fields_usage = DESKTOPPACKET_DATAUSAGE(blocks_cnt, frags_cnt+1);
        int diff_usage = new_fields_usage - old_fields_usage;
        TTASSERT(diff_usage>0);
        if(send_queue_bytes + frags_ite->frag_size + diff_usage
           <= max_payload_size)
        {
            packet_frags.push_back(*frags_ite);
            send_queue_bytes += frags_ite->frag_size;
            send_queue_bytes += diff_usage;
            data_usage += frags_ite->frag_size;
            fields_usage += diff_usage;
            frags_cnt++;
            send_frags.erase(frags_ite++);
        }
        else
            frags_ite++;
    }

    int single_blocks = 0;

    //select the duplicate block ranges or single blocks which can fit in packet
    map_dup_blocks_t::iterator dub_ite = send_dup_blocks.begin();
    while(dub_ite != send_dup_blocks.end())
    {
        TTASSERT(dub_ite->second.size());
        std::set<uint16_t> result_range;
        if(ExtractBlockRange(dub_ite->second, result_range))
        {
            old_fields_usage = DESKTOPPACKET_BLOCKRANGEUSAGE(range_cnt);
            new_fields_usage = DESKTOPPACKET_BLOCKRANGEUSAGE(range_cnt+1);
            int diff_usage = new_fields_usage - old_fields_usage;
            TTASSERT(diff_usage>0);
            if(send_queue_bytes + diff_usage <= max_payload_size)
            {
                send_queue_bytes += diff_usage;
                fields_usage += diff_usage;
                packet_dup_blocks.insert(dup_block_pair_t(dub_ite->first, result_range));
                range_cnt++;
                std::set<uint16_t>::iterator ii = result_range.begin();
                for(;ii!=result_range.end();ii++)
                    dub_ite->second.erase(*ii);

                if(dub_ite->second.empty())
                    send_dup_blocks.erase(dub_ite++);
            }
            else dub_ite++;
        }
        else
        {
            old_fields_usage = DESKTOPPACKET_BLOCKUSAGE(single_blocks_cnt,
                                                        single_blocks);
            new_fields_usage = DESKTOPPACKET_BLOCKUSAGE(single_blocks_cnt+1, 
                                        single_blocks + int(dub_ite->second.size()));
            int diff_usage = new_fields_usage - old_fields_usage;
            TTASSERT(diff_usage>0);
            if(send_queue_bytes + diff_usage <= max_payload_size)
            {
                send_queue_bytes += diff_usage;
                fields_usage += diff_usage;
                packet_dup_blocks.insert(dup_block_pair_t(dub_ite->first, dub_ite->second));
                single_blocks += int(dub_ite->second.size());
                single_blocks_cnt++;
                send_dup_blocks.erase(dub_ite++);
            }
            else dub_ite++;
        }
    }

#ifdef _DEBUG
    new_fields_usage = DESKTOPPACKET_SESSIONUSAGE(initial_desktoppacket);
    new_fields_usage += DESKTOPPACKET_DATAUSAGE(blocks_cnt, frags_cnt);
    new_fields_usage += DESKTOPPACKET_BLOCKRANGEUSAGE(range_cnt);
    new_fields_usage += DESKTOPPACKET_BLOCKUSAGE(single_blocks_cnt,
                                                 single_blocks);
    TTASSERT(fields_usage + data_usage == send_queue_bytes);
    TTASSERT(new_fields_usage + data_usage == send_queue_bytes);
#endif

    //MYTRACE(ACE_TEXT("\nRanges: %d, single entries: %d, single blocks: %d"),
    //        range_cnt, single_blocks_cnt, single_blocks);
    //MYTRACE(ACE_TEXT("\nData usage: %d, field usage: %d"), data_usage,
    //    new_fields_usage);

    TTASSERT(send_queue_bytes <= max_payload_size);
    TTASSERT(send_queue_bytes > 0);
    return send_queue_bytes;
}

void ReassembleDesktopBlocks(map_desktoppacket_t& frag_packets,
                             map_blocks_t& blocks)
{
#ifdef _DEBUG
    uint16_t session_id = 0;
#endif
    map_desktoppacket_t::iterator ii = frag_packets.begin();
    while(ii != frag_packets.end()) //iterate blocks
    {
        block_frags_t reassem_frags;
        const map_frag_desktoppacket_t& fragments = ii->second;
        map_frag_desktoppacket_t::const_iterator fi = fragments.begin();
        while(fi != fragments.end()) //iterate fragments
        {
            //look at the fragments of a packet
            block_frags_t tmp_frags;
            fi->second->GetBlockFragments(tmp_frags);
#ifdef _DEBUG
            if(!session_id)
                session_id = fi->second->GetSessionID();
            assert(fi->second->GetSessionID() == session_id);
#endif
            block_frags_t::const_iterator ff = tmp_frags.begin();

            while(ff != tmp_frags.end()) //iterate fragments in a packet
            {
                if(ff->block_no == ii->first)
                    reassem_frags.push_back(*ff);
                ff++;
            }
            fi++;
        }

        //see if we have all the fragments of the current block
        block_frags_t::iterator ri = reassem_frags.begin();
        if(reassem_frags.size() && ri->frag_cnt == reassem_frags.size())
        {
            //we can reassemble
            uint16_t block_size = 0, byte_pos = 0;
            while(ri != reassem_frags.end())
            {
                block_size += ri->frag_size;
                ri++;
            }
            vector<char> buf(block_size);
            ri = reassem_frags.begin();
            while(ri != reassem_frags.end())
            {
                memcpy(&buf[byte_pos], ri->frag_data, ri->frag_size);
                byte_pos += ri->frag_size;
                ri++;
            }

            blocks[ii->first] = buf;

            frag_packets.erase(ii++);
        }
        else
            ii++;
    }
}

bool GetAckedDesktopPackets(uint8_t session_id, uint32_t update_time, 
                            const desktoppackets_t& packets, 
                            std::set<uint16_t>& recv_packets)
{
    desktoppackets_t::const_iterator ii = packets.begin();
    while(ii != packets.end())
    {
        TTASSERT((*ii)->GetTime() == update_time);
        TTASSERT((*ii)->GetSessionID() == session_id);
        if((*ii)->GetTime() != update_time ||
            (*ii)->GetSessionID() != session_id)
            return false;

        recv_packets.insert((*ii)->GetPacketIndex());
        ii++;
    }
    return true;
}

int RemoveObsoleteDesktopPackets(const DesktopPacket& packet,
                                 desktoppackets_t& packets)
{
    int count = 0;
    desktoppackets_t::iterator ii = packets.begin();
    while(ii != packets.end())
    {
        if(!W32_GEQ(packet.GetTime(), (*ii)->GetTime()))
        {
            packets.erase(ii++);
            count++;
        }
        else ii++;
    }
    return count;
}


DesktopTransmitter::DesktopTransmitter(uint8_t session_id, uint32_t upd_timeid)
: m_session_id(session_id)
, m_update_timeid(upd_timeid)
, m_tx_count(4)
, m_pingtime(0)
{
}

void DesktopTransmitter::AddDesktopPacketToQueue(desktoppacket_t& packet)
{
    TTASSERT(packet->GetSessionID() == m_session_id);
    TTASSERT(packet->GetTime() == m_update_timeid);
    TTASSERT(packet->GetPacketIndex() != DesktopPacket::INVALID_PACKET_INDEX);
    m_queued_pkts[packet->GetPacketIndex()] = packet;
}

void DesktopTransmitter::AddSentDesktopPacket(const DesktopPacket& packet)
{
    uint16_t packet_no = packet.GetPacketIndex();

    //MYTRACE_COND(m_sent_pkts.find(packet_no) == m_sent_pkts.end(),
    //    ACE_TEXT("Desktop tx %d:%u - pkt index %d, %u\n"),
    //    GetSessionID(), GetUpdateID(), packet_no, GETTIMESTAMP());

    MYTRACE_COND(m_sent_pkts.find(packet_no) != m_sent_pkts.end(),
        ACE_TEXT("Desktop tx retransmitting %d:%u - pkt index %d, %u\n"),
        GetSessionID(), GetUpdateID(), packet_no, GETTIMESTAMP());
    TTASSERT(packet.GetTime() == GetUpdateID());

    //store time of first transmission of packet
    if(m_sent_times.find(packet_no) == m_sent_times.end())
        m_sent_ack_times[packet_no] = GETTIMESTAMP();
    else
        m_sent_ack_times.erase(packet_no);

    //store time of transmission
    m_sent_times[packet_no] = GETTIMESTAMP();

    TTASSERT(m_tx_count>=0);
    TTASSERT(m_sent_pkts.size() <= MAX_PACKETS_ON_WIRE);
}

bool DesktopTransmitter::IsDesktopPacketAcked(uint16_t packet_no) const
{
    return m_queued_pkts.find(packet_no) == m_queued_pkts.end() &&
        m_sent_pkts.find(packet_no) == m_sent_pkts.end();
}

bool DesktopTransmitter::ProcessDesktopAckPacket(const DesktopAckPacket& ack_packet)
{
    uint32_t tm = GETTIMESTAMP();
    //static int ack_size = ack_packet.GetPacketSize();
    //MYTRACE(ACE_TEXT("Ack Packet is %d bytes\n"), ack_size);
    //if(ack_packet.GetPacketSize()> ack_size)
    //    ack_size = ack_packet.GetPacketSize();

    uint16_t owner_userid;
    uint8_t session_id;
    uint32_t time_ack;
    if(!ack_packet.GetSessionInfo(owner_userid, session_id, time_ack))
        return false;

    TTASSERT(m_session_id == session_id);
    TTASSERT(m_update_timeid == time_ack);
    if(m_session_id != session_id || m_update_timeid != time_ack)
        return false;

    std::set<uint16_t> packet_nums;
    if(!ack_packet.GetPacketsAcked(packet_nums))
        return false;
    //{
    //MYTRACE(ACE_TEXT("Ack'ed: "));
    //std::set<uint16_t>::const_iterator ii = packet_nums.begin();
    //while(ii != packet_nums.end())
    //{
    //    MYTRACE(ACE_TEXT("%d,"), *ii);
    //    ii++;
    //}
    //MYTRACE(ACE_TEXT("\n"));
    //}
    TTASSERT(packet_nums.size());
    if(packet_nums.empty())
        return true;

    //remove packets which have been reported ack'ed
    //MYTRACE(ACE_TEXT("Ack'ed "));
    map_desktop_packets_t::iterator dpi = m_sent_pkts.begin();
    while(dpi != m_sent_pkts.end())
    {
        uint16_t ack_packetno = dpi->second->GetPacketIndex();

        if(packet_nums.find(ack_packetno) != packet_nums.end())
        {
            //MYTRACE(ACE_TEXT("%d,"), (int)packetno);
            m_sent_times.erase(ack_packetno);
            m_acked_missing.erase(ack_packetno);

            //calc round-trip
            map_sent_time_t::iterator sti = m_sent_ack_times.find(ack_packetno);
            if(sti != m_sent_ack_times.end())
            {
                m_pingtime = ACE_MAX((uint32_t)GETTIMESTAMP() - sti->second, m_pingtime);
                m_pingtime = ACE_MAX((uint32_t)1, m_pingtime);
                
                m_sent_ack_times.erase(ack_packetno);
            }

            m_sent_pkts.erase(dpi++);

            //an ACK releases two more packets for sending
            m_tx_count += 2;
            
            TTASSERT(m_sent_pkts.size() || m_tx_count>0);
            //never allow more than MAX_PACKETS_ON_WIRE packets on the wire
            if((int)m_sent_pkts.size() >= MAX_PACKETS_ON_WIRE)
                m_tx_count = 0;
            else if((int)m_sent_pkts.size() + m_tx_count >= MAX_PACKETS_ON_WIRE)
                m_tx_count = MAX_PACKETS_ON_WIRE - (int)m_sent_pkts.size();

            TTASSERT(m_tx_count>=0);
            TTASSERT(m_tx_count<=MAX_PACKETS_ON_WIRE);
            //MYTRACE(ACE_TEXT("Ack'ed desktop packet %d, tx_count is now %d\n"), ack_packetno, m_tx_count);
            TTASSERT(m_sent_pkts.size() || m_tx_count>0);
        }
        else dpi++;
    }
    //MYTRACE(ACE_TEXT("\n"));

    //store max packet
    uint16_t max_packet_no = *(--packet_nums.end());

    //store which packets have been reported missing (holes in packet order)
    for(uint16_t packet_no=0;packet_no<=max_packet_no;packet_no++)
    {
        if(packet_nums.find(packet_no) != packet_nums.end())
            continue;

        map_acked_missing_t::iterator ali = m_acked_missing.find(packet_no);
        if(ali == m_acked_missing.end())
        {
            m_acked_missing[packet_no] = 1;
            m_tx_count -= 4;
            //if there's holes and not outstanding packets we need to
            //allow at least one packet in order to continue
            //transmission
            if(m_sent_pkts.empty())
                m_tx_count = ACE_MAX(m_tx_count, 1);
            else
                m_tx_count = ACE_MAX(m_tx_count, 0);
        }
        else
            ali->second++;
    }
//     MYTRACE(ACE_TEXT("Ack took %u, max packet index %d\n"), GETTIMESTAMP() - tm, max_packet_no);

    TTASSERT(m_sent_pkts.size() || m_tx_count>0);

    return true;
}

int DesktopTransmitter::GetRemainingBytes() const
{
    int size = 0;
    map_desktop_packets_t::const_iterator ii = m_sent_pkts.begin();
    for(;ii != m_sent_pkts.end();ii++)
        size += (*ii->second).GetPacketSize();
    ii = m_queued_pkts.begin();
    for(;ii != m_queued_pkts.end();ii++)
        size += (*ii->second).GetPacketSize();
    return size;
}

void DesktopTransmitter::GetNextDesktopPackets(desktoppackets_t& packets)
{
    while(!m_queued_pkts.empty() && m_tx_count>0)
    {
        packets.push_back(m_queued_pkts.begin()->second);
        AddSentDesktopPacket(*m_queued_pkts.begin()->second);

        TTASSERT(m_sent_pkts.find(m_queued_pkts.begin()->first) == m_sent_pkts.end());
        m_sent_pkts[m_queued_pkts.begin()->first] = m_queued_pkts.begin()->second;
        m_queued_pkts.erase(m_queued_pkts.begin());
        m_tx_count--;
    }
}

void DesktopTransmitter::GetSentDesktopPackets(desktoppackets_t& packets) const
{
    map_desktop_packets_t::const_iterator ii = m_sent_pkts.begin();
    for(;ii!=m_sent_pkts.end();ii++)
    {
        packets.push_back(ii->second);
    }
}

void DesktopTransmitter::GetDupAckLostDesktopPackets(desktoppackets_t& packets)
{
    //rtx dup-acks
    map_acked_missing_t::const_iterator ali = m_acked_missing.begin();
    while(ali != m_acked_missing.end() && m_tx_count>0)
    {
        map_sent_time_t::const_iterator sti = m_sent_times.find(ali->first);
        if(m_pingtime && sti != m_sent_times.end())
        {
            if(W32_GEQ(GETTIMESTAMP() - sti->second, m_pingtime * 2))
            {
                map_desktop_packets_t::const_iterator dpi = m_sent_pkts.find(ali->first);
                if(dpi != m_sent_pkts.end())
                {
                    packets.push_back(dpi->second);
                    m_tx_count--;
                    AddSentDesktopPacket(*dpi->second);

                    MYTRACE(ACE_TEXT("Desktop packet %d in session %d:%u DUP ack lost, tx_count is %d\n"),
                        dpi->first, GetSessionID(), GetUpdateID(), m_tx_count);
                }
            }
        }
        ali++;
    }
}

void DesktopTransmitter::GetLostDesktopPackets(const ACE_Time_Value& rtx_timeout,
                                               desktoppackets_t& packets, int count)
{
    uint32_t rtx_ms = rtx_timeout.msec();
    uint32_t cur_time = GETTIMESTAMP();
    map_sent_time_t::const_iterator ii;
    for(ii=m_sent_times.begin();ii != m_sent_times.end() && count-->0;ii++)
    {
        if(W32_GEQ(cur_time, ii->second + rtx_ms))
        {
            MYTRACE(ACE_TEXT("Desktop packet %d in session %d:%u lost by %d, tx_count is %d\n"),
                    ii->first, GetSessionID(), GetUpdateID(),
                    cur_time - ii->second, m_tx_count);
            map_desktop_packets_t::const_iterator dpi = m_sent_pkts.find(ii->first);
            TTASSERT(dpi != m_sent_pkts.end());
            if(dpi != m_sent_pkts.end())
            {
                packets.push_back(dpi->second);
                AddSentDesktopPacket(*dpi->second);
            }
        }
    }
//     MYTRACE(ACE_TEXT("Sent packets %d, queued packets %d, tx_count = %d\n"), 
//             m_sent_pkts.size(), m_queued_pkts.size(), m_tx_count);
    
    //'m_sent_pkts' is not filled unless a transmission is successful,
    //i.e. AddSentDesktopPacket() is called with the packet. So we
    //need to ensure there's at least one packet flowing to keep the
    //connection open.
    if(packets.empty() && m_sent_pkts.empty() && !m_queued_pkts.empty())
    {
        packets.push_back(m_queued_pkts.begin()->second);
        AddSentDesktopPacket(*m_queued_pkts.begin()->second);
    }
}

void GetPacketRanges(const std::set<uint16_t>& packet_indexes,
                     packet_range_t& pkt_index_ranges, 
                     std::set<uint16_t>& pkt_single_indexes)
{
    set<uint16_t>::const_iterator ri = packet_indexes.begin();
    while(ri != packet_indexes.end())
    {
        set<uint16_t>::const_iterator r_end = ri;
        uint16_t pkt_index = *r_end;
        r_end++;
        while(r_end != packet_indexes.end() && pkt_index+1 == *r_end)
        {
            r_end++;
            pkt_index++;
        }
        if(*ri != pkt_index)
            pkt_index_ranges[*ri] = pkt_index;
        else
            pkt_single_indexes.insert(*ri);

        ri = r_end;
    }
}

ACE_Time_Value GetDesktopPacketRTxTimeout(int udp_pingtime)
{
    ACE_Time_Value rtx_timeout(udp_pingtime / 1000, (udp_pingtime % 1000) * 1000);
    rtx_timeout *= 4; //RTX is set to 4 * round-trip
    if(rtx_timeout < DESKTOP_RTX_MIN_TIMEOUT)
        rtx_timeout = DESKTOP_RTX_MIN_TIMEOUT;
    return rtx_timeout;
}

DesktopNakTransmitter::DesktopNakTransmitter(uint8_t session_id, uint32_t upd_timeid)
: m_session_id(session_id)
, m_update_timeid(upd_timeid)
{
}

} //namespace teamtalk
