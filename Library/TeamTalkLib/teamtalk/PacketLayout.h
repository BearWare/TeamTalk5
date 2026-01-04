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

#if !defined(PACKETLAYOUT_H)
#define PACKETLAYOUT_H

#include "myace/MyACE.h"

#include <ace/ACE.h>

#if defined(ENABLE_ENCRYPTION)
#include <openssl/evp.h>
#include <openssl/aes.h>
#endif

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

/******************************
*    TEAMTALK PACKET LAYOUT
*******************************/

constexpr auto TEAMTALK_PACKET_PROTOCOL = 1;

constexpr auto TEAMTALK_DEFAULT_PACKET_PROTOCOL = 1;

constexpr auto FIELDHEADER_PAYLOAD = 50;

constexpr auto MIN_PAYLOAD_DATA_SIZE = 400;    //The raw data must be split in at most this size;
constexpr auto MIN_PACKET_PAYLOAD_SIZE = MIN_PAYLOAD_DATA_SIZE + FIELDHEADER_PAYLOAD;   //The maximum size of fields and raw data

constexpr auto MAX_PAYLOAD_DATA_SIZE = 1250;   //The raw data must be split in at most this size;
constexpr auto MAX_PACKET_PAYLOAD_SIZE = MAX_PAYLOAD_DATA_SIZE + FIELDHEADER_PAYLOAD;   //The maximum size of fields and raw data

static constexpr uint16_t MTU_QUERY_SIZES[] = {MIN_PAYLOAD_DATA_SIZE, 800, 1000, MAX_PAYLOAD_DATA_SIZE};
constexpr auto MTU_QUERY_SIZES_COUNT = sizeof(MTU_QUERY_SIZES) / sizeof(MTU_QUERY_SIZES[0]);

constexpr auto MAX_PACKET_SIZE = 1350;           //The maximum size of a packet

namespace teamtalk {


#ifdef ACE_BIG_ENDIAN
#pragma error "Big endian not supported"
#endif

template<typename T1, typename T2>
constexpr uint8_t* SET_UINT4_PTR(uint8_t* buf, T1 val1, T2 val2)
{
    assert(val1 <= 0xF);
    assert(val2 <= 0xF);
    buf[0] = (val1 & 0xF) | ((val2 & 0xF) << 4);
    return buf += 1;
}

template<typename T1, typename T2>
constexpr void SET_UINT4(uint8_t* buf, T1 val1, T2 val2)
{
    set_uint4_ptr(buf, val1, val2);
}

template<typename T1, typename T2>
constexpr const uint8_t* GET_UINT4_PTR(const uint8_t* buf, T1& val1, T2& val2)
{
    val1 = (buf[0] & 0xF);
    val2 = (buf[0] >> 4);
    return buf += 1;
}

template<typename T1>
constexpr uint8_t* SET_UINT8_PTR(uint8_t* buf, T1 val)
{
    *buf = val;
    return buf += 1;
}

template<typename T1>
constexpr void SET_UINT8(uint8_t* buf, T1 val)
{
    set_uint8_ptr(buf, val);
}

template<typename T1>
constexpr const uint8_t* GET_UINT8_PTR(const uint8_t* buf, T1& val)
{
    val = *buf;
    return buf += 1;
}

constexpr auto GET_UINT8(const uint8_t* buf)
{
    uint8_t val = 0;
    GET_UINT8_PTR(buf, val);
    return val;
}

template<typename T1>
constexpr uint8_t* SET_UINT12_PTR(uint8_t* buf, T1 val1)
{
    assert(val1 <= 0xFFF);
    buf[0] = static_cast<char>(val1 & 0xFF);
    buf[1] = static_cast<char>((val1 >> 8) & 0xF);
    return buf += 2;
}

template<typename T1>
constexpr void SET_UINT12(uint8_t* buf, T1 val1)
{
    SET_UINT12_PTR(buf, val1);
}

template<typename T1, typename T2>
constexpr uint8_t* SET2_UINT12_PTR(uint8_t* buf, T1 val1, T2 val2)
{
    assert(val1 <= 0xFFF);
    assert(val2 <= 0xFFF);
    buf[0] = static_cast<char>(val1 & 0xFF);
    buf[1] = static_cast<char>((val1 >> 8) & 0xF);
    buf[1] |= static_cast<char>((val2 & 0xF) << 4);
    buf[2] = static_cast<char>((val2 >> 4) & 0xFF);
    return buf += 3;
}

template<typename T1, typename T2>
constexpr void SET2_UINT12(uint8_t* buf, T1 val1, T2 val2)
{
    SET2_UINT12_PTR(buf, val1, val2);
}

template<typename T1>
constexpr const uint8_t* GET_UINT12_PTR(const uint8_t* buf, T1& val1)
{
    val1 = buf[0] | ((buf[1] & 0xF) << 8);
    return buf += 2;
}

template<typename T1>
constexpr T1 GET_UINT12(const uint8_t* buf, T1& val1)
{
    GET_UINT12_PTR(buf, val1);
    return val1;
}

template<typename T1, typename T2>
constexpr const uint8_t* GET2_UINT12_PTR(const uint8_t* buf, T1& val1, T2& val2)
{
    val1 = (buf[0] | ((buf[1] & 0xF) << 8));
    val2 = ((buf[1] >> 4) | (buf[2] << 4));
    return buf += 3;
}

template<typename T1, typename T2>
constexpr void GET2_UINT12(const uint8_t* buf, T1& val1, T2& val2)
{
    GET2_UINT12_PTR(buf, val1, val2);
}

template<typename T1, typename T2>
constexpr uint8_t* SET_UINT12_UINT4_PTR(uint8_t* buf, T1 val1, T2 val2)
{
    assert(val1 <= 0xFFF);
    assert(val2 <= 0xF);
    buf[0] = static_cast<char>(val1 & 0xFF);
    buf[1] = static_cast<char>((val1 >> 8) & 0xF);
    buf[1] |= static_cast<char>((val2 & 0xF) << 4);
    return buf += 2;
}

template<typename T1, typename T2>
constexpr void SET_UINT12_UINT4(uint8_t* buf, T1 val1, T2 val2)
{
    SET_UINT12_UINT4_PTR(buf, val1, val2);
}

template<typename T1, typename T2>
constexpr const uint8_t* GET_UINT12_UINT4_PTR(const uint8_t* buf, T1& val1, T2& val2)
{
    val1 = buf[0] | ((buf[1] & 0xF) << 8);
    val2 = buf[1] >> 4;
    return buf += 2;
}

template<typename T1, typename T2>
constexpr void GET_UINT12_UINT4(const uint8_t* buf, T1& val1, T2& val2)
{
    get_uint12_uint4_ptr(buf, val1, val2);
}

template<typename T1>
constexpr uint8_t* SET_UINT16_PTR(uint8_t* buf, T1 val)
{
    assert((int)val <= 0xFFFF);
    buf[0] = static_cast<char>(val & 0xFF);
    buf[1] = static_cast<char>(val >> 8);
    return buf += 2;
}

template<typename T1>
constexpr void SET_UINT16(uint8_t* buf, T1 val)
{
    SET_UINT16_PTR(buf, val);
}

constexpr const uint8_t* GET_UINT16_PTR(const uint8_t* buf, uint16_t& val)
{
    val = (static_cast<uint16_t>(buf[1]) << 8) |
          static_cast<uint16_t>(buf[0]);
    return buf += 2;
}

constexpr uint16_t GET_UINT16(const uint8_t* buf)
{
    uint16_t val = 0;
    GET_UINT16_PTR(buf, val);
    return val;
}

template<typename T1, typename T2>
constexpr const uint8_t* GET_UINT16_UINT8_PTR(const uint8_t* buf, T1& val16, T2& val8)
{
    val16 = (static_cast<uint16_t>(buf[1]) << 8) |
            static_cast<uint16_t>(buf[0]);
    val8 = buf[2];
    return buf += 3;
}

template<typename T1>
constexpr uint8_t* SET_INT32_PTR(uint8_t* buf, T1 val)
{
    assert(sizeof(val) == 4);
    buf[0] = static_cast<char>(val & 0xFF);
    buf[1] = static_cast<char>((val >> 8) & 0xFF);
    buf[2] = static_cast<char>((val >> 16) & 0xFF);
    buf[3] = static_cast<char>(val >> 24);
    return buf += 4;
}

constexpr const uint8_t* GET_INT32_PTR(const uint8_t* buf, int32_t& val)
{
    val = (static_cast<int32_t>(buf[3]) << 24) |
          (static_cast<int32_t>(buf[2]) << 16) |
          (static_cast<int32_t>(buf[1]) << 8)  |
          (static_cast<int32_t>(buf[0]));
    return buf += 4;
}

constexpr int32_t GET_INT32(const uint8_t* buf)
{
    int32_t val = 0;
    GET_INT32_PTR(buf, val);
    return val;
}

template<typename T1>
constexpr uint8_t* SET_UINT32_PTR(uint8_t* buf, T1 val)
{
    assert(sizeof(val) == 4);
    buf[0] = static_cast<char>(val & 0xFF);
    buf[1] = static_cast<char>((val >> 8) & 0xFF);
    buf[2] = static_cast<char>((val >> 16) & 0xFF);
    buf[3] = static_cast<char>(val >> 24);
    return buf += 4;
}

template<typename T1>
constexpr void SET_UINT32(uint8_t* buf, T1 val)
{
    SET_UINT32_PTR(buf, val);
}


template<typename T1>
constexpr const uint8_t* GET_UINT32_PTR(const uint8_t* buf, T1& val)
{
    val = (static_cast<uint32_t>(buf[3]) << 24) |
          (static_cast<uint32_t>(buf[2]) << 16) |
          (static_cast<uint32_t>(buf[1]) << 8)  |
          (static_cast<uint32_t>(buf[0]));
    return buf += 4;
}

constexpr uint32_t GET_UINT32(const uint8_t* buf)
{
    uint32_t val = 0;
    GET_UINT32_PTR(buf, val);
    return val;
}

constexpr uint8_t* SET_INT64_PTR(uint8_t* buf, int64_t val)
{
    static_assert(sizeof(val) == 8);
    buf[0] = static_cast<char>(val & 0xFF);
    buf[1] = static_cast<char>((val >> 8) & 0xFF);
    buf[2] = static_cast<char>((val >> 16) & 0xFF);
    buf[3] = static_cast<char>((val >> 24) & 0xFF);
    buf[4] = static_cast<char>((val >> 32) & 0xFF);
    buf[5] = static_cast<char>((val >> 40) & 0xFF);
    buf[6] = static_cast<char>((val >> 48) & 0xFF);
    buf[7] = static_cast<char>(val >> 56);
    return buf += 8;
}

constexpr const uint8_t* GET_INT64_PTR(const uint8_t* buf, int64_t& val)
{
    val = (static_cast<int64_t>(buf[7]) << 56) |
          (static_cast<int64_t>(buf[6]) << 48) |
          (static_cast<int64_t>(buf[5]) << 40) |
          (static_cast<int64_t>(buf[4]) << 32) |
          (static_cast<int64_t>(buf[3]) << 24) |
          (static_cast<int64_t>(buf[2]) << 16) |
          (static_cast<int64_t>(buf[1]) << 8)  |
          (static_cast<int64_t>(buf[0]));
    return buf += 8;
}


    /////////////////////////////////////
    // FieldPacket
    /////////////////////////////////////

/* Each field value consist of a 12 bit length and 4 bits for type
   and then DATA [LENGTH,TYPE,DATA....] */
constexpr auto FIELDVALUE_PREFIX = 2;

template<typename FIELDTYPE, typename FIELDSIZE>
constexpr uint8_t* WRITEFIELD_TYPE(uint8_t* buf, FIELDTYPE fieldtype, FIELDSIZE fieldsize)
{
    assert(fieldtype >= 0 && fieldtype <= 0xF);
    assert(fieldsize >= 0 && fieldsize <= 0xFFF);
    return SET_UINT12_UINT4_PTR(buf, fieldsize, fieldtype);
}

template<typename FIELDTYPE, typename T2>
constexpr uint8_t* WRITEFIELD_VALUE_U8(uint8_t* buf, FIELDTYPE fieldtype, T2 value)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, sizeof(uint8_t));
    return SET_UINT8_PTR(buf, value);
}

template<typename FIELDTYPE, typename T2>
constexpr uint8_t* WRITEFIELD_VALUE_U16(uint8_t* buf, FIELDTYPE fieldtype, T2 value)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, sizeof(uint16_t));
    return SET_UINT16_PTR(buf, value);
}

template<typename FIELDTYPE, typename T2, typename T3>
constexpr uint8_t* WRITEFIELD_VALUE_U16_AND_U8(uint8_t* buf, FIELDTYPE fieldtype, T2 val16, T3 val8)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, (sizeof(uint16_t) + sizeof(uint8_t)));
    buf = SET_UINT16_PTR(buf, val16);
    buf = SET_UINT8_PTR(buf, val8);
    return buf;
}

template<typename FIELDTYPE, typename T2>
constexpr uint8_t* WRITEFIELD_VALUE_I32(uint8_t* buf, FIELDTYPE fieldtype, T2 value)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, sizeof(int32_t));
    buf = SET_INT32_PTR(buf, value);
    return buf;
}

template<typename FIELDTYPE, typename T2>
constexpr uint8_t* WRITEFIELD_VALUE_U32(uint8_t* buf, FIELDTYPE fieldtype, T2 value)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, sizeof(uint32_t));
    return SET_UINT32_PTR(buf, value);
}

template<typename FIELDTYPE, typename T2>
constexpr uint8_t* WRITEFIELD_VALUE_I64(uint8_t* buf, FIELDTYPE fieldtype, T2 value)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, sizeof(int64_t));
    buf = SET_INT64_PTR(buf, value);
    return buf;
}

template<typename FIELDTYPE, typename T2, typename T3>
constexpr uint8_t* WRITEFIELD_DATA(uint8_t* buf, FIELDTYPE fieldtype, T2 data, T3 size)
{
    buf = WRITEFIELD_TYPE(buf, fieldtype, size);
    memcpy(buf, data, size);
    buf += size;
    return buf;
}

constexpr uint16_t READFIELD_TYPE(const uint8_t* buf)
{
    return GET_UINT16(buf) >> 12;
}

constexpr uint16_t READFIELD_SIZE(const uint8_t* buf)
{
    return GET_UINT16(buf) & 0xFFF;
}

constexpr uint8_t* READFIELD_DATAPTR(uint8_t* buf)
{
    return buf + FIELDVALUE_PREFIX;
}

constexpr const uint8_t* READFIELD_DATAPTR(const uint8_t* buf)
{
    return buf + FIELDVALUE_PREFIX;
}

template<typename FIELDTYPE, typename T2>
constexpr const uint8_t* FINDFIELD_TYPE(const uint8_t* buf, FIELDTYPE fieldtype, T2 buf_size)
{
    const uint8_t* buf_start = buf;
    while (buf < buf_start + buf_size) {
        uint8_t new_field = READFIELD_TYPE(buf);
        if (new_field != fieldtype) {
            buf += FIELDVALUE_PREFIX + READFIELD_SIZE(buf);
            continue;
        }
        break;
    }
    buf = (buf >= buf_start + buf_size || buf + READFIELD_SIZE(buf) >= buf_start + buf_size)? nullptr : buf;
    return buf;
}

template<typename FIELDTYPE, typename T2>
constexpr uint8_t* FINDFIELD_TYPE(uint8_t* buf, FIELDTYPE fieldtype, T2 buf_size)
{
    return const_cast<uint8_t*>(FINDFIELD_TYPE(static_cast<const uint8_t*>(buf), fieldtype, buf_size));
}

    //packet kinds
    enum PacketKind
    {
        PACKET_KIND_HELLO                           =  1,
        PACKET_KIND_KEEPALIVE                       =  2,

        PACKET_KIND_VOICE                           =  3,
        PACKET_KIND_VOICE_CRYPT                     =  4, 

        PACKET_KIND_VIDEO                           =  5, 
        PACKET_KIND_VIDEO_CRYPT                     =  6, 

        PACKET_KIND_MEDIAFILE_AUDIO                 =  7,
        PACKET_KIND_MEDIAFILE_AUDIO_CRYPT           =  8,

        PACKET_KIND_MEDIAFILE_VIDEO                 =  9,
        PACKET_KIND_MEDIAFILE_VIDEO_CRYPT           = 10,

        PACKET_KIND_DESKTOP                         = 11,
        PACKET_KIND_DESKTOP_CRYPT                   = 12,
        PACKET_KIND_DESKTOP_ACK                     = 13,
        PACKET_KIND_DESKTOP_ACK_CRYPT               = 14,
        PACKET_KIND_DESKTOP_NAK                     = 15,
        PACKET_KIND_DESKTOP_NAK_CRYPT               = 16,
        PACKET_KIND_DESKTOPCURSOR                   = 17,
        PACKET_KIND_DESKTOPCURSOR_CRYPT             = 18,

        PACKET_KIND_DESKTOPINPUT                    = 19,
        PACKET_KIND_DESKTOPINPUT_CRYPT              = 20,
        PACKET_KIND_DESKTOPINPUT_ACK                = 21,
        PACKET_KIND_DESKTOPINPUT_ACK_CRYPT          = 22,

        /* When adding new packet types, then remember to 
         * update PacketQueue::RemoveChannelPackets() 
         * for none channel specific packet kinds */
    };

    //byte indexes for all packet types
    enum PacketIndex
    {
        PACKET_INDEX_KIND                       = 0,
        PACKET_MASK_KIND                        = 0x7F,
        PACKET_INDEX_DEST_USER_SET              = 0,
        PACKET_MASK_DEST_USER_SET               = 0x80, //bit set means TT_USER_HEADER_SIZE

        PACKET_INDEX_SRC_DEST                   = 1, //src user id, dest channel id
        PACKET_MASK_SRC_DEST                    = 0xFFFFFF,

        PACKET_INDEX_TIME                       = 4,
        PACKET_MASK_TIME                        = 0xFFFFFFFF,

        PACKET_INDEX_DEST_USER                  = 8, //dest userid
        PACKET_MASK_DEST_USER                   = 0xFFF,

        PACKET_INDEX_FIELDS_DEST_CHANNEL        = 8, //hdr size  TT_CHANNEL_HEADER_SIZE
        PACKET_INDEX_FIELDS_DEST_USER_SET       = 10, //hdr size TT_USER_HEADER_SIZE
    };

    enum PacketHdrType
    {
        PACKETHDR_CHANNEL_ONLY = 1, //TT_CHANNEL_HEADER_SIZE
        PACKETHDR_DEST_USER    = 2  //TT_USER_HEADER_SIZE
    };

//The minimum size of a packet
constexpr auto TT_CHANNEL_HEADER_SIZE = 8;
constexpr auto TT_USER_HEADER_SIZE = 10;

constexpr auto TT_MAX_HEADER_SIZE = 10;

constexpr auto MAX_FIELD_SIZE = 0xFFF;
constexpr auto MAX_ENC_FRAMESIZE = 0xFFF /* 12 bits */;

    class FieldPacket
    {
    private:
        void Init(PacketHdrType hdr_type, uint8_t kind, uint16_t src_userid, uint32_t time);
        const FieldPacket& operator= (const FieldPacket& p);
    public:
        FieldPacket(PacketHdrType hdr_type, uint8_t kind, uint16_t src_userid, uint32_t time);
        FieldPacket(const char* packet, uint16_t packet_size);
        FieldPacket(const iovec* v, uint16_t buffers);
        FieldPacket(const FieldPacket& p); //copy constructor
        //helper for decrypted packet
        FieldPacket(uint8_t kind, const FieldPacket& crypt_pkt, iovec& decrypt_fields);

        virtual ~FieldPacket();
        bool Finalized() const;
        PacketHdrType GetHdrType() const;

        uint8_t GetKind() const;
        uint16_t GetSrcUserID() const;
        uint16_t GetChannel() const;
        uint16_t GetDestUserID() const;
        uint32_t GetTime() const;
        void SetChannel(uint16_t channelid);
        //requires TT_USER_HEADER_SIZE header type
        void SetDestUser(uint16_t userid);

        const iovec* GetPacket(int& buffers) const;
        uint16_t GetPacketSize() const;
        bool ValidatePacket() const;

#ifdef ENABLE_ENCRYPTION
        const std::set<uint8_t>& GetCryptSections() const { return m_crypt_sections; }
#endif

    protected:
        enum : uint8_t
        {
            /* FIELDTYPE must NOT conflict with sub-classed packet */
            /* New fields here to be compatible */
            FIELDTYPE_LAST = 0
            //WARNING: If new fields are added ensure Crypt*Packet are 
            //updated with these new fields. BuildAudioPackets() and
            //ReassembleAudioPacket() also need to support these new fields
        };
        void GetSrcDest(uint16_t& src_userid, uint16_t& dest_chanid) const;
        uint8_t* GetFieldsStart() const;
        uint8_t* FindFieldNonConst(uint8_t fieldtype) const;
        const uint8_t* FindField(uint8_t fieldtype) const;
        std::vector<iovec> m_iovec;
        bool m_cleanup = false;
#ifdef ENABLE_ENCRYPTION
        //Holds which part of 'm_iovec' should be encrypted by 'CryptPacket'
        std::set<uint8_t> m_crypt_sections;
#endif
    };

    using packet_ptr_t = std::unique_ptr<FieldPacket>;

    class HelloPacket : public FieldPacket
    {
    public:
        HelloPacket(uint16_t src_userid, uint32_t time);
        HelloPacket(const char* packet, uint16_t packet_size)
            : FieldPacket(packet, packet_size) { }
        uint8_t GetProtocol() const;

    private:
        enum : uint8_t
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            FIELDTYPE_PROTOCOL = FIELDTYPE_LAST+1, //uint16_t
            /* New fields here to be compatible */
        };
    };

    //Keep alive packet with bogus payload for detecting UDP MTU
    class KeepAlivePacket : public FieldPacket
    {
    public:
        KeepAlivePacket(uint16_t src_userid, uint32_t time)
            : FieldPacket(PACKETHDR_CHANNEL_ONLY, PACKET_KIND_KEEPALIVE, src_userid, time) {}
        KeepAlivePacket(uint16_t src_userid, uint32_t time, 
                        uint16_t payload_size);
        
        KeepAlivePacket(const char* packet, uint16_t packet_size)
            : FieldPacket(packet, packet_size) { }

        KeepAlivePacket(const iovec* v, uint16_t buffers)
            : FieldPacket(v, buffers) { }

        KeepAlivePacket(const KeepAlivePacket& packet) 
             = default;

        uint16_t GetPayloadSize() const;

    private:
        enum : uint8_t
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            FIELDTYPE_PAYLOAD = FIELDTYPE_LAST+1, //uint16_t
            /* New fields here to be compatible */
        };
        
    };

    using ka_mtu_packet_t = std::shared_ptr< KeepAlivePacket >;


    /* The number of byes in a typical CBR AudioPacket (including FieldPacket).
     * (Used for segmentation). Update BuildAudioPackets() if changes are made
     * to the sizes. */
    constexpr size_t AUDIOPACKET_TYPICAL_CBR_HEADER_SIZE =
        TT_MAX_HEADER_SIZE +
        FIELDVALUE_PREFIX + sizeof(uint16_t) + // FIELDTYPE_CHANNEL
        FIELDVALUE_PREFIX + sizeof(uint32_t) + // FIELDTYPE_CHANNELKEY
        FIELDVALUE_PREFIX + sizeof(uint16_t) + // FIELDTYPE_STREAMID
        FIELDVALUE_PREFIX + sizeof(uint16_t) + // FIELDTYPE_PACKETNUMBER
        FIELDVALUE_PREFIX;                     // FIELDTYPE_ENCDATA

    /* The number of byes in a typical VBR AudioPacket (including FieldPacket).
     * (Used for segmentation). Update BuildAudioPackets() if changes are made
     * to the sizes. */
    constexpr size_t AUDIOPACKET_TYPICAL_VBR_HEADER_SIZE =
        TT_MAX_HEADER_SIZE +
        FIELDVALUE_PREFIX + sizeof(uint16_t) + // FIELDTYPE_CHANNEL
        FIELDVALUE_PREFIX + sizeof(uint32_t) + // FIELDTYPE_CHANNELKEY
        FIELDVALUE_PREFIX + sizeof(uint16_t) + // FIELDTYPE_STREAMID
        FIELDVALUE_PREFIX + sizeof(uint16_t) + // FIELDTYPE_PACKETNUMBER
        FIELDVALUE_PREFIX +                    // FIELDTYPE_ENCDATA
        FIELDVALUE_PREFIX;                      // FIELDTYPE_ENCFRAMESIZES

    /* Creates PACKET_KIND_VOICE or PACKET_KIND_MEDIAFILE_AUDIO */
    class AudioPacket : public FieldPacket
    {
    public:
        //Constructor for codec with no fragments and no frame sizes
        AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                    uint8_t stream_id, uint16_t packet_no, 
                    const char* enc_audio, uint16_t enc_length);

        //Constructor for codec with fragmented packets
        AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                    uint8_t stream_id, uint16_t packet_no,
                    uint8_t frag_no, uint8_t* frag_cnt, 
                    const char* enc_audio, uint16_t enc_length);

        //Constructor for codec with varirable frame sizes
        AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                    uint8_t stream_id, uint16_t packet_no, 
                    const char* enc_audio, uint16_t enc_length, 
                    const std::vector<uint16_t>& enc_framesizes);

        //Constructor for codec with variable frame sizes and fragmented packets
        AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                    uint8_t stream_id, uint16_t packet_no, uint8_t frag_no, 
                    uint8_t* frag_cnt, const char* enc_audio, uint16_t enc_length, 
                    const std::vector<uint16_t>* enc_framesizes);

        AudioPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                    iovec& decrypt_fields)
                    : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        AudioPacket(const char* packet, uint16_t packet_size);

        AudioPacket(const FieldPacket& packet);
        AudioPacket(const AudioPacket& packet);

        enum : uint8_t
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            FIELDTYPE_STREAMID_PKTNUM = FIELDTYPE_LAST+1, //uint8_t, uint16_t
            FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT,    //uint8_t, uint16_t, uint8_t (implies fragment 0)
            FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO,     //uint8_t, uint16_t, uint8_t
            FIELDTYPE_ENCDATA,                     //array uint8_t
            FIELDTYPE_ENCFRAMESIZES,               //array uint12_t
            /* New fields here to be compatible */
        };

        static constexpr uint8_t INVALID_FRAGMENT_NO = 0xFF;

        uint8_t GetStreamID() const;
        uint16_t GetPacketNumber() const;
        uint16_t GetPacketNumberAndFragNo(uint8_t& frag_no, uint8_t* frag_cnt) const;
        const char* GetEncodedAudio(uint16_t& length) const;
        std::vector<uint16_t> GetEncodedFrameSizes() const;
        bool HasFragments() const;
        bool HasFrameSizes() const { return FindField(FIELDTYPE_ENCFRAMESIZES) != nullptr; }

    private:
        void InitCommon(uint8_t stream_id, uint16_t packet_no, 
                        const uint8_t* frag_no, const uint8_t* frag_cnt, 
                        const char* enc_audio, uint16_t enc_length,
                        const std::vector<uint16_t>* enc_framesizes);
        bool GetStreamField(uint8_t& streamid, uint16_t& packet_no,
                            uint8_t& frag_no, uint8_t* frag_cnt) const;
    };

    using audiopacket_t = std::shared_ptr< AudioPacket >;

    using VoicePacket = AudioPacket;
    using AudioFilePacket = AudioPacket;

    /* Creates PACKET_KIND_VIDEO */
    class VideoPacket : public FieldPacket
    {
    private:
        //constructor helper
        uint8_t* Init(uint8_t kind, uint8_t stream_id, uint32_t packet_no,
                   const uint16_t* width, const uint16_t* height,
                   const char* enc_data, uint16_t enc_len, 
                   const uint16_t* fragmentno, const uint16_t* fragmentcnt);
    public:
        VideoPacket(const VideoPacket& p); //copy constructor
        VideoPacket(const FieldPacket& p);
        VideoPacket(const char* packet, uint16_t packet_size);
        VideoPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                    iovec& decrypt_fields)
                    : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        //build complete video packet
        VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                    uint8_t stream_id, uint32_t packet_no, 
                    const uint16_t* width, const uint16_t* height,
                    const char* enc_data, uint16_t enc_len);

        //build fragmented video packet
        VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time,
                    uint8_t stream_id, uint32_t packet_no, 
                    const uint16_t* width, const uint16_t* height,
                    const char* enc_data, uint16_t enc_len, 
                    uint16_t fragmentcnt);

        //build fragment video packet
        VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time,
                    uint8_t stream_id, uint32_t packet_no, 
                    const char* enc_data, uint16_t enc_len, 
                    uint16_t fragmentno);

        uint8_t GetStreamID() const { return GetStreamID(nullptr); }

        uint8_t GetStreamID(uint32_t* packet_no, uint16_t* fragno = nullptr,
                            uint16_t* fragcnt = nullptr, uint16_t* width = nullptr,
                            uint16_t* height = nullptr) const;
        uint32_t GetPacketNo() const;

        uint16_t GetFragmentNo() const;
        uint16_t GetFragmentCount() const;
        static constexpr uint16_t INVALID_FRAGMENT_NO = 0xFFFF;

        bool GetVideoInfo(uint16_t& width, uint16_t& height) const;

        const char* GetEncodedData(uint16_t& packet_bytes) const;

        enum : uint8_t
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            FIELDTYPE_STREAMID_PKTNUM_VIDINFO = FIELDTYPE_LAST+1, //[uint8_t,uint32_t,uint12_t,uint12_t]
            FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO, //[uint8_t,uint32_t,uint16_t,uint12_t,uint12_t]
            FIELDTYPE_STREAMID_PKTNUM, /* Packet which is not fragmented. [uint8_t,uint32_t] */
            FIELDTYPE_STREAMID_PKTNUM_FRAGCNT, /* Fragmented packet with frag_count, implies frag no = 0, [uint8_t,uint32_t,uint16_t] */
            FIELDTYPE_STREAMID_PKTNUM_FRAGNO, /* Fragment of packet with frag_no. [uint8_t,uint32_t,uint16_t] */
            FIELDTYPE_ENCDATA, /* Encoded video data. [Array-char...] */
            /* New fields here to be compatible */
        };
    };

    using VideoCapturePacket = VideoPacket;
    using VideoFilePacket = VideoPacket;

    using videopacket_t = std::shared_ptr< VideoPacket >;
    
    //desktop block with pointer to data
    struct desktop_block
    {
        const char* block_data = nullptr;
        uint16_t block_size = 0;
    };
    //blockno -> block
    using map_block_t = std::map< uint16_t, desktop_block >;
    //a fragmented block
    struct block_fragment
    {
        uint16_t block_no = 0;
        uint8_t frag_no = 0;
        uint8_t frag_cnt = 0;
        const char* frag_data = nullptr;
        uint16_t frag_size = 0;
    };
    //blockno -> fragment
    using block_frags_t = std::list<block_fragment>;
    //blockno -> set(block_nums)
    using map_dup_blocks_t = std::map< uint16_t, std::set<uint16_t> >;
    //blockno -> set(block_nums)
    using mmap_dup_blocks_t = std::multimap< uint16_t, std::set<uint16_t> >;
    using dup_block_pair_t = std::pair< uint16_t, std::set<uint16_t> >;

    bool IsBlockRange(const std::set<uint16_t>& blocks);

    class DesktopPacket : public FieldPacket
    {
    public:
        //Start new session (based on FIELDTYPE_SESSIONID_NEW), session id = session_id
        DesktopPacket(uint16_t src_userid, uint32_t time, uint8_t session_id, 
                      uint16_t width, uint16_t height, uint8_t bmp_mode, 
                      uint16_t pkt_upd_index, uint16_t pkt_upd_count,
                      const map_block_t& blocks, const block_frags_t& fragments,
                      const mmap_dup_blocks_t& dup_blocks);

        //Update session (packet based on FIELDTYPE_SESSIONID_UPD)
        DesktopPacket(uint16_t src_userid, uint32_t time, uint8_t session_id,
                      uint16_t pkt_upd_index, uint16_t pkt_upd_count,
                      const map_block_t& blocks, const block_frags_t& fragments,
                      const mmap_dup_blocks_t& dup_blocks);

        DesktopPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                      iovec& decrypt_fields)
                      : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        DesktopPacket(const char* packet, uint16_t packet_size);

        DesktopPacket(const DesktopPacket& packet);

        bool UpdatePacketCount(uint16_t pkt_upd_count);

        uint8_t GetSessionID() const;

        //returns INVALID_PACKET_INDEX on error
        uint16_t GetPacketIndex() const;

        //session properties only apply if packet contains 'FIELDTYPE_SESSIONID_NEW' field
        bool GetSessionProperties(uint8_t* session_id, uint16_t* width, 
                                  uint16_t* height, uint8_t* bmp_mode,
                                  uint16_t* pkt_upd_index,
                                  uint16_t* pkt_upd_count) const;

        //stream properties only apply if packet contains 'FIELDTYPE_SESSIONID_UPD' field
        bool GetUpdateProperties(uint8_t* session_id, uint16_t* pkt_upd_index,
                                 uint16_t* pkt_upd_count) const;
        
        bool GetBlocks(map_block_t& blocks) const;
        bool GetBlockFragments(block_frags_t& fragments) const;
        bool GetDuplicateBlocks(map_dup_blocks_t& dup_blocks) const;

        const char* GetBlock(uint16_t block_no, uint16_t& length) const;
        
        static constexpr uint16_t INVALID_PACKET_INDEX = -1;
        static constexpr uint16_t BLOCKNO_INDEX_MAX = 0xFFF;
        static constexpr uint16_t BLOCKNUMS_MAX = 0x1000; // uint12 limit
        static constexpr uint16_t WIDTH_MAX = 0xFFFF, HEIGHT_MAX = 0xFFFF; // uint16

    private:
        uint16_t InitCommon(const map_block_t& blocks, 
                            const block_frags_t& fragments,
                            const mmap_dup_blocks_t& dup_blocks);

        enum : uint8_t
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            //(total blocks not needed since it can be calc'ed from width-height)

            //[sessionid(uint8_t), width(uint16_t), height(uint16_t), color-mode(uint8_t), pkt_upd_index(uint16_t), pkt_upd_count(uint16_t)]
            FIELDTYPE_SESSIONID_NEW = FIELDTYPE_LAST+1,
            //[sessionid(uint8_t), pkt_upd_index(uint16_t), pkt_upd_count(uint16_t)]
            FIELDTYPE_SESSIONID_UPD, 
            //[[blockno(uint12_t), blocksize(uint12_t)], ... ]
            FIELDTYPE_BLOCKNUMS_AND_SIZES, 
            FIELDTYPE_BLOCKS_DATA,   //array uint8_t
            //[[blockno(uint12_t), fragsize(uint12_t), fragno(uint4_t), frag_cnt(uint4_t)], ...]
            FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES, 
            FIELDTYPE_BLOCKS_FRAG_DATA,   //array uint8_t
            //[[blockno(uint12_t), ..., 0xFFF], ...]
            FIELDTYPE_BLOCK_DUP,
            //[[blockno(uint12_t), startno(uint12_t), endno(uint12_t)], ...]
            FIELDTYPE_BLOCK_DUP_RANGE
        };
    };

    using desktoppacket_t = std::shared_ptr< DesktopPacket >;

    //Calc the size of fields used
    constexpr auto DESKTOPPACKET_SESSIONUSAGE(bool new_session)
    {
        //FIELDTYPE_SESSIONID_NEW | FIELDTYPE_SESSIONID_UPD
        return new_session ? (FIELDVALUE_PREFIX + 10) : (FIELDVALUE_PREFIX + 5);
    }

    template<typename T1, typename T2>
    constexpr auto DESKTOPPACKET_DATAUSAGE(T1 blocks_cnt, T2  frags_cnt)
    {
        //FIELDTYPE_BLOCKNUMS_AND_SIZES & FIELDTYPE_BLOCKS_DATA
        //FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES & FIELDTYPE_BLOCKS_FRAG_DATA
        return ((blocks_cnt)? (FIELDVALUE_PREFIX + (blocks_cnt) * 3) + FIELDVALUE_PREFIX : 0) + ((frags_cnt)? (FIELDVALUE_PREFIX + (frags_cnt) * 4) + FIELDVALUE_PREFIX : 0);
    }

    template<typename T1, typename T2>
    constexpr auto DESKTOPPACKET_BLOCKUSAGE(T1 dup_blocks_cnt, T2  total_blocks)
    {
        //FIELDTYPE_BLOCK_DUP
        return ((dup_blocks_cnt)? (FIELDVALUE_PREFIX + (((((dup_blocks_cnt) * 2 + (total_blocks)) * 12) % 8) ? ((((dup_blocks_cnt) * 2 + (total_blocks)) * 12) / 8 + 1) : ((((dup_blocks_cnt) * 2 + (total_blocks)) * 12) / 8))) : 0);
    }

    template<typename T>
    constexpr auto DESKTOPPACKET_BLOCKRANGEUSAGE(T block_range_cnt) {
        //FIELDTYPE_BLOCK_DUP_RANGE
        return ((block_range_cnt)? (FIELDVALUE_PREFIX + (((((block_range_cnt) * 3 * 12) % 8)? (((block_range_cnt) * 3 * 12) / 8 + 1) :     ((block_range_cnt) * 3 * 12) / 8))) : 0);
    }

    
    //packetno -> packetno
    using packet_range_t = std::map<uint16_t, uint16_t>;

    class DesktopAckPacket : public FieldPacket
    {
    public:
        DesktopAckPacket(uint16_t src_userid, uint32_t time, 
                         uint16_t owner_userid, uint8_t session_id, 
                         uint32_t time_ack,
                         const std::set<uint16_t>& packets_ack,
                         const packet_range_t& packet_range_ack);

        DesktopAckPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                         iovec& decrypt_fields)
                         : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        DesktopAckPacket(const char* packet, uint16_t packet_size);

        DesktopAckPacket(const DesktopAckPacket& packet);

        bool GetSessionInfo(uint16_t* owner_userid, uint8_t* session_id, 
                            uint32_t* upd_time) const;

        uint8_t GetSessionID() const
        {
            uint8_t sessionid = 0;
            if (GetSessionInfo(nullptr, &sessionid, nullptr))
                return sessionid;
            return sessionid;
         }

        bool GetPacketsAcked(std::set<uint16_t>& packets_ack) const;

    private:
        void InitCommon(const std::set<uint16_t>& packets_ack,
                        const packet_range_t& packet_range_ack);

        enum : uint8_t
        {
            //[sessionid(uint8_t), userid(uint16_t), time_ack(uint32_t)]
            FIELDTYPE_SESSIONID_ACK = FIELDTYPE_LAST+1,
            //[pktnum(uint16), ...
            FIELDTYPE_PACKETS_ACK,
            //[[pktnum(uint16), pktnum(uint16)], ...
            FIELDTYPE_PACKETRANGE_ACK,
        };
    };

    class DesktopNakPacket : public FieldPacket
    {
    public:
        DesktopNakPacket(uint16_t src_userid, uint32_t time, 
                         uint8_t session_id);

        DesktopNakPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                         iovec& decrypt_fields)
                         : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        DesktopNakPacket(const char* packet, uint16_t packet_size);

        DesktopNakPacket(const DesktopNakPacket& packet);

        uint8_t GetSessionID() const;

    private:
       enum : uint8_t
        {
            //[sessionid(uint8_t)]
            FIELDTYPE_SESSIONID_NAK = FIELDTYPE_LAST+1,
        };

    };

    class DesktopCursorPacket : public FieldPacket
    {
    public:
        DesktopCursorPacket(uint16_t src_userid, uint32_t time, 
                            uint8_t session_id, int16_t x, int16_t y);

        DesktopCursorPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                            iovec& decrypt_fields)
                            : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        DesktopCursorPacket(const char* packet, uint16_t packet_size);

        DesktopCursorPacket(const DesktopCursorPacket& packet);

        bool GetSessionCursor(uint16_t* dest_userid, uint8_t* session_id, 
                              int16_t* x, int16_t* y) const;

        uint8_t GetSessionID() const
        {
            uint8_t streamid = 0;
            GetSessionCursor(nullptr, &streamid, nullptr, nullptr);
            return streamid;
        }

        int16_t GetX() const
        {
            int16_t v = 0;
            GetSessionCursor(nullptr, nullptr, &v, nullptr);
            return v;
        }
        
        int16_t GetY() const
        {
            int16_t v = 0;
            GetSessionCursor(nullptr, nullptr, nullptr, &v);
            return v;
        }
            
        static const uint16_t INVALID_DEST_USERID = -1;

        uint16_t GetDestUserID() const;

    private:
       enum : uint8_t
        {
            //[sessionid(uint8_t), x(uint16_t), y(uint16_t)]
            FIELDTYPE_MY_CURSORPOS = FIELDTYPE_LAST+1,
            //[destuserid(uint16_t), sessionid(uint8_t), x(uint16_t), y(uint16_t)]
            FIELDTYPE_REMOTE_CURSORPOS,
        };
    };

    struct DesktopInput;

    class DesktopInputPacket : public FieldPacket
    {
    public:
        DesktopInputPacket(uint16_t src_userid, uint32_t time, 
                           uint8_t session_id, uint8_t packetno,
                           const std::vector<DesktopInput>& inputs);
        
        DesktopInputPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                           iovec& decrypt_fields)
                           : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        DesktopInputPacket(const char* packet, uint16_t packet_size);

        DesktopInputPacket(const DesktopInputPacket& packet);

        bool GetSessionInfo(uint8_t* session_id, uint8_t* packetno) const;
        uint8_t GetSessionID() const;
        uint8_t GetPacketNo(bool* found = nullptr) const;

        bool GetDesktopInput(std::vector<DesktopInput>& desktopinputs) const;
        std::vector<DesktopInput> GetDesktopInput() const;

    private:

       enum : uint8_t
        {
            //[sessionid(uint8_t), packetno(uint8_t), [[x(uint16_t), y(uint16_t), keycode(uint32_t), keystate(uint32_t)], ...
            FIELDTYPE_REMOTE_INPUT = FIELDTYPE_LAST+1, //send input to remote desktop
        };
    };

    using desktopinput_pkt_t = std::shared_ptr< DesktopInputPacket >;

    class DesktopInputAckPacket : public FieldPacket
    {
    public:
        DesktopInputAckPacket(uint16_t src_userid, uint32_t time, 
                              uint8_t session_id, uint8_t packetno);

        DesktopInputAckPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                              iovec& decrypt_fields)
                              : FieldPacket(kind, crypt_pkt, decrypt_fields){}

        DesktopInputAckPacket(const char* packet, uint16_t packet_size)
            : FieldPacket(packet, packet_size) { }

        DesktopInputAckPacket(const DesktopInputAckPacket& packet);

        bool GetSessionInfo(uint8_t* session_id, uint8_t* packetno) const;

        uint8_t GetSessionID() const;
        uint8_t GetPacketNo(bool* found = nullptr) const;

    private:
       enum : uint8_t
        {
            //[sessionid(uint8_t), packetno(uint8_t)]
            FIELDTYPE_DESKTOPINPUT_ACK = FIELDTYPE_LAST+1,
        };
    };


#if defined(ENABLE_ENCRYPTION)

    constexpr auto  CRYPTKEY_SIZE = 32;

    template < typename PACKETTYPE, uint8_t PACKET_KIND_CRYPT, uint8_t PACKET_KIND_DECRYPTED >
    class CryptPacket : public FieldPacket
    {
        using decrypt_pkt_t = std::unique_ptr< PACKETTYPE >;
    public:
        CryptPacket(const PACKETTYPE& p, const std::array<uint8_t, CRYPTKEY_SIZE>& encryptkey);
        CryptPacket(const char* packet, uint16_t packet_size);
        CryptPacket(const FieldPacket& packet) : FieldPacket(packet) { assert(GetKind() == packet.GetKind()); }
        std::unique_ptr< PACKETTYPE > Decrypt(const std::array<uint8_t, CRYPTKEY_SIZE>& decryptkey) const;

        enum : uint8_t
        {
            FIELDTYPE_CRYPTDATA = FIELDTYPE_LAST+1,
        };
    };

#include "PacketLayout.inl"

    using CryptVoicePacket = CryptPacket<VoicePacket, PACKET_KIND_VOICE_CRYPT, PACKET_KIND_VOICE>;
    using CryptAudioFilePacket = CryptPacket<AudioFilePacket, PACKET_KIND_MEDIAFILE_AUDIO_CRYPT, PACKET_KIND_MEDIAFILE_AUDIO>;

    using CryptVideoCapturePacket = CryptPacket<VideoCapturePacket, PACKET_KIND_VIDEO_CRYPT, PACKET_KIND_VIDEO>;
    using CryptVideoFilePacket = CryptPacket<VideoFilePacket, PACKET_KIND_MEDIAFILE_VIDEO_CRYPT, PACKET_KIND_MEDIAFILE_VIDEO>;

    using CryptDesktopPacket = CryptPacket<DesktopPacket, PACKET_KIND_DESKTOP_CRYPT, PACKET_KIND_DESKTOP>;

    using CryptDesktopAckPacket = CryptPacket<DesktopAckPacket, PACKET_KIND_DESKTOP_ACK_CRYPT, PACKET_KIND_DESKTOP_ACK>;

    using CryptDesktopNakPacket = CryptPacket<DesktopNakPacket, PACKET_KIND_DESKTOP_NAK_CRYPT, PACKET_KIND_DESKTOP_NAK>;
    
    using CryptDesktopCursorPacket = CryptPacket<DesktopCursorPacket, PACKET_KIND_DESKTOPCURSOR_CRYPT, PACKET_KIND_DESKTOPCURSOR>;

    using CryptDesktopInputPacket = CryptPacket<DesktopInputPacket, PACKET_KIND_DESKTOPINPUT_CRYPT, PACKET_KIND_DESKTOPINPUT>;

    using CryptDesktopInputAckPacket = CryptPacket<DesktopInputAckPacket, PACKET_KIND_DESKTOPINPUT_ACK_CRYPT, PACKET_KIND_DESKTOPINPUT_ACK>;
    
#endif
} // namespace teamtalk
#endif
