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

#include "DesktopCache.h"

#include <teamtalk/ttassert.h>

using namespace std;
using namespace teamtalk;

DesktopCache::DesktopCache(int src_userid, const DesktopWindow& wnd,
                           uint32_t initial_time)
: DesktopSession(wnd)
, m_current_desktop_time(initial_time)
, m_pending_update_time(initial_time)
, m_userid(src_userid)
{
}

bool DesktopCache::AddDesktopPacket(const DesktopPacket& packet)
{
    TTASSERT(packet.GetSessionID() == GetSessionID());
    TTASSERT(packet.GetPacketIndex() != DesktopPacket::INVALID_PACKET_INDEX);
    uint32_t packet_time = packet.GetTime();

//     MYTRACE(ACE_TEXT("Desktop packet index %d, upd %u\n"), 
//             packet.GetPacketIndex(), packet.GetTime());
    if(packet.GetSessionID() != GetSessionID() ||
       !W32_GEQ(packet_time, m_pending_update_time))
        return false;

    uint8_t session_id;
    uint16_t pkt_index, pkt_count;
    if(!packet.GetUpdateProperties(&session_id, &pkt_index, &pkt_count) &&
       !packet.GetSessionProperties(&session_id, NULL, NULL, NULL, &pkt_index, 
                                    &pkt_count))
       return false;

    TTASSERT(pkt_count);

//     MYTRACE(ACE_TEXT("Packet index %d/%d, upd %u\n"),
//             pkt_index, pkt_count, packet_time);

    //special case for initial packet
    if(m_pending_update_time == m_current_desktop_time && 
       m_expected_packets.empty())
       m_expected_packets.resize(pkt_count, false);

    //update how many packets are expected (if it's a new update)
    if(m_pending_update_time != packet_time)
    {
        TTASSERT(W32_GEQ(packet_time, m_pending_update_time));
        TTASSERT(GetMissingPacketsCount(m_pending_update_time)>=0);
        if(GetMissingPacketsCount(m_pending_update_time) > 0)
            return true; //don't start new update until previous has finished

        m_expected_packets.resize(pkt_count);
        m_expected_packets.assign(pkt_count, false);
        m_pending_update_time = packet_time;
        MYTRACE(ACE_TEXT("Starting new desktop update %d:%u for #%d\n"),
            GetSessionID(), packet_time, m_userid);
    }

    TTASSERT(m_expected_packets.size() == pkt_count); //TODO: what if incorrect?
    
    TTASSERT(pkt_index < m_expected_packets.size());
    if(pkt_index < m_expected_packets.size())
    {
        MYTRACE_COND(m_expected_packets[pkt_index], 
            ACE_TEXT("Desktop RTX for index %d user #%d update %d:%u\n"),
                     pkt_index, m_userid, GetSessionID(), GetPendingUpdateTime());
        if(m_expected_packets[pkt_index])
            return true; //this is a retransmission

        m_expected_packets[pkt_index] = true;
    }
    else
        return false;

    //store packet in list of updates
    map_desktop_updates_t::iterator dui = m_block_updates.find(packet_time);
    if(dui == m_block_updates.end())
    {
        m_block_updates[packet_time] = desktoppackets_t();
        dui = m_block_updates.find(packet_time);
    }
    desktoppackets_t& update_packets = dui->second;
    DesktopPacket* p;
    ACE_NEW_RETURN(p, DesktopPacket(packet), false);
    update_packets.push_back(desktoppacket_t(p));

    //check if all packets which are part of the update has arrived.
    if(update_packets.size() != pkt_count)
        return true;

    //erase the previous update
    if(W32_GT(packet_time, m_current_desktop_time))
        m_block_updates.erase(m_current_desktop_time);

    //update time which will be the new current view
    m_current_desktop_time = packet_time;
    TTASSERT(packet_time == m_pending_update_time);

    UpdateCurrentDesktopWindow(update_packets);

    LimitUpdateHistory(m_current_desktop_time, 100);

    return true;
}

int DesktopCache::GetMissingPacketsCount(uint32_t upd_time) const
{
    if(upd_time != m_pending_update_time)
        return -1;

    int count = 0;
    for(size_t i=0;i<m_expected_packets.size();i++)
    {
        if(!m_expected_packets[i])
            count++;
    }
    return count;
}

bool DesktopCache::GetMissingPackets(uint32_t upd_time, 
                                     std::set<uint16_t>& missing_packets) const
{
    if(upd_time != m_pending_update_time)
        return false;

    for(size_t i=0;i<m_expected_packets.size();i++)
    {
        if(!m_expected_packets[i])
            missing_packets.insert((uint16_t)i);
    }
    return true;
}

bool DesktopCache::GetReceivedPackets(uint32_t upd_time, 
                                     std::set<uint16_t>& recv_packets) const
{
    if(upd_time != m_pending_update_time)
        return false;

    for(size_t i=0;i<m_expected_packets.size();i++)
    {
        if(m_expected_packets[i])
            recv_packets.insert((uint16_t)i);
    }
    return true;
}

bool DesktopCache::GetDesktopPackets(uint32_t last_upd_time,
                                     uint16_t max_chunk_size,
                                     uint16_t max_payload_size,
                                     desktoppackets_t& packets) const
{
    map_updated_blocks_t::const_iterator ubi = m_updated_blocks.find(GetCurrentDesktopTime());
    TTASSERT(ubi != m_updated_blocks.end());
    if(ubi == m_updated_blocks.end())
        return false;

    map_updated_blocks_t::const_iterator ubi_last = m_updated_blocks.find(last_upd_time);

    desktoppackets_t new_packets;

    //get all the updated block between 'current upd time' and 'last_upd_time'
    if(last_upd_time != GetCurrentDesktopTime() &&
       ubi_last != m_updated_blocks.end())
    {
        set<uint16_t> blocks_updated;
        blocks_updated.insert(ubi->second.begin(), ubi->second.end());

        //run current-upd_time -> begin-iterator
        if(ubi != m_updated_blocks.begin())
        {
            do
            {
                ubi--;
                if(W32_GT(ubi->first, last_upd_time))
                {
                    //insert blocks updated in this update session
                    blocks_updated.insert(ubi->second.begin(), ubi->second.end());
                }
            }
            while(ubi != m_updated_blocks.begin());
        }

        //run end-iterator -> current-upd_time (reverse)
        ubi = m_updated_blocks.end();
        ubi--;
        while(ubi->first != last_upd_time)
        {
            if(W32_GT(ubi->first, last_upd_time))
            {
                //insert blocks updated in this update session
                blocks_updated.insert(ubi->second.begin(), ubi->second.end());
            }
            ubi--;
        }

        //process duplicate blocks
        map_dup_blocks_t dups;
        set<uint16_t> ignore_blocks;
        DuplicateBlocks(blocks_updated, m_block_crcs,
                        m_crc_blocks, dups, ignore_blocks);

        new_packets = BuildDesktopPackets(false, m_userid, 
                                          GetCurrentDesktopTime(),
                                          max_chunk_size, max_payload_size,
                                          this->GetDesktopWindow(),
                                          m_blocks, dups, &blocks_updated,
                                          &ignore_blocks);
    }
    else //build update containing all blocks
    {
        set<uint16_t> blocks_updated;
        map_blocks_t::const_iterator ii=m_blocks.begin();
        for(;ii!=m_blocks.end();ii++)
            blocks_updated.insert(ii->first);

        //process duplicate blocks
        map_dup_blocks_t dups;
        set<uint16_t> ignore_blocks;
        DuplicateBlocks(blocks_updated, m_block_crcs,
                        m_crc_blocks, dups, ignore_blocks);

        new_packets = BuildDesktopPackets(true, m_userid, 
                                          GetCurrentDesktopTime(), 
                                          max_chunk_size, max_payload_size,
                                          this->GetDesktopWindow(), 
                                          m_blocks, dups, NULL,
                                          &ignore_blocks);
    }

    desktoppackets_t::const_iterator ldi = new_packets.begin();
    for(;ldi != new_packets.end();ldi++)
    {
        (*ldi)->UpdatePacketCount((uint16_t)new_packets.size());
        packets.push_back(*ldi);
    }

    return true;
}

void DesktopCache::UpdateCurrentDesktopWindow(const desktoppackets_t& update_packets)
{
    set<uint16_t> upd_block_nums;

    map_block_t blocks;
    block_frags_t frags;

    map_desktoppacket_t block_fragments;

    desktoppackets_t::const_iterator dpi = update_packets.begin();
    while(dpi != update_packets.end())
    {
        //process blocks in packet
        (*dpi)->GetBlocks(blocks);
        map_block_t::iterator bi = blocks.begin();
        while(bi != blocks.end())
        {
            m_blocks[bi->first] = vector<char>(bi->second.block_data, 
                bi->second.block_data+bi->second.block_size);
            upd_block_nums.insert(bi->first);
            bi++;
        }
        blocks.clear();
        //process fragments in packet
        (*dpi)->GetBlockFragments(frags);
        block_frags_t::iterator fi = frags.begin();
        while(fi != frags.end())
        {
            map_desktoppacket_t::iterator bfi=block_fragments.find(fi->block_no);
            if(bfi != block_fragments.end())
            {
                bfi->second[fi->frag_no] = *dpi;
                upd_block_nums.insert(fi->block_no);
            }
            else
            {
                map_frag_desktoppacket_t frag_set;
                frag_set[fi->frag_no] = *dpi;
                block_fragments[fi->block_no] = frag_set;
            }
            fi++;
        }
        frags.clear();

        dpi++;
    }

    //reassemble fragments
    ReassembleDesktopBlocks(block_fragments, m_blocks);

    //MYTRACE(ACE_TEXT("Update %u\n"), m_current_desktop_time);
    //process duplicate blocks
    dpi = update_packets.begin();
    for(;dpi!=update_packets.end();dpi++)
    {
        map_dup_blocks_t dup_blocks;
        (*dpi)->GetDuplicateBlocks(dup_blocks);
        InsertDuplicateBlocks(dup_blocks, m_blocks, upd_block_nums);
    }
    //MYTRACE(ACE_TEXT("Ended %u\n"), m_current_desktop_time);

    //update CRC values for updated window
    UpdateBlocksCRC(m_blocks, upd_block_nums, m_block_crcs, m_crc_blocks);

    //update which blocks have been updated in this round
    TTASSERT(m_updated_blocks.find(m_current_desktop_time) == m_updated_blocks.end());
    m_updated_blocks[m_current_desktop_time] = upd_block_nums;

    TTASSERT(m_blocks.size() == (size_t)GetBlocksCount());
}

void DesktopCache::LimitUpdateHistory(uint32_t update_ref_time, int count)
{
    map_updated_blocks_t::iterator ii = m_updated_blocks.find(update_ref_time);
    TTASSERT(ii != m_updated_blocks.end());
    if(ii == m_updated_blocks.end() || m_updated_blocks.size() <= (size_t)count)
        return;

    int survive_count = 0;
    //erase updates 'm_updated_blocks.begin() -> update_ref_time'
    do
    {
        if(survive_count < count && W32_LEQ(ii->first, update_ref_time))
        {
            survive_count++;
            ii--;
        }
        else
        {
            if(ii == m_updated_blocks.begin())
            {
                m_updated_blocks.erase(ii);
                break;
            }
            else
                m_updated_blocks.erase(ii--);
        }
    }
    while(m_updated_blocks.size()>(size_t)count);

    //erase updates 'update_ref_time' -> m_updated_blocks.end()
    ii = m_updated_blocks.end();
    ii--;
    TTASSERT(!m_updated_blocks.empty());
    while(ii->first != update_ref_time && m_updated_blocks.size()>(size_t)count)
    {
        if(survive_count < count && W32_LT(ii->first, update_ref_time))
            survive_count++;
        else
            m_updated_blocks.erase(ii--);
    }
}


namespace teamtalk {

bool HasFragments(uint16_t blockno, const desktoppackets_t& packets)
{
    TTASSERT(!packets.empty());
    if(packets.empty())
        return false;

    map_block_t blocks;
    desktoppackets_t::const_iterator ii = packets.begin();
    if((*ii)->GetBlocks(blocks) && blocks.find(blockno) != blocks.end())
        return false;

    block_frags_t frags;
    (*ii)->GetBlockFragments(frags);
    block_frags_t::const_iterator bfi = frags.begin();
    while(bfi != frags.end())
    {
        if(bfi->block_no == blockno)
            return true;
        bfi++;
    }
    TTASSERT(0); //querying for a 'blockno' which doesn't exist in 'packets'

    return false;
}

bool InsertFragment(uint16_t blockno, uint8_t fragno,
                    const DesktopPacket& packet, desktoppackets_t& packets)
{
    desktoppackets_t::iterator ii = packets.begin();
    while(ii != packets.end())
    {
        assert(packet.GetTime() == (*ii)->GetTime());
        block_frags_t frags;
        (*ii)->GetBlockFragments(frags);
        assert(frags.size());
        block_frags_t::iterator bfi = frags.begin();
        while(bfi != frags.end())
        {
            if(bfi->block_no == blockno)
            {
                if(fragno < bfi->frag_no)
                {
                    DesktopPacket* p;
                    ACE_NEW_NORETURN(p, DesktopPacket(packet));
                    if(p)
                    {
                        packets.insert(ii, desktoppacket_t(p));
                        assert(packets.size() <= bfi->frag_cnt);
                        return true;
                    }
                }
            }
            bfi++;
        }

        ii++;
    }
    return false;
}

bool BlockComplete(uint16_t blockno, const desktoppackets_t& packets)
{
    if(packets.empty())
        return false;

    block_frags_t frags;
    (*packets.begin())->GetBlockFragments(frags);
    assert(frags.size());
    block_frags_t::const_iterator ii=frags.begin();
    while(ii != frags.end())
    {
        if(ii->block_no == blockno && packets.size() == ii->frag_cnt)
            return true;
        ii++;
    }
    return false;
}

bool GetMissingFragments(uint16_t blockno, const desktoppackets_t& packets,
                         std::set<uint8_t>& missing_fragnums)
{
    TTASSERT(packets.size());
    if(packets.empty())
        return false;

    std::set<uint8_t> recv_fragnums;
    uint8_t frag_count = 0;
    desktoppackets_t::const_iterator dpi = packets.begin();
    while(dpi != packets.end())
    {
        block_frags_t frags;
        (*dpi)->GetBlockFragments(frags);
        TTASSERT(frags.size());
        block_frags_t::const_iterator ii=frags.begin();
        while(ii != frags.end())
        {
            if(ii->block_no == blockno)
            {
                recv_fragnums.insert(ii->frag_no);
                frag_count = ii->frag_cnt;
            }
            ii++;
        }
        dpi++;
    }
    for(uint8_t i=0;i<frag_count;i++)
    {
        if(recv_fragnums.find(i) == recv_fragnums.end())
            missing_fragnums.insert(i);
    }
    return missing_fragnums.size();
}

}
