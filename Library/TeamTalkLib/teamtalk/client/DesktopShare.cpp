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

#include "DesktopShare.h"

#include <teamtalk/ttassert.h>
#include <codec/BmpFile.h>

#include <ace/FILE_IO.h>
#include <ace/FILE_Connector.h>
#include <ace/FILE_Addr.h>

#include <zlib.h>

using namespace std;
using namespace teamtalk;

#define DEFAULT_COLOR 127

DesktopInitiator::DesktopInitiator(int userid, const DesktopWindow& wnd,
                                   uint16_t max_chunk_size, 
                                   uint16_t max_payload_size)
: DesktopSession(wnd)
, m_userid(userid)
, m_newsession(true)
, m_abort(false)
, m_max_chunk_size(max_chunk_size)
, m_max_payload_size(max_payload_size)
{
}

DesktopInitiator::~DesktopInitiator()
{
    TTASSERT(this->thr_count() == 0);
    MYTRACE(ACE_TEXT("DesktopInitiator::~DesktopInitiator()\n"));
}

int DesktopInitiator::NewBitmap(const char* bmp_bits, int size, uint32_t tm)
{
    TTASSERT(this->thr_count() == 0);

    //ACE_UINT32 start_tm = GETTIMESTAMP();

    //don't allow new bitmap when thread is working
    if(this->thr_count())
        return -1;
    //don't allow new bitmap if there's dirty data
    if(m_desktop_packets.size())
        return -1;

    TTASSERT(size == GetBitmapSize());
    if(size != GetBitmapSize())
        return -1;

    TTASSERT(m_dirty_blocknums.empty());

    for(int h=0;h<m_h_blocks;h++)
    {
        int height = (h == m_h_blocks-1 && (GetHeight() % m_block_height))? GetHeight() % m_block_height : m_block_height;

        for(int w=0;w<m_w_blocks;w++)
        {
            int width = (w == m_w_blocks-1 && (GetWidth() % m_block_width))? GetWidth() % m_block_width : m_block_width;

            if((int)m_tmp_block.size() != width * height * m_pixel_size)
            {
                char def_color = DEFAULT_COLOR;
                m_tmp_block.resize(width * height * m_pixel_size, def_color);
            }

            for(int i=0;i<height;i++)
            {
                int pixel_x = w * m_block_width;
                int pixel_y = h * m_block_height + i;
                TTASSERT(pixel_x < GetWidth());
                TTASSERT(pixel_y < GetHeight());

                int byte_pos = (pixel_x + pixel_y * GetWidth()) * m_pixel_size;
                byte_pos += GetHeight() * m_padding;
                TTASSERT(byte_pos < size);
                const char* byte_pos_ptr = &bmp_bits[byte_pos];
                memcpy(&m_tmp_block[width*i*m_pixel_size], byte_pos_ptr, width * m_pixel_size);
            }

            const int BLOCK_INDEX = w + h * m_w_blocks;

            ACE_UINT32 crc = ACE::crc32(&m_tmp_block[0], m_tmp_block.size());
            
            //only replace if it's different
            map_blocks_t::iterator ii = m_blocks.find(BLOCK_INDEX);
            if(ii == m_blocks.end() || crc != m_block_crcs[BLOCK_INDEX]/*ii->second != m_tmp_block*/)
            {
                m_blocks[BLOCK_INDEX].swap(m_tmp_block);
                m_dirty_blocknums.insert(BLOCK_INDEX);
            }
        }
    }
    TTASSERT(m_w_blocks*m_h_blocks == (int)m_blocks.size());
    TTASSERT(m_abort == false);

    //MYTRACE(ACE_TEXT("Build packets thread new bmp done %u, duration %u. Dirty blocks: %u\n"), 
    //        tm, GETTIMESTAMP() - start_tm, m_dirty_blocknums.size());

    int n_dirty = int(m_dirty_blocknums.size());
    if(n_dirty)
    {
        uint32_t tmp = m_timestamp;
        m_timestamp = tm;
        
        // ensure thread is shut down properly before activating next one
        this->wait();

        if(this->activate() < 0)
        {
            m_timestamp = tmp;
            return -1;
        }
    }
    return n_dirty;
}

void DesktopInitiator::Abort()
{
    m_abort = true;
}

void DesktopInitiator::GetDesktopPackets(desktoppackets_t& packets)
{
    //ensure thread has exited from
    TTASSERT(this->thr_count() == 0);
    TTASSERT(m_desktop_packets.size());
    TTASSERT(!m_abort);
    if(m_abort)
        return;

    packets = m_desktop_packets;
    m_desktop_packets.clear();
}

int DesktopInitiator::svc(void)
{
    TTASSERT(m_dirty_blocknums.size());
    TTASSERT(m_desktop_packets.empty());

    //compressed dirty blocks
    map_blocks_t dirty_blocks;

    CompressDirtyBlocks(dirty_blocks);
    if(m_abort)
        return 0;

    //update CRC values
    UpdateBlocksCRC(m_blocks, m_dirty_blocknums, m_block_crcs, m_crc_blocks);

    //process duplicate blocks
    map_dup_blocks_t dups;
    set<uint16_t> ignore_blocks;
    DuplicateBlocks(m_dirty_blocknums, m_block_crcs,
                    m_crc_blocks, dups, ignore_blocks);
    
    m_dirty_blocknums.clear();

    m_desktop_packets = BuildDesktopPackets(m_newsession, m_userid, 
                                            m_timestamp, m_max_chunk_size,
                                            m_max_payload_size, 
                                            GetDesktopWindow(),
                                            dirty_blocks, dups, NULL, &ignore_blocks);
    m_newsession = false;
    TTASSERT(m_desktop_packets.size());
    return 0;
}

void DesktopInitiator::CompressDirtyBlocks(map_blocks_t& blocks)
{
    std::set<uint16_t>::iterator ii=m_dirty_blocknums.begin();
    while(ii != m_dirty_blocknums.end() && !m_abort)
    {
        blocks[*ii] = vector<char>(BLOCK_MAX_BYTESIZE);
        if(!CompressBlock(*ii, blocks[*ii]))
            blocks.erase(*ii);
        ii++;
    }
}

bool DesktopInitiator::CompressBlock(int block_no, std::vector<char>& outbuf)
{
    map_blocks_t::iterator ii = m_blocks.find(block_no);
    if(ii == m_blocks.end())
        return false;

    int ret;
    bool success;

    z_stream strm = {Z_NULL};

    //Z_BEST_COMPRESSION
    //Z_DEFAULT_COMPRESSION
    //Z_BEST_SPEED
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);

    assert(ret == Z_OK);

    strm.avail_in = (uInt)ii->second.size();
    strm.next_in = reinterpret_cast<Bytef*>(&ii->second[0]);

    strm.avail_out = (uInt)outbuf.size();
    strm.next_out = reinterpret_cast<Bytef*>(&outbuf[0]);

    ret = deflate(&strm, Z_FINISH);
    assert(ret == Z_STREAM_END);

    success = ret == Z_STREAM_END;

    std::vector<char> tmp = outbuf;

    if(success)
        outbuf.resize(outbuf.size() - strm.avail_out);

    ret = deflateEnd(&strm);
    assert(ret == Z_OK);

    return success;
}



DesktopViewer::DesktopViewer(const DesktopWindow& wnd)
: DesktopSession(wnd)
{
    char gray = DEFAULT_COLOR;
    m_bitmap.assign(GetBitmapSize(), gray);
}

void DesktopViewer::AddCompressedBlock(int block_no, const char* inbuf, int in_size)
{
    assert(block_no < m_w_blocks * m_h_blocks);
    if(block_no >= m_w_blocks * m_h_blocks)
        return;

    std::vector<char> outbuf(BLOCK_MAX_BYTESIZE);
    if(!DecompressBlock(inbuf, in_size, outbuf))
    {
        assert(0);
        return;
    }

    int h = block_no / m_w_blocks;
    int w = block_no - h * m_w_blocks;

    assert(h < m_h_blocks);
    assert(w < m_w_blocks);

    int height = (h == m_h_blocks-1 && (GetHeight() % m_block_height))? 
        GetHeight() % m_block_height : m_block_height;
    int width = (w == m_w_blocks-1 && (GetWidth() % m_block_width))? 
        GetWidth() % m_block_width : m_block_width;

    for(int i=0;i<height;i++)
    {
        int pixel_x = w * m_block_width;
        int pixel_y = h * m_block_height + i;

        int byte_pos = (pixel_x + pixel_y * GetWidth()) * m_pixel_size;
        byte_pos += GetHeight() * m_padding;
        TTASSERT(byte_pos < (int)m_bitmap.size());
        char* byte_pos_ptr = &m_bitmap[byte_pos];
        memcpy(byte_pos_ptr, &outbuf[width*i*m_pixel_size], width * m_pixel_size);
    }
}

void DesktopViewer::AddDuplicateBlock(int src_block_no, int dest_block_no)
{
    int src_h = src_block_no / m_w_blocks;
    int src_w = src_block_no - src_h * m_w_blocks;

    assert(src_h < m_h_blocks);
    assert(src_w < m_w_blocks);

    int dest_h = dest_block_no / m_w_blocks;
    int dest_w = dest_block_no - dest_h * m_w_blocks;

    assert(dest_h < m_h_blocks);
    assert(dest_w < m_w_blocks);

    int height = (src_h == m_h_blocks-1 && (GetHeight() % m_block_height))? 
        GetHeight() % m_block_height : m_block_height;
    int width = (src_w == m_w_blocks-1 && (GetWidth() % m_block_width))? 
        GetWidth() % m_block_width : m_block_width;

    for(int i=0;i<height;i++)
    {
        int src_pixel_x = src_w * m_block_width;
        int src_pixel_y = src_h * m_block_height + i;

        int dest_pixel_x = dest_w * m_block_width;
        int dest_pixel_y = dest_h * m_block_height + i;

        int src_byte_pos = (src_pixel_x + src_pixel_y * GetWidth()) * m_pixel_size;
        int dest_byte_pos = (dest_pixel_x + dest_pixel_y * GetWidth()) * m_pixel_size;
        src_byte_pos += GetHeight() * m_padding;
        dest_byte_pos += GetHeight() * m_padding;
        TTASSERT(src_byte_pos < (int)m_bitmap.size());
        TTASSERT(dest_byte_pos < (int)m_bitmap.size());
        char* src_byte_pos_ptr = &m_bitmap[src_byte_pos];
        char* dest_byte_pos_ptr = &m_bitmap[dest_byte_pos];
        memcpy(dest_byte_pos_ptr, src_byte_pos_ptr, width * m_pixel_size);
    }
}

void DesktopViewer::ResetBitmap(const std::vector<char>* bmp/* = 0*/)
{
    if(bmp && bmp->size() == m_bitmap.size())
        m_bitmap =  *bmp;
    else
    {
        char def_color = DEFAULT_COLOR;
        m_bitmap.assign(m_bitmap.size(), def_color);
    }
}

bool DesktopViewer::DecompressBlock(const char* inbuf,
                                    int in_size, 
                                    std::vector<char>& outbuf)
{
    int ret;
    bool success = false;
    z_stream strm = {Z_NULL};

    ret = inflateInit(&strm);
    assert(ret == Z_OK);
    if(ret != Z_OK)
        return false;

    strm.avail_in = (uInt)in_size;
    strm.total_in = (uInt)in_size;
    strm.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(inbuf));
    strm.avail_out = (uInt)outbuf.size();
    strm.next_out = reinterpret_cast<Bytef*>(&outbuf[0]);

    ret = inflate(&strm, Z_FINISH);
    assert(ret == Z_STREAM_END);
    success = ret == Z_STREAM_END;

    if(success)
        outbuf.resize(outbuf.size() - strm.avail_out);

    ret = inflateEnd(&strm);
    assert(ret == Z_OK);

    return success;
}

void DesktopViewer::WriteBitmapToFile(const ACE_TString& filename)
{
    WriteBitmap(filename, GetWidth(), GetHeight(), m_pixel_size,
                &m_bitmap[0], int(m_bitmap.size()));
}

const char* DesktopViewer::GetBitmap(int* size/* = 0*/) const
{
    if(size)
        *size = (int)m_bitmap.size();
    return &m_bitmap[0];
}
