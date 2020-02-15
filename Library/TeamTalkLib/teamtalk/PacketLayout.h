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

#include <ace/ACE.h>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <list>
#include <assert.h>

#if defined(ENABLE_ENCRYPTION)
#include <openssl/evp.h>
#include <openssl/aes.h>
#endif

#include <stdint.h>

#include "Common.h"

/******************************
*    TEAMTALK 4 PACKET LAYOUT
*******************************/

#define TEAMTALK_PACKET_PROTOCOL 1

#define TEAMTALK_DEFAULT_PACKET_PROTOCOL    1

#define FIELDHEADER_PAYLOAD     50

#define MIN_PAYLOAD_DATA_SIZE   400    //The raw data must be split in at most this size
#define MIN_PACKET_PAYLOAD_SIZE (MIN_PAYLOAD_DATA_SIZE + FIELDHEADER_PAYLOAD)   //The maximum size of fields and raw data

#define MAX_PAYLOAD_DATA_SIZE   1250   //The raw data must be split in at most this size
#define MAX_PACKET_PAYLOAD_SIZE (MAX_PAYLOAD_DATA_SIZE + FIELDHEADER_PAYLOAD)   //The maximum size of fields and raw data

static uint16_t MTU_QUERY_SIZES[] = {MIN_PAYLOAD_DATA_SIZE, 800, 1000, MAX_PAYLOAD_DATA_SIZE};
#define MTU_QUERY_SIZES_COUNT (sizeof(MTU_QUERY_SIZES)/sizeof(MTU_QUERY_SIZES[0]))

#define MAX_PACKET_SIZE 1350           //The maximum size of a packet

namespace teamtalk {


#ifdef ACE_BIG_ENDIAN
#pragma error "Big endian not supported"
#endif


#define set_uint4(buf, val1, val2)                  \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(val1 <= 0xF);                        \
        assert(val2 <= 0xF);                        \
        buf[0] = (val1 & 0xF) | ((val2 & 0xF) << 4);\
    } while(0)

#define set_uint4_ptr(buf, val1, val2, ptr)         \
    do {                                            \
        set_uint4(buf, val1, val2); ptr++;          \
    } while(0)

#define get_uint4(buf, val1, val2)                  \
    do {                                            \
        val1 = (buf[0] & 0xF);                      \
        val2 = (buf[0] >> 4);                       \
    } while(0)

#define get_uint4_ptr(buf, val1, val2, ptr)         \
    do {                                            \
        get_uint4(buf, val1, val2); ptr += 1;       \
    } while(0)

#define set_uint8(buf, val) (*(uint8_t*)buf = val)
#define set_uint8_ptr(buf, val, ptr)                \
    do {                                            \
        set_uint8(buf, val); ptr += 1;              \
    } while(0)
#define get_uint8(buf) *(const uint8_t*)(buf)
#define get_uint8_ptr(val, buf, ptr) {val = get_uint8(buf); ptr += 1;}

#define set_uint12(buf, val1)                       \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(val1 <= 0xFFF);                      \
        (buf)[0] = (char)(val1 & 0xFF);             \
        (buf)[1] = (char)((val1 >> 8) & 0xF);       \
    } while(0)

#define set_uint12_ptr(buf, val1, ptr)              \
    do {                                            \
        set_uint12(buf, val1);                      \
        ptr += 2;                                   \
    } while(0)

#define set2_uint12(buf, val1, val2)                \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(val1 <= 0xFFF);                      \
        assert(val2 <= 0xFFF);                      \
        (buf)[0] = (char)(val1 & 0xFF);             \
        (buf)[1] = (char)((val1 >> 8) & 0xF);       \
        (buf)[1] |= (char)((val2 & 0xF) << 4);      \
        (buf)[2] = (char)((val2 >> 4) & 0xFF);      \
    } while(0)

#define set2_uint12_ptr(buf, val1, val2, ptr)       \
    do {                                            \
        set2_uint12(buf, val1, val2);               \
        ptr += 3;                                   \
    } while(0)

#define get_uint12(buf, val1)                       \
    do {                                            \
       val1 = ((buf)[0] | ((buf[1] & 0xF) << 8));     \
    } while(0)

#define get_uint12_ptr(buf, val1, ptr)              \
    do {                                            \
        get_uint12(buf, val1);                      \
        ptr += 2;                                   \
    } while(0)

#define get2_uint12(buf, val1, val2)                \
    do {                                            \
       val1 = (buf[0] | ((buf[1] & 0xF) << 8));     \
       val2 = ((buf[1] >> 4) | (buf[2] << 4));      \
    } while(0)

#define get2_uint12_ptr(buf, val1, val2, ptr)       \
    do {                                            \
        get2_uint12(buf, val1, val2);               \
        ptr += 3;                                   \
    } while(0)

#define set_uint12_uint4(buf, val1, val2)           \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(val1 <= 0xFFF);                      \
        assert(val2 <= 0xF);                        \
        (buf)[0] = (char)(val1 & 0xFF);             \
        (buf)[1] = (char)((val1 >> 8) & 0xF);       \
        (buf)[1] |= (char)((val2 & 0xF) << 4);      \
    } while(0)

#define set_uint12_uint4_ptr(buf, val1, val2, ptr)  \
    do {                                            \
        set_uint12_uint4(buf, val1, val2);          \
        ptr += 2;                                   \
    } while(0)

#define get_uint12_uint4(buf, val1, val2)           \
    do {                                            \
       val1 = (buf[0] | ((buf[1] & 0xF) << 8));     \
       val2 = (buf[1] >> 4);                        \
    } while(0)

#define get_uint12_uint4_ptr(buf, val1, val2, ptr)  \
    do {                                            \
        get_uint12_uint4(buf, val1, val2);          \
        ptr += 2;                                   \
    } while(0)

#define set_uint16(buf, val)                        \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert((int)val <= 0xFFFF);                 \
        (buf)[0] = (char)(val & 0xFF);              \
        (buf)[1] = (char)(val >> 8);                \
    } while(0)

#define set_uint16_ptr(buf, val, ptr)               \
    do {                                            \
        set_uint16(buf, val); ptr += 2;             \
    } while(0)

#define get_uint16(buf) (((uint16_t)((const uint8_t*)buf)[1] << 8) |    \
                        ((uint16_t)((const uint8_t*)buf)[0]))

#define get_uint16_ptr(val, buf, ptr) {val = get_uint16(buf); ptr += 2;}

#define get_uint16_uint8(buf, val16, val8)                              \
    do {                                                                \
        val16 = (((uint16_t)((const uint8_t*)buf)[1] << 8) |            \
                (uint16_t)((const uint8_t*)buf)[0]);                    \
        val8 = (uint8_t)((const uint8_t*)buf)[2];                       \
    } while(0)


#define set_int32(buf, val)                         \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(sizeof(val) == 4);                   \
        (buf)[0] = (char)(val & 0xFF);              \
        (buf)[1] = (char)((val >> 8) & 0xFF);       \
        (buf)[2] = (char)((val >> 16) & 0xFF);      \
        (buf)[3] = (char)(val >> 24);               \
    }while(0)

#define get_int32(buf) (((int32_t)((const uint8_t*)buf)[3] << 24) | \
                        ((int32_t)((const uint8_t*)buf)[2] << 16) | \
                        ((int32_t)((const uint8_t*)buf)[1] << 8)  | \
                        ((int32_t)((const uint8_t*)buf)[0]))

#define set_uint32(buf, val)                        \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(sizeof(val) == 4);                   \
        (buf)[0] = (char)(val & 0xFF);              \
        (buf)[1] = (char)((val >> 8) & 0xFF);       \
        (buf)[2] = (char)((val >> 16) & 0xFF);      \
        (buf)[3] = (char)(val >> 24);               \
    }while(0)

#define set_uint32_ptr(buf, val, ptr)               \
    do {                                            \
        set_uint32(buf, val); ptr += 4;             \
    } while(0)

#define get_uint32(buf) (((uint32_t)((const uint8_t*)buf)[3] << 24) | \
                         ((uint32_t)((const uint8_t*)buf)[2] << 16) | \
                         ((uint32_t)((const uint8_t*)buf)[1] << 8)  | \
                         ((uint32_t)((const uint8_t*)buf)[0]))

#define get_uint32_ptr(val, buf, ptr) {val = get_uint32(buf); ptr += 4; }

#define set_int64(buf, val)                         \
    do {                                            \
        assert(sizeof((buf)[0]) == 1);              \
        assert(sizeof(val) == 8);                   \
        (buf)[0] = (char)(val & 0xFF);              \
        (buf)[1] = (char)((val >> 8) & 0xFF);       \
        (buf)[2] = (char)((val >> 16) & 0xFF);      \
        (buf)[3] = (char)((val >> 24) & 0xFF);      \
        (buf)[4] = (char)((val >> 32) & 0xFF);      \
        (buf)[5] = (char)((val >> 40) & 0xFF);      \
        (buf)[6] = (char)((val >> 48) & 0xFF);      \
        (buf)[7] = (char)(val >> 56);               \
    }while(0)

#define get_int64(buf) (((int64_t)((const uint8_t*)buf)[7] << 56) | \
                        ((int64_t)((const uint8_t*)buf)[6] << 48) | \
                        ((int64_t)((const uint8_t*)buf)[5] << 40) | \
                        ((int64_t)((const uint8_t*)buf)[4] << 32) | \
                        ((int64_t)((const uint8_t*)buf)[3] << 24) | \
                        ((int64_t)((const uint8_t*)buf)[2] << 16) | \
                        ((int64_t)((const uint8_t*)buf)[1] << 8)  | \
                        ((int64_t)((const uint8_t*)buf)[0]))


    /////////////////////////////////////
    // FieldPacket
    /////////////////////////////////////

/* Each field value consist of a 12 bit length and 4 bits for type
   and then DATA [LENGTH,TYPE,DATA....] */
#define FIELDVALUE_PREFIX 2

#define WRITEFIELD_TYPE(buf, fieldtype, fieldsize, ptr)                     \
    do {                                                                    \
    assert(sizeof(ptr[0]) == sizeof(uint8_t));                              \
    assert(fieldsize >= 0 && fieldsize <= 0xFFF);                           \
    set_uint12_uint4_ptr(buf, fieldsize, fieldtype, ptr);                   \
    } while(0)

#define WRITEFIELD_VALUE_U8(buf, fieldtype, value, ptr)                     \
    do {                                                                    \
    WRITEFIELD_TYPE(buf, fieldtype, sizeof(uint8_t), ptr);                  \
    set_uint8(ptr, value); ptr += sizeof(uint8_t);                          \
    } while(0)

#define WRITEFIELD_VALUE_U16(buf, fieldtype, value, ptr)                    \
    do {                                                                    \
    WRITEFIELD_TYPE(buf, fieldtype, sizeof(uint16_t), ptr);                 \
    set_uint16(ptr, value); ptr += sizeof(uint16_t);                        \
    } while(0)

#define WRITEFIELD_VALUE_U16_AND_U8(buf, fieldtype, val16, val8, ptr)           \
    do {                                                                        \
    WRITEFIELD_TYPE(buf, fieldtype, (sizeof(uint16_t) + sizeof(uint8_t)), ptr); \
    set_uint16(ptr, val16); ptr += sizeof(uint16_t);                            \
    set_uint8(ptr, val8); ptr += sizeof(uint8_t);                               \
    } while(0)

#define WRITEFIELD_VALUE_I32(buf, fieldtype, value, ptr)                    \
    do {                                                                    \
    WRITEFIELD_TYPE(buf, fieldtype, sizeof(int32_t), ptr);                  \
    set_int32(ptr, value); ptr += sizeof(int32_t);                          \
    } while(0)

#define WRITEFIELD_VALUE_U32(buf, fieldtype, value, ptr)                    \
    do {                                                                    \
    WRITEFIELD_TYPE(buf, fieldtype, sizeof(uint32_t), ptr);                 \
    set_uint32(ptr, value); ptr += sizeof(uint32_t);                        \
    } while(0)

#define WRITEFIELD_VALUE_I64(buf, fieldtype, value, ptr)                    \
    do {                                                                    \
    WRITEFIELD_TYPE(buf, fieldtype, sizeof(int64_t), ptr);                  \
    set_int64(ptr, value); ptr += sizeof(int64_t);                          \
    } while(0)

#define WRITEFIELD_DATA(buf, fieldtype, data, size, ptr)                    \
    do {                                                                    \
    WRITEFIELD_TYPE(buf, fieldtype, size, ptr);                             \
    memcpy(ptr, data, size); ptr += size;                                   \
    } while(0)

#define READFIELD_TYPE(buf) (get_uint16(buf) >> 12)
#define READFIELD_SIZE(buf) (get_uint16(buf) & 0xFFF)
#define READFIELD_DATAPTR(buf) (buf + FIELDVALUE_PREFIX)

#define FINDFIELD_TYPE(buf, fieldtype, buf_size, ptr)                       \
    do {                                                                    \
    const uint8_t* buf_start = buf;                                         \
    ptr = buf;                                                              \
    while(ptr < buf_start + buf_size) {                                     \
      uint8_t new_field = READFIELD_TYPE(ptr);                              \
      if(new_field != fieldtype) {                                          \
            ptr += FIELDVALUE_PREFIX + READFIELD_SIZE(ptr);                 \
            continue;                                                       \
      }                                                                     \
      break;                                                                \
    }                                                                       \
    ptr = (ptr >= buf_start + buf_size ||                                   \
        ptr + READFIELD_SIZE(ptr) >= buf_start + buf_size)? 0 : ptr;        \
    } while(0)

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
#define TT_CHANNEL_HEADER_SIZE 8
#define TT_USER_HEADER_SIZE 10

#define TT_MAX_HEADER_SIZE 10

#define MAX_FIELD_SIZE 0xFFF
#define MAX_ENC_FRAMESIZE 0xFFF /* 12 bits */

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
        enum
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
        uint8_t* FindField_NonConst(uint8_t fieldtype) const;
        const uint8_t* FindField(uint8_t fieldtype) const;
        std::vector<iovec> m_iovec;
        bool m_cleanup;
#ifdef ENABLE_ENCRYPTION
        //Holds which part of 'm_iovec' should be encrypted by 'CryptPacket'
        std::set<uint8_t> m_crypt_sections;
#endif
    };

    typedef std::unique_ptr<FieldPacket> packet_ptr_t;

    class HelloPacket : public FieldPacket
    {
    public:
        HelloPacket(uint16_t src_userid, uint32_t time);
        HelloPacket(const char* packet, uint16_t packet_size)
            : FieldPacket(packet, packet_size) { }
        uint8_t GetProtocol() const;

    private:
        enum
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
            : FieldPacket(packet) { }

        uint16_t GetPayloadSize() const;

    private:
        enum
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            FIELDTYPE_PAYLOAD = FIELDTYPE_LAST+1, //uint16_t
            /* New fields here to be compatible */
        };
        
    };

    typedef std::shared_ptr< KeepAlivePacket > ka_mtu_packet_t;


    /* The number of byes in a typical CBR AudioPacket (including FieldPacket).
     * (Used for segmentation). Update BuildAudioPackets() if changes are made
     * to the sizes. */
#define AUDIOPACKET_TYPICAL_CBR_HEADER_SIZE                           \
        (TT_MAX_HEADER_SIZE +                                         \
        FIELDVALUE_PREFIX + sizeof(uint16_t) +/*FIELDTYPE_CHANNEL*/   \
        FIELDVALUE_PREFIX + sizeof(uint32_t) +/*FIELDTYPE_CHANNELKEY*/\
        FIELDVALUE_PREFIX + sizeof(uint16_t) +/*FIELDTYPE_STREAMID*/  \
        FIELDVALUE_PREFIX + sizeof(uint16_t) +/*FIELDTYPE_PACKETNUMBER*/ \
        FIELDVALUE_PREFIX /*FIELDTYPE_ENCDATA*/ )

    /* The number of byes in a typical VBR AudioPacket (including FieldPacket).
     * (Used for segmentation). Update BuildAudioPackets() if changes are made
     * to the sizes. */
#define AUDIOPACKET_TYPICAL_VBR_HEADER_SIZE                           \
        (TT_MAX_HEADER_SIZE +                                         \
        FIELDVALUE_PREFIX + sizeof(uint16_t) +/*FIELDTYPE_CHANNEL*/   \
        FIELDVALUE_PREFIX + sizeof(uint32_t) +/*FIELDTYPE_CHANNELKEY*/\
        FIELDVALUE_PREFIX + sizeof(uint16_t) +/*FIELDTYPE_STREAMID*/  \
        FIELDVALUE_PREFIX + sizeof(uint16_t) +/*FIELDTYPE_PACKETNUMBER*/ \
        FIELDVALUE_PREFIX + /*FIELDTYPE_ENCDATA*/                     \
        FIELDVALUE_PREFIX /*FIELDTYPE_ENCFRAMESIZES*/ )

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

        enum
        {
            /* FIELDTYPE must NOT conflict with parent class packet */
            FIELDTYPE_STREAMID_PKTNUM = FIELDTYPE_LAST+1, //uint8_t, uint16_t
            FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT,    //uint8_t, uint16_t, uint8_t (implies fragment 0)
            FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO,     //uint8_t, uint16_t, uint8_t
            FIELDTYPE_ENCDATA,                     //array uint8_t
            FIELDTYPE_ENCFRAMESIZES,               //array uint12_t
            /* New fields here to be compatible */
        };

        static const uint8_t INVALID_FRAGMENT_NO = 0xFF;

        uint8_t GetStreamID() const;
        uint16_t GetPacketNumber() const;
        uint16_t GetPacketNumberAndFragNo(uint8_t& frag_no, uint8_t* frag_cnt) const;
        const char* GetEncodedAudio(uint16_t& length) const;
        std::vector<uint16_t> GetEncodedFrameSizes() const;
        bool HasFragments() const;
        bool HasFrameSizes() const { return FindField(FIELDTYPE_ENCFRAMESIZES) != NULL; }

    private:
        void InitCommon(uint8_t stream_id, uint16_t packet_no, 
                        uint8_t* frag_no, uint8_t* frag_cnt, 
                        const char* enc_audio, uint16_t enc_length,
                        const std::vector<uint16_t>* enc_framesizes);
        bool GetStreamField(uint8_t& streamid, uint16_t& packet_no,
                            uint8_t& frag_no, uint8_t* frag_cnt) const;
    };

    typedef std::shared_ptr< AudioPacket > audiopacket_t;

    typedef AudioPacket VoicePacket;
    typedef AudioPacket AudioFilePacket;

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
                    const uint16_t fragmentcnt);

        //build fragment video packet
        VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time,
                    uint8_t stream_id, uint32_t packet_no, 
                    const char* enc_data, uint16_t enc_len, 
                    uint16_t fragmentno);

        uint8_t GetStreamID() const { return GetStreamID(0); }

        uint8_t GetStreamID(uint32_t* packet_no, uint16_t* fragno = NULL,
                            uint16_t* fragcnt = NULL, uint16_t* width = NULL,
                            uint16_t* height = NULL) const;
        uint32_t GetPacketNo() const;

        uint16_t GetFragmentNo() const;
        uint16_t GetFragmentCount() const;
        static const uint16_t INVALID_FRAGMENT_NO = 0xFFFF;

        bool GetVideoInfo(uint16_t& width, uint16_t& height) const;

        const char* GetEncodedData(uint16_t& packet_bytes) const;

        enum
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

    typedef VideoPacket VideoCapturePacket;
    typedef VideoPacket VideoFilePacket;

    typedef std::shared_ptr< VideoPacket > videopacket_t;
    
    //desktop block with pointer to data
    struct desktop_block
    {
        const char* block_data;
        uint16_t block_size;
    };
    //blockno -> block
    typedef std::map< uint16_t, desktop_block > map_block_t;
    //a fragmented block
    struct block_fragment
    {
        uint16_t block_no;
        uint8_t frag_no;
        uint8_t frag_cnt;
        const char* frag_data;
        uint16_t frag_size;
    };
    //blockno -> fragment
    typedef std::list<block_fragment> block_frags_t;
    //blockno -> set(block_nums)
    typedef std::map< uint16_t, std::set<uint16_t> > map_dup_blocks_t;
    //blockno -> set(block_nums)
    typedef std::multimap< uint16_t, std::set<uint16_t> > mmap_dup_blocks_t;
    typedef std::pair< uint16_t, std::set<uint16_t> > dup_block_pair_t;

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
        
        static const uint16_t INVALID_PACKET_INDEX = -1;

    private:
        uint16_t InitCommon(const map_block_t& blocks, 
                            const block_frags_t& fragments,
                            const mmap_dup_blocks_t& dup_blocks);

        enum
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

    typedef std::shared_ptr< DesktopPacket > desktoppacket_t;

    //Calc the size of fields used
#define DESKTOPPACKET_SESSIONUSAGE(new_session)                                 \
    ((new_session? (FIELDVALUE_PREFIX + 10) : (FIELDVALUE_PREFIX + 5)))
    //FIELDTYPE_SESSIONID_NEW | FIELDTYPE_SESSIONID_UPD

#define DESKTOPPACKET_DATAUSAGE(blocks_cnt, frags_cnt)             \
    (((blocks_cnt)? (FIELDVALUE_PREFIX + (blocks_cnt) * 3) + FIELDVALUE_PREFIX : 0) + \
    ((frags_cnt)? (FIELDVALUE_PREFIX + (frags_cnt) * 4) + FIELDVALUE_PREFIX : 0))
    //FIELDTYPE_BLOCKNUMS_AND_SIZES & FIELDTYPE_BLOCKS_DATA
    //FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES & FIELDTYPE_BLOCKS_FRAG_DATA

#define DESKTOPPACKET_BLOCKUSAGE(dup_blocks_cnt, total_blocks)               \
    ((dup_blocks_cnt)? (FIELDVALUE_PREFIX +                                    \
    (((((dup_blocks_cnt) * 2 + (total_blocks)) * 12) % 8)?                       \
        ((((dup_blocks_cnt) * 2 + (total_blocks)) * 12) / 8 + 1) :               \
        ((((dup_blocks_cnt) * 2 + (total_blocks)) * 12) / 8))) : 0)
    //FIELDTYPE_BLOCK_DUP

#define DESKTOPPACKET_BLOCKRANGEUSAGE(block_range_cnt)                       \
    ((block_range_cnt)? (FIELDVALUE_PREFIX +                                   \
    (((((block_range_cnt) * 3 * 12) % 8)? (((block_range_cnt) * 3 * 12) / 8 + 1) : \
    ((block_range_cnt) * 3 * 12) / 8))) : 0)
    //FIELDTYPE_BLOCK_DUP_RANGE

    
    //packetno -> packetno
    typedef std::map<uint16_t, uint16_t> packet_range_t;

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
            if (GetSessionInfo(0, &sessionid, 0))
                return sessionid;
            return sessionid;
         }

        bool GetPacketsAcked(std::set<uint16_t>& packets_ack) const;

    private:
        void InitCommon(const std::set<uint16_t>& packets_ack,
                        const packet_range_t& packet_range_ack);

        enum
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
       enum
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
            GetSessionCursor(0, &streamid, 0, 0);
            return streamid;
        }

        int16_t GetX() const
        {
            int16_t v = 0;
            GetSessionCursor(0, 0, &v, 0);
            return v;
        }
        
        int16_t GetY() const
        {
            int16_t v = 0;
            GetSessionCursor(0, 0, 0, &v);
            return v;
        }
            
        static const uint16_t INVALID_DEST_USERID = -1;

        uint16_t GetDestUserID() const;

    private:
       enum
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
        uint8_t GetPacketNo(bool* found = NULL) const;

        bool GetDesktopInput(std::vector<DesktopInput>& desktopinputs) const;
        std::vector<DesktopInput> GetDesktopInput() const;

    private:

       enum
        {
            //[sessionid(uint8_t), packetno(uint8_t), [[x(uint16_t), y(uint16_t), keycode(uint32_t), keystate(uint32_t)], ...
            FIELDTYPE_REMOTE_INPUT = FIELDTYPE_LAST+1, //send input to remote desktop
        };
    };

    typedef std::shared_ptr< DesktopInputPacket > desktopinput_pkt_t;

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
        uint8_t GetPacketNo(bool* found = NULL) const;

    private:
       enum
        {
            //[sessionid(uint8_t), packetno(uint8_t)]
            FIELDTYPE_DESKTOPINPUT_ACK = FIELDTYPE_LAST+1,
        };
    };


#ifdef ENABLE_ENCRYPTION

#define CRYPTKEY_SIZE 32

    template < typename PACKETTYPE, uint8_t PACKET_KIND_CRYPT, uint8_t PACKET_KIND_DECRYPTED >
    class CryptPacket : public FieldPacket
    {
        typedef std::unique_ptr< PACKETTYPE > decrypt_pkt_t;
    public:
        CryptPacket(const PACKETTYPE& p, const uint8_t* encryptkey);
        CryptPacket(const char* packet, uint16_t packet_size);
        CryptPacket(const FieldPacket& packet) : FieldPacket(packet) { assert(GetKind() == packet.GetKind()); }
        std::unique_ptr< PACKETTYPE > Decrypt(const uint8_t* decryptkey) const;

        enum
        {
            FIELDTYPE_CRYPTDATA = FIELDTYPE_LAST+1,
        };
    };

#include "PacketLayout.inl"

    typedef CryptPacket<VoicePacket, PACKET_KIND_VOICE_CRYPT, PACKET_KIND_VOICE> CryptVoicePacket;
    typedef CryptPacket<AudioFilePacket, PACKET_KIND_MEDIAFILE_AUDIO_CRYPT, PACKET_KIND_MEDIAFILE_AUDIO> CryptAudioFilePacket;

    typedef CryptPacket<VideoCapturePacket, PACKET_KIND_VIDEO_CRYPT, PACKET_KIND_VIDEO> CryptVideoCapturePacket;
    typedef CryptPacket<VideoFilePacket, PACKET_KIND_MEDIAFILE_VIDEO_CRYPT, PACKET_KIND_MEDIAFILE_VIDEO> CryptVideoFilePacket;

    typedef CryptPacket<DesktopPacket, PACKET_KIND_DESKTOP_CRYPT, PACKET_KIND_DESKTOP> CryptDesktopPacket;

    typedef CryptPacket<DesktopAckPacket, PACKET_KIND_DESKTOP_ACK_CRYPT, PACKET_KIND_DESKTOP_ACK> CryptDesktopAckPacket;

    typedef CryptPacket<DesktopNakPacket, PACKET_KIND_DESKTOP_NAK_CRYPT, PACKET_KIND_DESKTOP_NAK> CryptDesktopNakPacket;
    
    typedef CryptPacket<DesktopCursorPacket, PACKET_KIND_DESKTOPCURSOR_CRYPT, PACKET_KIND_DESKTOPCURSOR> CryptDesktopCursorPacket;

    typedef CryptPacket<DesktopInputPacket, PACKET_KIND_DESKTOPINPUT_CRYPT, PACKET_KIND_DESKTOPINPUT> CryptDesktopInputPacket;

    typedef CryptPacket<DesktopInputAckPacket, PACKET_KIND_DESKTOPINPUT_ACK_CRYPT, PACKET_KIND_DESKTOPINPUT_ACK> CryptDesktopInputAckPacket;
    
#endif
}
#endif
