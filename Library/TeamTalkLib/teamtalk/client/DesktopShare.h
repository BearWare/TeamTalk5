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

#ifndef DESKTOPSHARE_H
#define DESKTOPSHARE_H

#include "teamtalk/Common.h"
#include "teamtalk/DesktopSession.h"
#include "teamtalk/PacketHelper.h"

#include <ace/SString.h>
#include <ace/Task.h>

#include <cstdint>
#include <memory>
#include <set>
#include <vector>

namespace teamtalk {

    class DesktopInitiator 
        : public DesktopSession
        , public ACE_Task_Base
    {
    public:
        DesktopInitiator(int userid, const DesktopWindow& wnd,
                         uint16_t max_chunk_size, uint16_t max_payload_size);
        ~DesktopInitiator() override;
        int NewBitmap(const char* bmp_bits, int size, uint32_t tm);

        void Abort();

        void GetDesktopPackets(desktoppackets_t& packets);
        bool HasDesktopPackets() const { return !m_desktop_packets.empty(); }

        int svc() override;

    private:
        void CompressDirtyBlocks(map_blocks_t& blocks);
        bool CompressBlock(int block_no, std::vector<char>& outbuf);

        //the grid of blocks
        map_blocks_t m_blocks;
        //tmp buffer
        std::vector<char> m_tmp_block;
        //the blocknums which became dirty by last call to NewBitmap()
        std::set<uint16_t> m_dirty_blocknums;
        //blocks' crc value
        map_block_crc_t m_block_crcs;
        //crc value for blocks
        map_crc_blocks_t m_crc_blocks;
        //timestamp of dirty blocks
        uint32_t m_timestamp = 0;
        //generated desktop packets from new bitmap
        desktoppackets_t m_desktop_packets;
        //true when a new session is started
        bool m_newsession = true;
        //my userid
        int m_userid = 0;
        //abort thread
        bool m_abort = false;
        //max packet data size
        uint16_t m_max_chunk_size = 0, m_max_payload_size = 0;
    };

    using desktop_initiator_t = std::shared_ptr< DesktopInitiator >;

    class DesktopViewer : public DesktopSession
    {
    public:
        DesktopViewer(const DesktopWindow& wnd);

        void AddCompressedBlock(int block_no, const char* inbuf, int in_size);
        void AddDuplicateBlock(int src_block_no, int dest_block_no);

        void ResetBitmap(const std::vector<char>* bmp = nullptr);

        void WriteBitmapToFile(const ACE_TString& filename);

        const char* GetBitmap(int* size = nullptr) const;

    private:
        static bool DecompressBlock(const char* inbuf, int in_size, 
                             std::vector<char>& outbuf);
        std::vector<char> m_bitmap;
    };

    using desktop_viewer_t = std::shared_ptr< DesktopViewer >;
} // namespace teamtalk
#endif
