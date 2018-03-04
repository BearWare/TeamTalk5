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

#ifndef CODECHELPER_H
#define CODECHELPER_H

#include <vector>
#include <list>
#include <map>

#include <teamtalk/PacketLayout.h>
#include <teamtalk/CodecCommon.h>
#include <teamtalk/Common.h>

#include "DesktopSession.h"

namespace teamtalk {

    /** Audio packets **/
    typedef std::vector<audiopacket_t> audiopackets_t;
    //fragno -> AudioPacket
    typedef std::map<uint8_t, audiopacket_t> audiofragments_t;

    audiopackets_t BuildAudioPackets(uint16_t src_userid,
                                     uint32_t time,
                                     uint8_t streamid,
                                     uint16_t packet_no,
                                     const char* enc_data,
                                     uint16_t enc_length,
                                     const std::vector<int>* enc_frame_sizes);

    audiopackets_t BuildAudioFragments(const AudioPacket& in_packet,
                                       uint16_t max_chunk_size);
    audiopacket_t ReassembleAudioPacket(const audiofragments_t& fragments,
                                        const AudioCodec& codec);
    
    std::vector<uint16_t> GetAudioPacketFrameSizes(const AudioPacket& packet,
                                                   const AudioCodec& codec);

    /* Video packets */
    typedef std::vector<VideoPacket*> videopackets_t;
    videopackets_t BuildVideoPackets(uint8_t kind,
                                     uint16_t src_userid,
                                     uint32_t time,
                                     uint16_t max_chunk_size,
                                     uint8_t streamid,
                                     uint32_t packet_no,
                                     uint16_t* width,
                                     uint16_t* height,
                                     const char* enc_data,
                                     uint32_t enc_len);

    //fragno -> VideoPacket
    typedef std::map<uint16_t, videopacket_t> video_fragments_t;
    bool ReassembleVideoPackets(const video_fragments_t& fragments,
                                const VideoPacket& packet,
                                std::vector<char>& enc_frame);

    /** Desktop packets **/
    typedef std::map< uint16_t, std::vector<char> > map_blocks_t;
    typedef std::list< desktoppacket_t > desktoppackets_t;

    //blockno -> crc32
    typedef std::map< uint16_t, uint32_t > map_block_crc_t;
    //crc32 -> set(block_nums)
    typedef std::map< uint32_t, std::set<uint16_t> > map_crc_blocks_t;

    void UpdateBlocksCRC(const map_blocks_t& blocks,
                         const std::set<uint16_t>& dirty_blocks,
                         map_block_crc_t& block_crcs,
                         map_crc_blocks_t& crc_blocks);

    void DuplicateBlocks(const std::set<uint16_t>& dirty_blocks,
                         const map_block_crc_t& block_crcs,
                         const map_crc_blocks_t& crc_blocks,
                         map_dup_blocks_t& dup_blocks,
                         std::set<uint16_t>& ignore_blocks);

    void InsertDuplicateBlocks(const map_dup_blocks_t& dup_blocks,
                               map_blocks_t& blocks, 
                               std::set<uint16_t>& updated_blocks);
                               
                         
    bool ExtractBlockRange(const std::set<uint16_t>& blocknums, 
                           std::set<uint16_t>& result_range); 

    desktoppackets_t BuildDesktopPackets(bool new_session,
                                         uint16_t src_userid,
                                         uint32_t time,
                                         uint16_t max_chunk_size,
                                         uint16_t max_payload_size,
                                         const DesktopWindow& dwnd,
                                         const map_blocks_t& blocks,
                                         const map_dup_blocks_t& dup_blocks,
                                         const std::set<uint16_t>* inc_blocks = NULL,
                                         const std::set<uint16_t>* ignore_blocks = NULL);

    //select desktop packets for next transmission
    int SelectDesktopBlocks(bool initial_desktoppacket, 
                            map_block_t& send_blocks,
                            block_frags_t& send_frags,
                            map_dup_blocks_t& send_dup_blocks,
                            map_block_t& packet_blocks,
                            block_frags_t& packet_frags,
                            mmap_dup_blocks_t& packet_dup_blocks,
                            uint16_t max_payload_size);

    //fragno -> desktoppacket_t
    typedef std::map<uint8_t, desktoppacket_t> map_frag_desktoppacket_t;
    //blockno -> fragments
    typedef std::map<uint16_t, map_frag_desktoppacket_t> map_desktoppacket_t;
    //reassemble blocks from fragments
    void ReassembleDesktopBlocks(map_desktoppacket_t& frag_packets,
                                 map_blocks_t& blocks);
    //build set of packets which have been acked
    bool GetAckedDesktopPackets(uint8_t session_id, uint32_t update_time, 
                                const desktoppackets_t& packets, 
                                std::set<uint16_t>& recv_packets);
    //Remove packets which are not of the same session and update-id
    int RemoveObsoleteDesktopPackets(const DesktopPacket& packet,
                                     desktoppackets_t& packets);

    class DesktopTransmitter
    {
    public:
        DesktopTransmitter(uint8_t session_id, uint32_t upd_timeid);

        void AddDesktopPacketToQueue(desktoppacket_t& packet);
        bool IsDesktopPacketAcked(uint16_t packet_no) const;

        bool ProcessDesktopAckPacket(const DesktopAckPacket& ack_packet);

        int GetPacketSentSize() const { return (int)m_sent_pkts.size(); }

        int GetPacketQueueSize() const { return (int)m_queued_pkts.size(); }

        int GetRemainingPacketsCount() const { return GetPacketSentSize() + GetPacketQueueSize(); }

        int GetRemainingBytes() const;

        bool Done() const { return GetPacketSentSize() == 0 && GetPacketQueueSize() == 0; }

        void GetNextDesktopPackets(desktoppackets_t& packets);

        void GetSentDesktopPackets(desktoppackets_t& packets) const;

        void GetDupAckLostDesktopPackets(desktoppackets_t& packets);

        void GetLostDesktopPackets(const ACE_Time_Value& rtx_timeout,
                                   desktoppackets_t& packets, int count);

        uint8_t GetSessionID() const { return m_session_id; }
        uint32_t GetUpdateID() const { return m_update_timeid; }

    private:
        void AddSentDesktopPacket(const DesktopPacket& packet);

        uint8_t m_session_id;
        uint32_t m_update_timeid;

        //packet id -> packet
        typedef std::map<uint16_t, desktoppacket_t> map_desktop_packets_t;
        //packets 'on the wire'
        map_desktop_packets_t m_sent_pkts;
        //packets queued for transmission (not yet sent)
        map_desktop_packets_t m_queued_pkts;
        //packet id -> acked lost count
        typedef std::map<uint16_t, int> map_acked_missing_t;
        map_acked_missing_t m_acked_missing;
        //packet id -> sent time
        typedef std::map<uint16_t, uint32_t> map_sent_time_t;
        map_sent_time_t m_sent_times, m_sent_ack_times;
        int m_tx_count;
        //round trip time (sent -> ack time)
        uint32_t m_pingtime;
    };

    typedef ACE_Strong_Bound_Ptr< DesktopTransmitter, ACE_Null_Mutex > desktop_transmitter_t;

    class DesktopNakTransmitter
    {
    public:
        DesktopNakTransmitter(uint8_t session_id, uint32_t upd_timeid);

        uint8_t GetSessionID() const { return m_session_id; }
        uint32_t GetUpdateID() const { return m_update_timeid; }

    private:
        uint8_t m_session_id;
        uint32_t m_update_timeid;
    };

    typedef ACE_Strong_Bound_Ptr< DesktopNakTransmitter, ACE_Null_Mutex > desktop_nak_tx_t;

    void GetPacketRanges(const std::set<uint16_t>& packet_indexes,
                         packet_range_t& pkt_index_ranges, 
                         std::set<uint16_t>& pkt_single_indexes);

    ACE_Time_Value GetDesktopPacketRTxTimeout(int udp_pingtime);

}
#endif
