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

#ifndef DESKTOPCACHE_H
#define DESKTOPCACHE_H

#include <ace/Bound_Ptr.h> 
#include <ace/Null_Mutex.h> 

#include <teamtalk/DesktopSession.h>
#include <teamtalk/PacketHelper.h>

namespace teamtalk {

    //updateid (time) -> packets
    typedef std::map<uint32_t, desktoppackets_t> map_desktop_updates_t;
    //updateid (time) -> block nums
    typedef std::map<uint32_t, std::set<uint16_t> > map_updated_blocks_t;

    class DesktopCache : public DesktopSession
    {
    public:
        DesktopCache(int src_userid, const DesktopWindow& wnd, 
                     uint32_t initial_time);

        bool AddDesktopPacket(const DesktopPacket& packet);

        uint32_t GetCurrentDesktopTime() const { return m_current_desktop_time; }
        uint32_t GetPendingUpdateTime() const { return m_pending_update_time; }
        
        int GetMissingPacketsCount(uint32_t upd_time) const;
        bool GetMissingPackets(uint32_t upd_time,
                               std::set<uint16_t>& missing_packets) const;
        bool GetReceivedPackets(uint32_t upd_time, 
                                std::set<uint16_t>& recv_packets) const;

        bool GetDesktopPackets(uint32_t last_upd_time,
                               uint16_t max_chunk_size,
                               uint16_t max_payload_size,
                               desktoppackets_t& packets) const;

        bool IsReady() const { return !m_blocks.empty(); }
    private:
        void UpdateCurrentDesktopWindow(const desktoppackets_t& update_packets);
        void LimitUpdateHistory(uint32_t update_ref_time, int count);
        //container of the packets which were part of an update
        //(for now this could actually be a vector, since only the latest 
        //update matters).
        map_desktop_updates_t m_block_updates;
        //the number of packets which are expected in current update cycle.
        std::vector<bool> m_expected_packets;
        //the blocks which were updated in an view update
        map_updated_blocks_t m_updated_blocks;
        //time of the current desktop window (update time)
        uint32_t m_current_desktop_time;
        //time of the update which is currently pending (can be the same as 
        //'m_current_desktop_time' if it's the initial update or if the 
        //current desktop has completed).
        uint32_t m_pending_update_time;
        //container of the current desktop window (all blocks)
        map_blocks_t m_blocks;
        //blocks' crc value
        map_block_crc_t m_block_crcs;
        //crc value for blocks
        map_crc_blocks_t m_crc_blocks;
        //owner user id
        int m_userid;
    };

    typedef ACE_Strong_Bound_Ptr< DesktopCache, ACE_Null_Mutex > desktop_cache_t;

    //blockno -> frags
    typedef std::map<uint16_t, std::set<uint8_t> > map_missing_frags_t;

    bool HasFragments(uint16_t blockno, const desktoppackets_t& packets);
    bool InsertFragment(uint16_t blockno, uint8_t fragno,
                        const DesktopPacket& packet,
                        desktoppackets_t& packets);
    bool BlockComplete(uint16_t blockno, const desktoppackets_t& packets);
    bool GetMissingFragments(uint16_t blockno, const desktoppackets_t& packets,
                             std::set<uint8_t>& missing_fragnums);
}

#endif
