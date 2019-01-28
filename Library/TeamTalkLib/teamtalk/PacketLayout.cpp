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

#include "PacketLayout.h"

using namespace std;

namespace teamtalk
{
    void ConvertToUInt12Array(const std::vector<uint16_t>& source,
                              std::vector<uint8_t>& target)
    {
        std::vector<uint8_t>::size_type target_size = 0;
        if(source.size() % 2 == 1)
            target_size = source.size() * 12 / 8 + 1;
        else
            target_size = source.size() * 12 / 8;

        target.resize(target_size);

        uint8_t* target_ptr = &target[0];
        for(size_t i=0;i<source.size();)
        {
            if(source.size()-i >= 2)
            {
                set2_uint12_ptr(target_ptr, source[i], source[i+1], target_ptr);
                i += 2;
            }
            else
            {
                set_uint12_ptr(target_ptr, source[i], target_ptr);
                i += 1;
            }
        }
        assert(target_ptr == (&target[0])+target_size);
    }


    void ConvertFromUInt12Array(const uint8_t* source,
                                uint16_t source_size,
                                std::vector<uint16_t>& target)
    {
        target.reserve(source_size);

        for(uint16_t i=0;i<source_size;)
        {
            uint16_t v1, v2;
            const uint8_t* ptr = &source[i];
            if(source_size - i >= 3)
            {
                get2_uint12(ptr, v1, v2);
                target.push_back(v1);
                target.push_back(v2);
                i += 3;
            }
            else if(source_size - i == 2)
            {
                get_uint12(ptr, v1);
                target.push_back(v1);
                i += 2;
            }
            else assert(0);
        }
    }

    bool ReadUInt12Array(const uint8_t* ptr, uint8_t field_type, 
                         std::vector<uint16_t>& output)
    {
        uint16_t field_size = READFIELD_SIZE(ptr);
        if(!field_size)
            return false;
        const uint8_t* field_ptr = READFIELD_DATAPTR(ptr);
        ConvertFromUInt12Array(field_ptr, field_size, output);
        return true;
    }

    void WriteUInt12ArrayToIOVec(const std::vector<uint16_t>& input,
                                 uint8_t field_type,
                                 std::vector<iovec>& out_iovec)
    {
        std::vector<uint8_t> field_data;
        ConvertToUInt12Array(input, field_data);

        //new field
        int alloc_size = 0;
        alloc_size += FIELDVALUE_PREFIX + int(field_data.size());

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        uint8_t* data_ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        WRITEFIELD_DATA(data_ptr, field_type, &field_data[0],
            field_data.size(), data_ptr);

        assert(alloc_size == data_ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
        out_iovec.push_back(v);
    }

    bool ReadUInt16Array(const uint8_t* ptr, uint8_t field_type, 
                         std::vector<uint16_t>& output)
    {
        uint16_t field_size = READFIELD_SIZE(ptr);
        if(!field_size || (field_size % 2))
            return false;

        const uint8_t* field_ptr = READFIELD_DATAPTR(ptr);
        for(uint16_t i=0;i<field_size;i+=2)
        {
            output.push_back(get_uint16(field_ptr));
            field_ptr += 2;
        }
        return true;
    }

    void WriteUInt16ArrayToIOVec(const std::vector<uint16_t>& input,
                                 uint8_t field_type,
                                 std::vector<iovec>& out_iovec)
    {
        std::vector<uint8_t> field_data(input.size()*sizeof(uint16_t));
        uint8_t* field_ptr = &field_data[0];
        for(size_t i=0;i<input.size();i++)
            set_uint16_ptr(field_ptr, input[i], field_ptr);

        //new field
        int alloc_size = 0;
        alloc_size += FIELDVALUE_PREFIX + int(field_data.size());

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        uint8_t* data_ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        WRITEFIELD_DATA(data_ptr, field_type, &field_data[0],
                        field_data.size(), data_ptr);

        assert(alloc_size == data_ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
        out_iovec.push_back(v);
    }

    uint16_t GetHdrSize(PacketHdrType hdr_type)
    {
        switch(hdr_type)
        {
        default :
        case PACKETHDR_CHANNEL_ONLY :
            return TT_CHANNEL_HEADER_SIZE;
        case PACKETHDR_DEST_USER :
            return TT_USER_HEADER_SIZE;
        }
    }

    uint8_t FieldPacket::GetKind() const
    {
        int cnt;
        const iovec* buf = GetPacket(cnt);
        assert(buf);
        assert(buf[0].iov_len >= TT_CHANNEL_HEADER_SIZE);
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buf[0].iov_base);
        return ptr[PACKET_INDEX_KIND] & PACKET_MASK_KIND;
    }

    void FieldPacket::GetSrcDest(uint16_t& src_userid, uint16_t& dest_chanid) const
    {
        int cnt;
        const iovec* buf = GetPacket(cnt);
        assert(buf);
        assert(buf[0].iov_len >= TT_CHANNEL_HEADER_SIZE);
        const uint8_t* ptr = &reinterpret_cast<const uint8_t*>(buf[0].iov_base)[PACKET_INDEX_SRC_DEST];
        get2_uint12(ptr, src_userid, dest_chanid);
    }

    uint8_t* FieldPacket::GetFieldsStart() const
    {
        switch(GetHdrType())
        {
        case PACKETHDR_CHANNEL_ONLY :
            return &reinterpret_cast<uint8_t*>(m_iovec[0].iov_base)[PACKET_INDEX_FIELDS_DEST_CHANNEL];
        case PACKETHDR_DEST_USER :
            return &reinterpret_cast<uint8_t*>(m_iovec[0].iov_base)[PACKET_INDEX_FIELDS_DEST_USER_SET];
        default :
            assert(0);
            return NULL;
        }
    }
    
    uint16_t FieldPacket::GetSrcUserID() const
    {
        uint16_t src = 0, dest = 0;
        GetSrcDest(src, dest);
        return src;
    }

    uint16_t FieldPacket::GetDestUserID() const
    {
        int cnt;
        const iovec* buf = GetPacket(cnt);
        assert(buf);

        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buf[0].iov_base);
        uint8_t kind = ptr[PACKET_INDEX_DEST_USER_SET];
        if((kind & PACKET_MASK_DEST_USER_SET) == 0)
            return 0;
        uint16_t dest_userid = 0;
        ptr = &ptr[PACKET_INDEX_DEST_USER];
        get_uint12(ptr, dest_userid);
        return dest_userid;
    }
    
    uint16_t FieldPacket::GetChannel() const
    {
        uint16_t src = 0, dest = 0;
        GetSrcDest(src, dest);
        return dest;
    }

    uint32_t FieldPacket::GetTime() const
    {
        int cnt;
        const iovec* buf = GetPacket(cnt);
        assert(buf);
        const uint8_t* ptr = &reinterpret_cast<const uint8_t*>(buf[0].iov_base)[PACKET_INDEX_TIME];
        return get_uint32(ptr);
    }

    void FieldPacket::SetDestUser(uint16_t userid)
    {
        assert(userid);
        assert(m_iovec.size());
        assert(GetHdrType() == PACKETHDR_DEST_USER);
        uint8_t* ptr = &reinterpret_cast<uint8_t*>(m_iovec[0].iov_base)[PACKET_INDEX_DEST_USER];
        set_uint12(ptr, userid);
        assert(GetDestUserID() == userid);
    }

    void FieldPacket::SetChannel(uint16_t channelid)
    {
        assert(channelid);
        assert(m_iovec.size());
        uint8_t* ptr = &reinterpret_cast<uint8_t*>(m_iovec[0].iov_base)[PACKET_INDEX_KIND];

        uint16_t src_userid, tmp;
        GetSrcDest(src_userid, tmp);
        ptr = &ptr[PACKET_INDEX_SRC_DEST];
        set2_uint12(ptr, src_userid, channelid);
    }

    uint16_t FieldPacket::GetPacketSize() const
    {
        int buffers;
        int size = 0;
        const iovec* vv = GetPacket(buffers);
        for(int i=0;i<buffers;i++)
            size += vv[i].iov_len;
        return size;
    }

    bool FieldPacket::ValidatePacket() const
    {
        int buffers = 0;
        int size = GetPacketSize();
        const iovec* vv = GetPacket(buffers);
        //assert(vv);
        //assert(size >= MINIMUM_PACKET_SIZE);
        if(!vv || size < GetHdrSize(GetHdrType()))
            return false;

        if(size == GetHdrSize(GetHdrType()))
            return true;

        if(GetHdrSize(GetHdrType()) + FIELDVALUE_PREFIX >= size)
            return false;

        int pos_buf;
        if(GetHdrType() == PACKETHDR_DEST_USER)
            pos_buf = PACKET_INDEX_FIELDS_DEST_USER_SET;
        else
            pos_buf = PACKET_INDEX_FIELDS_DEST_CHANNEL;

        int pos = 0;
        for(int i=0;i<buffers;i++)
        {
            while(pos_buf < (int)vv[i].iov_len)
            {
                uint16_t field_size = READFIELD_SIZE(&reinterpret_cast<const uint8_t*>(vv[i].iov_base)[pos_buf]);
                pos_buf += field_size + FIELDVALUE_PREFIX;
            }
            //assert(pos_buf == vv[i].iov_len);
            if(pos_buf != vv[i].iov_len)
                return false;
            pos += pos_buf;
            //assert(pos <= size);
            if(pos > size)
                return false;
            pos_buf = 0;
        }
        //assert(pos == size);
        return pos == size;
    }

    void FieldPacket::Init(PacketHdrType hdr_type, uint8_t kind, uint16_t src_userid, uint32_t time)
    {
        m_iovec.reserve(16);

        int HDR_SIZE = GetHdrSize(hdr_type);

        uint8_t* packet_hdr;
        ACE_NEW(packet_hdr, uint8_t[HDR_SIZE]);
        m_cleanup = true;

        if(hdr_type == PACKETHDR_DEST_USER)
        {
            packet_hdr[PACKET_INDEX_KIND] = kind | PACKET_MASK_DEST_USER_SET;
            //init to zero
            uint8_t* ptr = &packet_hdr[PACKET_INDEX_DEST_USER];
            set_uint12(ptr, 0);
        }
        else
            packet_hdr[PACKET_INDEX_KIND] = kind;

        set2_uint12(&packet_hdr[PACKET_INDEX_SRC_DEST], src_userid, (uint16_t)0);
        set_uint32(&packet_hdr[PACKET_INDEX_TIME], time);

        iovec v;
        v.iov_base = reinterpret_cast<char*>(packet_hdr);
        v.iov_len = HDR_SIZE;
        m_iovec.push_back(v);

        //ensure compatibility with CryptPacket
        assert(m_iovec.size() == 1);
    }

    bool FieldPacket::Finalized() const
    {
        uint16_t src_userid = 0, chanid = 0;
        GetSrcDest(src_userid, chanid);
        return chanid>0;
    }

    PacketHdrType FieldPacket::GetHdrType() const
    {
        int buffers = 0;

        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(m_iovec[0].iov_base);
        if(ptr[PACKET_INDEX_DEST_USER_SET] & PACKET_MASK_DEST_USER_SET)
            return PACKETHDR_DEST_USER;
        else
            return PACKETHDR_CHANNEL_ONLY;
    }

    FieldPacket::FieldPacket(PacketHdrType hdr_type, uint8_t kind, uint16_t src_userid, uint32_t time)
    {
        Init(hdr_type, kind, src_userid, time);
    }

    FieldPacket::FieldPacket(const char* packet, uint16_t packet_size)
        : m_iovec(1)
    {
        m_cleanup = false;
        m_iovec[0].iov_base = const_cast<char*>(packet);
        m_iovec[0].iov_len = packet_size;

        //TODO: validate that all fields are within bounds
        //set everything to 0 if the packet size is greater than what has been received
    }

    FieldPacket::FieldPacket(const iovec* v, uint16_t buffers)
    {
        for(size_t i=0;i<buffers;i++)
            m_iovec.push_back(v[i]);

        m_cleanup = false;
    }

    FieldPacket::FieldPacket(const FieldPacket& p)
    {
        int buffers = 0;
        const iovec* v = p.GetPacket(buffers);
        for(int i=0;i<buffers;i++)
        {
            iovec new_v;
            ACE_NEW(new_v.iov_base, char[v[i].iov_len]);
            memcpy(new_v.iov_base, v[i].iov_base, v[i].iov_len);
            new_v.iov_len = v[i].iov_len;
            m_iovec.push_back(new_v);
        }
#ifdef ENABLE_ENCRYPTION
        m_crypt_sections = p.GetCryptSections();
#endif
        m_cleanup = true;

        assert(p.GetKind() == GetKind()); //cannot copy packet of different kind
    }

    FieldPacket::FieldPacket(uint8_t kind, const FieldPacket& crypt_pkt,
                             iovec& decrypt_fields)
    {
        //helper for encrypted packet
        Init(crypt_pkt.GetHdrType(), kind, crypt_pkt.GetSrcUserID(),
             crypt_pkt.GetTime());

        //transfer FieldPacket settings
        if(crypt_pkt.GetDestUserID())
            SetDestUser(crypt_pkt.GetDestUserID());
        if(crypt_pkt.GetChannel())
            SetChannel(crypt_pkt.GetChannel());

        assert(m_iovec.size() == 1);
        m_iovec.push_back(decrypt_fields);
    }

    FieldPacket::~FieldPacket()
    {
        if(m_cleanup)
        {
            for(size_t i=0;i<m_iovec.size();i++)
                delete [] reinterpret_cast<uint8_t*>(m_iovec[i].iov_base);
        }
    }

    uint8_t* FieldPacket::FindField_NonConst(uint8_t fieldtype) const
    {
        uint8_t* ptr = NULL;
        if(m_iovec.size())
        {
            if(GetPacketSize() == GetHdrSize(GetHdrType()))
                return NULL;

            int size = int(GetFieldsStart() - reinterpret_cast<uint8_t*>(m_iovec[0].iov_base));
            size = m_iovec[0].iov_len - size;
            ptr = GetFieldsStart();
            FINDFIELD_TYPE(ptr, fieldtype, size, ptr);
        }
        if(m_iovec.size() > 1 && !ptr)
        {
            for(size_t i=1;i<m_iovec.size() && !ptr;i++)
                FINDFIELD_TYPE(reinterpret_cast<uint8_t*>(m_iovec[i].iov_base), 
                    fieldtype, m_iovec[i].iov_len, ptr);
        }
        return ptr;
    }

    const uint8_t* FieldPacket::FindField(uint8_t fieldtype) const
    {
        return FindField_NonConst(fieldtype);
    }

    const iovec* FieldPacket::GetPacket(int& buffers) const
    {
        buffers = (int)m_iovec.size();
        if(buffers)
            return &m_iovec[0];
        return NULL;
    }

    HelloPacket::HelloPacket(uint16_t src_userid, uint32_t time) 
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, PACKET_KIND_HELLO, src_userid, time)
    {
        vector<uint8_t> protocol(1);
        protocol[0] = TEAMTALK_PACKET_PROTOCOL;

        int alloc_size = int(FIELDVALUE_PREFIX + protocol.size()); //FIELDTYPE_PAYLOAD

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);
        
        uint8_t* ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);

        WRITEFIELD_DATA(ptr, FIELDTYPE_PROTOCOL, &protocol[0], protocol.size(), ptr);

        v.iov_len = (u_long)(ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
        assert(v.iov_len == alloc_size);

        assert(m_iovec.size());

        m_iovec.push_back(v);
    }

    uint8_t HelloPacket::GetProtocol() const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_PROTOCOL);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            return get_uint8(ptr);
        }
        return 0;
    }

    /* KeepAlivePacket */

    KeepAlivePacket::KeepAlivePacket(uint16_t src_userid, uint32_t time, 
                               uint16_t payload_size)
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, PACKET_KIND_KEEPALIVE, src_userid, time)
    {
        vector<char> payload(payload_size);

        int alloc_size = FIELDVALUE_PREFIX + payload_size; //FIELDTYPE_PAYLOAD

        char* data_buf;
        ACE_NEW(data_buf, char[alloc_size]);
        
        char* ptr = data_buf;
        iovec v;
        v.iov_base = data_buf;

        WRITEFIELD_DATA(ptr, FIELDTYPE_PAYLOAD, &payload[0], payload_size, ptr);

        v.iov_len = (u_long)(ptr - static_cast<const char*>(v.iov_base));
        assert(v.iov_len == alloc_size);

        assert(m_iovec.size());

        m_iovec.push_back(v);
    }

    uint16_t KeepAlivePacket::GetPayloadSize() const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_PAYLOAD);
        if(ptr)
            return READFIELD_SIZE(ptr);
        return 0;
    }

    /* AudioPacket */

    AudioPacket::AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time,
                             uint8_t stream_id, uint16_t packet_no, 
                             const char* enc_audio, uint16_t enc_length)
            : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        InitCommon(stream_id, packet_no, NULL, NULL, enc_audio, enc_length, NULL);
    }

    AudioPacket::AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                             uint8_t stream_id, uint16_t packet_no, 
                             uint8_t frag_no, uint8_t* frag_cnt,
                             const char* enc_audio, uint16_t enc_length)
            : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        InitCommon(stream_id, packet_no, &frag_no, frag_cnt,
                   enc_audio, enc_length, NULL);
    }


    AudioPacket::AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                             uint8_t stream_id, uint16_t packet_no, 
                             const char* enc_audio, uint16_t enc_length, 
                             const std::vector<uint16_t>& enc_framesizes)
            : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        InitCommon(stream_id, packet_no, NULL, NULL, enc_audio, enc_length, &enc_framesizes);
    }

    AudioPacket::AudioPacket(uint8_t kind, uint16_t src_userid, uint32_t time, uint8_t stream_id, 
                    uint16_t packet_no, uint8_t frag_no, uint8_t* frag_cnt, 
                    const char* enc_audio, uint16_t enc_length, const std::vector<uint16_t>* enc_framesizes)
            : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        InitCommon(stream_id, packet_no, &frag_no, frag_cnt, enc_audio, 
                   enc_length, enc_framesizes);
    }

    void AudioPacket::InitCommon(uint8_t stream_id, uint16_t packet_no, 
                                 uint8_t* frag_no, uint8_t* frag_cnt, 
                                 const char* enc_audio, uint16_t enc_length,
                                 const std::vector<uint16_t>* enc_framesizes)
    {
        int alloc_size = 0;

        std::vector<uint8_t> stream_field;
        if(frag_no)
        {
            //FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT || FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO
            uint16_t field_size =  sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t);
            alloc_size += field_size + FIELDVALUE_PREFIX;
            stream_field.resize(field_size);
        }
        else
        {
            uint16_t field_size = sizeof(uint8_t) + sizeof(uint16_t); //FIELDTYPE_STREAMID_PKTNUM
            alloc_size += field_size + FIELDVALUE_PREFIX;
            stream_field.resize(field_size);
        }

        alloc_size += FIELDVALUE_PREFIX + enc_length; //FIELDTYPE_ENCDATA

        //encoded frame sizes are stored in a 12-bit array
        std::vector<char>::size_type enc_array_size = 0;
        assert(!enc_framesizes || (enc_framesizes && enc_framesizes->size()));
        if(enc_framesizes && enc_framesizes->size())
        {
            if(enc_framesizes->size() % 2 == 1)
                enc_array_size = (enc_framesizes->size() * 12 / 8) + 1;
            else
                enc_array_size = (enc_framesizes->size() * 12 / 8);
            alloc_size += int(FIELDVALUE_PREFIX + enc_array_size);
        }

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);
        //store data indexes
        uint8_t* ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        WRITEFIELD_DATA(ptr, FIELDTYPE_ENCDATA, enc_audio, enc_length, ptr);

        assert(!frag_cnt || !frag_no || frag_cnt && *frag_no == 0);
        uint8_t* field_buf_ptr = &stream_field[0];
        if(frag_cnt)
        {
            set_uint8_ptr(field_buf_ptr, stream_id, field_buf_ptr);
            set_uint16_ptr(field_buf_ptr, packet_no, field_buf_ptr);
            set_uint8_ptr(field_buf_ptr, *frag_cnt, field_buf_ptr);
            WRITEFIELD_DATA(ptr, FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT, 
                            &stream_field[0], stream_field.size(), ptr);
        }
        else if(frag_no)
        {
            set_uint8_ptr(field_buf_ptr, stream_id, field_buf_ptr);
            set_uint16_ptr(field_buf_ptr, packet_no, field_buf_ptr);
            set_uint8_ptr(field_buf_ptr, *frag_no, field_buf_ptr);
            WRITEFIELD_DATA(ptr, FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO, 
                            &stream_field[0], stream_field.size(), ptr);
        }
        else
        {
            set_uint8_ptr(field_buf_ptr, stream_id, field_buf_ptr);
            set_uint16_ptr(field_buf_ptr, packet_no, field_buf_ptr);
            WRITEFIELD_DATA(ptr, FIELDTYPE_STREAMID_PKTNUM, 
                            &stream_field[0], stream_field.size(), ptr);
        }
        
        if(enc_framesizes && enc_framesizes->size())
        {
            const std::vector<uint16_t>& frm_sizes = *enc_framesizes;
            std::vector<uint8_t> enc_array;
            ConvertToUInt12Array(frm_sizes, enc_array);
            assert(enc_array.size() == enc_array_size);

            WRITEFIELD_DATA(ptr, FIELDTYPE_ENCFRAMESIZES, 
                            &enc_array[0], enc_array.size(), ptr);
        }

        v.iov_len = (u_long)(ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
        assert(v.iov_len == alloc_size);

        //CryptPacket will become incompatible if m_iovec[1] doesn't contain 
        //the part which needs to be encrypted
        assert(m_iovec.size() == 1);

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
    }


    AudioPacket::AudioPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size)
    {
    }

    AudioPacket::AudioPacket(const AudioPacket& packet)
        : FieldPacket(packet)
    {
    }

    bool AudioPacket::GetStreamField(uint8_t& streamid, uint16_t& packet_no,
                                     uint8_t& frag_no, uint8_t* frag_cnt) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_STREAMID_PKTNUM);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t) + sizeof(uint16_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            get_uint8_ptr(streamid, ptr, ptr);
            get_uint16_ptr(packet_no, ptr, ptr);
            frag_no = INVALID_FRAGMENT_NO;
            return true;
        }
        ptr =  FindField(FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            get_uint8_ptr(streamid, ptr, ptr);
            get_uint16_ptr(packet_no, ptr, ptr);
            if(frag_cnt)
                get_uint8_ptr(*frag_cnt, ptr, ptr); //this is frag count
            frag_no = 0;
            return true;
        }
        ptr =  FindField(FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint8_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            get_uint8_ptr(streamid, ptr, ptr);
            get_uint16_ptr(packet_no, ptr, ptr);
            get_uint8_ptr(frag_no, ptr, ptr);
            return true;
        }
        return false;
    }

    uint8_t AudioPacket::GetStreamID() const
    {
        uint8_t streamid = 0; uint16_t packet_no = 0;
        uint8_t frag_no = INVALID_FRAGMENT_NO;
        if(GetStreamField(streamid, packet_no, frag_no, NULL))
            return streamid;
        return 0;
    }

    uint16_t AudioPacket::GetPacketNumber() const
    {
        //only applies to FIELDTYPE_STREAMID_PKTNUM (to distinguish fragments)
        if(FindField(FIELDTYPE_STREAMID_PKTNUM) == NULL)
            return 0;

        uint8_t streamid = 0; uint16_t packet_no = 0;
        uint8_t frag_no = INVALID_FRAGMENT_NO;
        if(GetStreamField(streamid, packet_no, frag_no, NULL))
            return packet_no;
        return 0;
    }

    uint16_t AudioPacket::GetPacketNumberAndFragNo(uint8_t& frag_no,
                                                   uint8_t* frag_cnt) const
    {
        if(FindField(FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO) == NULL &&
           FindField(FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT) == NULL)
            return 0;

        uint8_t streamid = 0; uint16_t packet_no = 0;
        frag_no = INVALID_FRAGMENT_NO;
        if(GetStreamField(streamid, packet_no, frag_no, frag_cnt))
            return packet_no;
        return 0;
    }

    const char* AudioPacket::GetEncodedAudio(uint16_t& length) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_ENCDATA);
        if(ptr)
        {
            length = READFIELD_SIZE(ptr);
            ptr = READFIELD_DATAPTR(ptr);
        }
        else length = 0;
        return reinterpret_cast<const char*>(ptr);
    }

    std::vector<uint16_t> AudioPacket::GetEncodedFrameSizes() const
    {
        std::vector<uint16_t> v_frm_sizes;

        const uint8_t* ptr = FindField(FIELDTYPE_ENCFRAMESIZES);
        if(!ptr)
            return v_frm_sizes;

        uint16_t length = READFIELD_SIZE(ptr);
        ptr = READFIELD_DATAPTR(ptr);

        const uint8_t* frm_sizes = reinterpret_cast<const uint8_t*>(ptr);

        ConvertFromUInt12Array(frm_sizes, length, v_frm_sizes);

        return v_frm_sizes;
    }

    bool AudioPacket::HasFragments() const
    {
        return FindField(FIELDTYPE_STREAMID_PKTNUM_AND_FRAGNO) != NULL ||
            FindField(FIELDTYPE_STREAMID_PKTNUM_AND_FRAGCNT) != NULL;
    }

    VideoPacket::VideoPacket(const VideoPacket& p)
        : FieldPacket(p)
    {
    }

    VideoPacket::VideoPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size)
    {
    }

    VideoPacket::VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time, 
                             uint8_t stream_id, uint32_t packet_no, 
                             const uint16_t* width, const uint16_t* height,
                             const char* enc_data, uint16_t enc_len) 
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        //CryptPacket will become incompatible if m_iovec[1] doesn't contain 
        //the part which needs to be encrypted
        assert(m_iovec.size() == 1);

        Init(kind, stream_id, packet_no, width, height, enc_data, enc_len, NULL, NULL);

        assert(m_iovec.size() == 2); //CryptPacket compatibility
    }

    VideoPacket::VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time,
                             uint8_t stream_id, uint32_t packet_no, 
                             const uint16_t* width, const uint16_t* height,
                             const char* enc_data, uint16_t enc_len, 
                             const uint16_t fragmentcnt) 
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        //CryptPacket will become incompatible if m_iovec[1] doesn't contain 
        //the part which needs to be encrypted
        assert(m_iovec.size() == 1);

        Init(kind, stream_id, packet_no, width, height, enc_data, enc_len, 
             NULL, &fragmentcnt);

        assert(m_iovec.size() == 2); //CryptPacket compatibility
    }

    VideoPacket::VideoPacket(uint8_t kind, uint16_t src_userid, uint32_t time,
                             uint8_t stream_id, uint32_t packet_no, 
                             const char* enc_data, uint16_t enc_len, 
                             uint16_t fragmentno)
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, kind, src_userid, time)
    {
        //CryptPacket will become incompatible if m_iovec[1] doesn't contain 
        //the part which needs to be encrypted
        assert(m_iovec.size() == 1);

        Init(kind, stream_id, packet_no, NULL, NULL, enc_data, enc_len, &fragmentno, NULL);

        assert(m_iovec.size() == 2); //CryptPacket compatibility
    }

    uint8_t* VideoPacket::Init(uint8_t kind, uint8_t stream_id, uint32_t packet_no,
                               const uint16_t* width, const uint16_t* height,
                               const char* enc_data, uint16_t enc_len, 
                               const uint16_t* fragmentno, const uint16_t* fragmentcnt)
    {
        assert(FindField(FIELDTYPE_STREAMID_PKTNUM_VIDINFO) == NULL);
        assert(FindField(FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO) == NULL);
        assert(FindField(FIELDTYPE_STREAMID_PKTNUM) == NULL);
        assert(FindField(FIELDTYPE_STREAMID_PKTNUM_FRAGCNT) == NULL);
        assert(FindField(FIELDTYPE_STREAMID_PKTNUM_FRAGNO) == NULL);
        assert(FindField(FIELDTYPE_ENCDATA) == NULL);

        assert(stream_id);
        assert(width && height || !width && !height);

        int field_size = 0;
        int field_type = 0;
        if(width && height && fragmentcnt) //FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO
        {
            field_type = FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO;
            field_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t) + (12+12) / 8;
            assert(!fragmentno || *fragmentno == 0);
        }
        else if(width && height) //FIELDTYPE_STREAMID_PKTNUM_VIDINFO
        {
            field_type = FIELDTYPE_STREAMID_PKTNUM_VIDINFO;
            field_size = sizeof(uint8_t) + sizeof(uint32_t) + (12 + 12) / 8;
            assert(!fragmentno && !fragmentcnt);
        }
        else if(fragmentcnt) //FIELDTYPE_STREAMID_PKTNUM_FRAGCNT
        {
            field_type = FIELDTYPE_STREAMID_PKTNUM_FRAGCNT;
            field_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t);
            assert(!fragmentno || *fragmentno == 0);
        }
        else if(fragmentno) //FIELDTYPE_STREAMID_PKTNUM_FRAGNO
        {
            field_type = FIELDTYPE_STREAMID_PKTNUM_FRAGNO;
            field_size = sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t);
            assert(!fragmentcnt);
        }
        else //FIELDTYPE_STREAMID_PKTNUM
        {
            field_type = FIELDTYPE_STREAMID_PKTNUM;
            field_size = sizeof(uint8_t) + sizeof(uint32_t);
            assert(!fragmentcnt && !fragmentno);
            assert(!width && !height);
        }

        int alloc_size = FIELDVALUE_PREFIX + field_size + FIELDVALUE_PREFIX + enc_len;

        uint8_t* data_buf;
        ACE_NEW_RETURN(data_buf, uint8_t[alloc_size], NULL);

        //store data indexes
        uint8_t* ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        std::vector<uint8_t> field(field_size);
        uint8_t* field_ptr = &field[0];
        switch(field_type)
        {
        case FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO :
            assert(width && height && fragmentcnt);
            set_uint8_ptr(field_ptr, stream_id, field_ptr);
            set_uint32_ptr(field_ptr, packet_no, field_ptr);
            set_uint16_ptr(field_ptr, *fragmentcnt, field_ptr);
            set2_uint12_ptr(field_ptr, *width, *height, field_ptr);
            break;
        case FIELDTYPE_STREAMID_PKTNUM_VIDINFO :
            assert(width && height);
            set_uint8_ptr(field_ptr, stream_id, field_ptr);
            set_uint32_ptr(field_ptr, packet_no, field_ptr);
            set2_uint12_ptr(field_ptr, *width, *height, field_ptr);
            break;
        case FIELDTYPE_STREAMID_PKTNUM_FRAGCNT :
            assert(fragmentcnt);
            set_uint8_ptr(field_ptr, stream_id, field_ptr);
            set_uint32_ptr(field_ptr, packet_no, field_ptr);
            set_uint16_ptr(field_ptr, *fragmentcnt, field_ptr);
            break;
        case FIELDTYPE_STREAMID_PKTNUM_FRAGNO :
            assert(fragmentno);
            set_uint8_ptr(field_ptr, stream_id, field_ptr);
            set_uint32_ptr(field_ptr, packet_no, field_ptr);
            set_uint16_ptr(field_ptr, *fragmentno, field_ptr);
            break;
        default :
            assert(0);
        case FIELDTYPE_STREAMID_PKTNUM :
            set_uint8_ptr(field_ptr, stream_id, field_ptr);
            set_uint32_ptr(field_ptr, packet_no, field_ptr);
            break;
        }

        WRITEFIELD_DATA(ptr, field_type, &field[0], field_size, ptr);
        WRITEFIELD_DATA(ptr, FIELDTYPE_ENCDATA, enc_data, enc_len, ptr);

        //int x = ptr - reinterpret_cast<const uint8_t*>(v.iov_base) ;
        assert(ptr - reinterpret_cast<const uint8_t*>(v.iov_base) == alloc_size);

        assert(m_iovec.size());

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif

        return ptr; //callee must continue to write from this position
    }

    uint8_t VideoPacket::GetStreamID(uint32_t* packet_no, uint16_t* fragno,
                                     uint16_t* fragcnt, uint16_t* width,
                                     uint16_t* height) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_STREAMID_PKTNUM_VIDINFO);
        if(!ptr) ptr = FindField(FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO);
        if(!ptr) ptr = FindField(FIELDTYPE_STREAMID_PKTNUM);
        if(!ptr) ptr = FindField(FIELDTYPE_STREAMID_PKTNUM_FRAGCNT);
        if(!ptr) ptr = FindField(FIELDTYPE_STREAMID_PKTNUM_FRAGNO);
        if(!ptr) return 0;

        uint8_t stream_id = 0;
        uint16_t u16_1, u16_2;
        uint32_t u32;

        uint16_t field_size = READFIELD_SIZE(ptr);
        uint8_t field_type = READFIELD_TYPE(ptr);
        ptr = READFIELD_DATAPTR(ptr);
        switch(field_type)
        {
        case FIELDTYPE_STREAMID_PKTNUM_VIDINFO :
            if(field_size < sizeof(uint8_t) + sizeof(uint32_t) + (12+12) / 8)
                return 0;
            //invalid parameters for this field
            if(fragno || fragcnt)
                return 0;
            //stream id
            get_uint8_ptr(stream_id, ptr, ptr);
            //packet no
            get_uint32_ptr(u32, ptr, ptr);
            if(packet_no)
                *packet_no = u32;
            //width & height
            get2_uint12_ptr(ptr, u16_1, u16_2, ptr);
            if(width)
                *width = u16_1;
            if(height)
                *height = u16_2;
            break;
        case FIELDTYPE_STREAMID_PKTNUM_FRAGCNT_VIDINFO :
            if(field_size < sizeof(uint8_t) + sizeof(uint32_t) + 
                sizeof(uint16_t) + (12+12) / 8)
                return 0;
            //stream id
            get_uint8_ptr(stream_id, ptr, ptr);
            //packet no
            get_uint32_ptr(u32, ptr, ptr);
            if(packet_no)
                *packet_no = u32;
            //fragment count
            get_uint16_ptr(u16_1, ptr, ptr);
            if(fragcnt)
                *fragcnt = u16_1;
            //fragment no
            if(fragno)
                *fragno = 0;
            //width & height
            get2_uint12_ptr(ptr, u16_1, u16_2, ptr);
            if(width)
                *width = u16_1;
            if(height)
                *height = u16_2;
            break;
        case FIELDTYPE_STREAMID_PKTNUM :
            if(field_size < sizeof(uint8_t) + sizeof(uint32_t))
                return 0;
            //invalid parameters for this field
            if(fragno || fragcnt || width || height)
                return 0;
            //stream id
            get_uint8_ptr(stream_id, ptr, ptr);
            //packet no
            get_uint32_ptr(u32, ptr, ptr);
            if(packet_no)
                *packet_no = u32;
            break;
        case FIELDTYPE_STREAMID_PKTNUM_FRAGCNT :
            if(field_size < sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t))
                return 0;
            //invalid parameters for this field
            if(width || height)
                return 0;
            //stream id
            get_uint8_ptr(stream_id, ptr, ptr);
            //packet no
            get_uint32_ptr(u32, ptr, ptr);
            if(packet_no)
                *packet_no = u32;
            //fragment cnt
            get_uint16_ptr(u16_1, ptr, ptr);
            if(fragcnt)
                *fragcnt = u16_1;
            //fragment no
            if(fragno)
                *fragno = 0;
            break;
        case FIELDTYPE_STREAMID_PKTNUM_FRAGNO :
            if(field_size < sizeof(uint8_t) + sizeof(uint32_t) + sizeof(uint16_t))
                return 0;
            //invalid parameters for this field
            if(fragcnt || width || height)
                return 0;
            //stream id
            get_uint8_ptr(stream_id, ptr, ptr);
            //packet no
            get_uint32_ptr(u32, ptr, ptr);
            if(packet_no)
                *packet_no = u32;
            get_uint16_ptr(u16_1, ptr, ptr);
            //fragment no
            if(fragno)
                *fragno = u16_1;
            break;
        }
        return stream_id;
    }

    uint32_t VideoPacket::GetPacketNo() const
    {
        uint32_t packet_no = 0;
        uint8_t streamid = GetStreamID(&packet_no);
        return packet_no;
    }

    uint16_t VideoPacket::GetFragmentNo() const
    {
        uint16_t fragno;
        if(!GetStreamID(0, &fragno))
            return INVALID_FRAGMENT_NO;
        return fragno;
    }

    uint16_t VideoPacket::GetFragmentCount() const
    {
        uint16_t fragcnt;
        if(GetStreamID(0, 0, &fragcnt))
            return fragcnt;
        return 0;
    }

    bool VideoPacket::GetVideoInfo(uint16_t& width, uint16_t& height) const
    {
        if(GetStreamID(0, 0, 0, &width, &height))
            return true;
        return false;
    }

    const char* VideoPacket::GetEncodedData(uint16_t& packet_bytes) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_ENCDATA);
        if(!ptr)
            return NULL;
        packet_bytes = READFIELD_SIZE(ptr);
        assert(packet_bytes>0);
        return reinterpret_cast<const char*>(READFIELD_DATAPTR(ptr));
    }

    bool IsBlockRange(const std::set<uint16_t>& blocks)
    {
        if(blocks.size()>1)
        {
            set<uint16_t>::const_iterator ii = blocks.end();
            ii--;
            return (*blocks.begin() + blocks.size() - 1 == *ii);
        }
        return false;
    }
    
    DesktopPacket::DesktopPacket(uint16_t src_userid, uint32_t time, 
                                 uint8_t stream_id, uint16_t width, 
                                 uint16_t height, uint8_t bmp_mode, 
                                 uint16_t pkt_upd_index, 
                                 uint16_t pkt_upd_count,
                                 const map_block_t& blocks,
                                 const block_frags_t& fragments,
                                 const mmap_dup_blocks_t& dup_blocks)
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, PACKET_KIND_DESKTOP, src_userid, time)
    {
        int alloc_size = 0;
        
        //FIELDTYPE_SESSIONID_NEW
        //[stream_id, width, height, bmp_mode, pkt_upd_index, pkt_upd_count]
        int field_size = sizeof(uint8_t) + sizeof(uint16_t) +
            sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t);

        alloc_size += FIELDVALUE_PREFIX + field_size;

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        vector<uint8_t> streamid_field((size_t)field_size);
        uint8_t* field_ptr = &streamid_field[0];

        set_uint8_ptr(field_ptr, stream_id, field_ptr);
        set_uint16_ptr(field_ptr, width, field_ptr);
        set_uint16_ptr(field_ptr, height, field_ptr);
        set_uint8(field_ptr, bmp_mode); field_ptr += sizeof(uint8_t);
        set_uint16_ptr(field_ptr, pkt_upd_index, field_ptr);
        set_uint16_ptr(field_ptr, pkt_upd_count, field_ptr); //change UpdatePacketCount() if changed

        WRITEFIELD_DATA(data_buf, FIELDTYPE_SESSIONID_NEW, &streamid_field[0], 
                        streamid_field.size(), data_buf);

        assert(reinterpret_cast<uint8_t*>(v.iov_base) == data_buf - alloc_size);
        
        assert(m_iovec.size());

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
        uint16_t fieldsize_alloced = InitCommon(blocks, fragments, dup_blocks);

#ifdef _DEBUG
        size_t data_size = 0;
        map_block_t::const_iterator bi;
        for(bi=blocks.begin();bi!= blocks.end();bi++)
            data_size += bi->second.block_size;
        block_frags_t::const_iterator fi;
        for(fi=fragments.begin();fi!=fragments.end();fi++)
            data_size += fi->frag_size;
        size_t range_blocks = 0, single_blocks = 0, single_entries = 0;
        mmap_dup_blocks_t::const_iterator dbi;
        for(dbi=dup_blocks.begin();dbi!=dup_blocks.end();dbi++)
        {
            if(IsBlockRange(dbi->second))
                range_blocks++;
            else
            {
                single_blocks += dbi->second.size();
                single_entries++;
            }
        }
        uint16_t fields = DESKTOPPACKET_SESSIONUSAGE(true);
        fields += DESKTOPPACKET_DATAUSAGE(uint16_t(blocks.size()), uint16_t(fragments.size()));
        fields += DESKTOPPACKET_BLOCKUSAGE(uint16_t(single_entries), uint16_t(single_blocks));
        fields += DESKTOPPACKET_BLOCKRANGEUSAGE(uint16_t(range_blocks));

        assert(fields + data_size == fieldsize_alloced + (uint16_t)v.iov_len);
        assert(GetPacketSize() <= MAX_PACKET_SIZE);
#endif
    }

    //Update session (first packet based on FIELDTYPE_SESSIONID_UPD)
    DesktopPacket::DesktopPacket(uint16_t src_userid, uint32_t time, 
                                 uint8_t session_id, uint16_t pkt_upd_index, 
                                 uint16_t pkt_upd_count, 
                                 const map_block_t& blocks, 
                                 const block_frags_t& fragments,
                                 const mmap_dup_blocks_t& dup_blocks)
        : FieldPacket(PACKETHDR_CHANNEL_ONLY, PACKET_KIND_DESKTOP, src_userid, time)
    {
        int alloc_size = 0;
        
        //FIELDTYPE_SESSIONID_UPD
        //[streamid, pkt_upd_index, pkt_upd_count]
        int field_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t);
        alloc_size += FIELDVALUE_PREFIX + field_size;

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        vector<uint8_t> streamid_field((size_t)field_size);
        uint8_t* field_ptr = &streamid_field[0];

        set_uint8_ptr(field_ptr, session_id, field_ptr);
        set_uint16_ptr(field_ptr, pkt_upd_index, field_ptr);
        set_uint16_ptr(field_ptr, pkt_upd_count, field_ptr); //change UpdatePacketCount() if changed

        WRITEFIELD_DATA(data_buf, FIELDTYPE_SESSIONID_UPD, 
                        &streamid_field[0], streamid_field.size(), data_buf);

        assert(reinterpret_cast<uint8_t*>(v.iov_base) == data_buf - alloc_size);

        assert(m_iovec.size());

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
        uint16_t fieldsize_alloced = InitCommon(blocks, fragments, dup_blocks);

#ifdef _DEBUG
        int data_size = 0;
        map_block_t::const_iterator bi;
        for(bi=blocks.begin();bi!= blocks.end();bi++)
            data_size += bi->second.block_size;
        block_frags_t::const_iterator fi;
        for(fi=fragments.begin();fi!=fragments.end();fi++)
            data_size += fi->frag_size;
        int range_blocks = 0, single_blocks = 0, single_entries = 0;
        mmap_dup_blocks_t::const_iterator dbi;
        for(dbi=dup_blocks.begin();dbi!=dup_blocks.end();dbi++)
        {
            if(IsBlockRange(dbi->second))
                range_blocks++;
            else
            {
                single_blocks += uint16_t(dbi->second.size());
                single_entries++;
            }
        }
        uint16_t fields = DESKTOPPACKET_SESSIONUSAGE(false);
        fields += DESKTOPPACKET_DATAUSAGE(uint16_t(blocks.size()), uint16_t(fragments.size()));
        fields += DESKTOPPACKET_BLOCKUSAGE(single_entries, single_blocks);
        fields += DESKTOPPACKET_BLOCKRANGEUSAGE(range_blocks);

        assert(fields + data_size == fieldsize_alloced + (uint16_t)v.iov_len);
        assert(GetPacketSize() <= MAX_PACKET_SIZE);
#endif
    }

    DesktopPacket::DesktopPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size)
    {
    }

    DesktopPacket::DesktopPacket(const DesktopPacket& packet)
        : FieldPacket(packet)
    {
    }

    bool DesktopPacket::UpdatePacketCount(uint16_t pkt_upd_count)
    {
        uint8_t* ptr = FindField_NonConst(FIELDTYPE_SESSIONID_NEW);
        if(ptr)
        {
            if(READFIELD_SIZE(ptr) >= sizeof(uint8_t) + sizeof(uint16_t) +
                sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t))
            {
                ptr = READFIELD_DATAPTR(ptr);
                ptr += 1; //session_id
                ptr += 2; //width
                ptr += 2; //height
                ptr += 1; //bmp_mode
                ptr += 2; //pkt_upd_index
                set_uint16(ptr, pkt_upd_count);
#ifdef _DEBUG
                uint8_t session_id;
                uint16_t pkt_index, pkt_count;
                if(GetSessionProperties(&session_id, NULL, NULL, NULL,
                                        &pkt_index, &pkt_count))
                    assert(pkt_count == pkt_upd_count);
                if(GetUpdateProperties(&session_id, &pkt_index, &pkt_count))
                    assert(pkt_count == pkt_upd_count);
#endif
                return true;
            }
            return false;
        }

        ptr = FindField_NonConst(FIELDTYPE_SESSIONID_UPD);
        if(!ptr)
            return false;

        if(READFIELD_SIZE(ptr) >= sizeof(uint8_t) + sizeof(uint16_t) +
            sizeof(uint16_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            ptr += 1; //session_id
            ptr += 2; //pkt_upd_index
            set_uint16(ptr, pkt_upd_count);
            return true;
        }
        return false;
    }


    uint16_t DesktopPacket::InitCommon(const map_block_t& blocks, 
                                       const block_frags_t& fragments,
                                       const mmap_dup_blocks_t& dup_blocks)
    {
        uint16_t alloced = 0;
        if(blocks.size())
        {
            int alloc_size = 0;
            int blocks_size = 0;

            std::vector<uint16_t> blocknums_sizes_input;

            map_block_t::const_iterator ii = blocks.begin();
            while(ii != blocks.end())
            {
                blocknums_sizes_input.push_back(ii->first); //block no
                blocknums_sizes_input.push_back(ii->second.block_size); //block size

                blocks_size += ii->second.block_size;
                ii++;
            }

            std::vector<uint8_t> blocknums_sizes_output;
            ConvertToUInt12Array(blocknums_sizes_input,
                                 blocknums_sizes_output);

            //FIELDTYPE_BLOCKNUMS_AND_SIZES
            alloc_size += int(FIELDVALUE_PREFIX + blocknums_sizes_output.size());

            //FIELDTYPE_BLOCKS_DATA
            alloc_size += FIELDVALUE_PREFIX + blocks_size;

            uint8_t* data_buf;
            ACE_NEW_RETURN(data_buf, uint8_t[alloc_size], alloced);
            
            uint8_t* data_ptr = data_buf;
            iovec v;
            v.iov_base = reinterpret_cast<char*>(data_buf);
            v.iov_len = alloc_size;

            //write FIELDTYPE_BLOCKNUMS_AND_SIZES
            WRITEFIELD_DATA(data_ptr, FIELDTYPE_BLOCKNUMS_AND_SIZES,
                            &blocknums_sizes_output[0],
                            blocknums_sizes_output.size(), data_ptr);

            //write FIELDTYPE_BLOCKS_DATA
            WRITEFIELD_TYPE(data_ptr, FIELDTYPE_BLOCKS_DATA,
                            blocks_size, data_ptr);
            
            ii = blocks.begin();
            while(ii != blocks.end())
            {
                memcpy(data_ptr, ii->second.block_data, ii->second.block_size);
                data_ptr += ii->second.block_size;
                ii++;
            }            

            assert(alloc_size == data_ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
            m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
            m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
            alloced += (uint16_t)v.iov_len;
        }

        if(fragments.size())
        {
            int alloc_size = 0;
            int frags_size = 0;

            //Build array for FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES
            //[[blockno(uint12_t), fragsize(uint12_t), fragno(uint4_t), frag_cnt(uint4_t)], ...] = 4 bytes
            std::vector<uint8_t> frags_info_output(4*fragments.size());
            uint8_t* frags_info_ptr = &frags_info_output[0];

            block_frags_t::const_iterator ii = fragments.begin();
            while(ii != fragments.end())
            {
                set2_uint12_ptr(frags_info_ptr, ii->block_no, ii->frag_size, frags_info_ptr);
                set_uint4_ptr(frags_info_ptr, ii->frag_no, ii->frag_cnt, frags_info_ptr);

                frags_size += ii->frag_size;
                ii++;
            }


            //FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES
            alloc_size += int(FIELDVALUE_PREFIX + frags_info_output.size());

            //FIELDTYPE_BLOCKS_FRAG_DATA
            alloc_size += FIELDVALUE_PREFIX + frags_size;

            uint8_t* data_buf;
            ACE_NEW_RETURN(data_buf, uint8_t[alloc_size], alloced);
            
            uint8_t* data_ptr = data_buf;
            iovec v;
            v.iov_base = reinterpret_cast<char*>(data_buf);
            v.iov_len = alloc_size;

            //FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES
            WRITEFIELD_DATA(data_ptr, FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES,
                            &frags_info_output[0], frags_info_output.size(),
                            data_ptr);

            //write FIELDTYPE_BLOCKS_FRAG_DATA
            WRITEFIELD_TYPE(data_ptr, FIELDTYPE_BLOCKS_FRAG_DATA,
                            frags_size, data_ptr);
            
            ii = fragments.begin();
            while(ii != fragments.end())
            {
                memcpy(data_ptr, ii->frag_data, ii->frag_size);
                data_ptr += ii->frag_size;
                ii++;
            }

            assert(alloc_size == data_ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
            m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
            m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
            alloced += (uint16_t)v.iov_len;
        }

        if(dup_blocks.size())
        {
            int alloc_size = 0;

            vector<uint16_t> blocknums_single_input, blocknums_range_input;
#ifdef _DEBUG
            size_t block_ranges = 0, single_entries = 0, single_blocks = 0;
#endif
            mmap_dup_blocks_t::const_iterator dbi = dup_blocks.begin();
            for(;dbi!=dup_blocks.end();dbi++)
            {
                assert(dbi->second.size());

                set<uint16_t>::const_iterator ii = dbi->second.begin();
                if(IsBlockRange(dbi->second))
                {
                    blocknums_range_input.push_back(dbi->first);
                    ii = dbi->second.begin();
                    blocknums_range_input.push_back(*ii);
                    ii = dbi->second.end();
                    ii--;
                    blocknums_range_input.push_back(*ii);
#ifdef _DEBUG
                    block_ranges++;
#endif
                }
                else
                {
                    blocknums_single_input.push_back(dbi->first);
                    for(;ii!=dbi->second.end();ii++)
                        blocknums_single_input.push_back(*ii);
                    blocknums_single_input.push_back(0xFFF);
#ifdef _DEBUG
                    single_blocks += dbi->second.size();
                    single_entries++;
#endif
                }
            }

            std::vector<uint8_t> blocknums_single_output, blocknums_range_output;
            if(blocknums_single_input.size())
            {
                ConvertToUInt12Array(blocknums_single_input,
                                     blocknums_single_output);

                //FIELDTYPE_BLOCK_DUP
                alloc_size += int(FIELDVALUE_PREFIX + blocknums_single_output.size());

            }
            if(blocknums_range_input.size())
            {
                ConvertToUInt12Array(blocknums_range_input,
                                     blocknums_range_output);

                //FIELDTYPE_BLOCK_DUP_RANGE
                alloc_size += int(FIELDVALUE_PREFIX + blocknums_range_output.size());
            }

            uint8_t* data_buf;
            ACE_NEW_RETURN(data_buf, uint8_t[alloc_size], alloced);
            
            uint8_t* data_ptr = data_buf;
            iovec v;
            v.iov_base = reinterpret_cast<char*>(data_buf);
            v.iov_len = alloc_size;

            if(blocknums_single_output.size())
            {
                //write FIELDTYPE_BLOCK_DUP
                WRITEFIELD_DATA(data_ptr, FIELDTYPE_BLOCK_DUP,
                                &blocknums_single_output[0],
                                blocknums_single_output.size(), data_ptr);
#ifdef _DEBUG
                size_t bytes = DESKTOPPACKET_BLOCKUSAGE(single_entries, single_blocks);
                assert(bytes == blocknums_single_output.size() + FIELDVALUE_PREFIX);
#endif
            }
            if(blocknums_range_output.size())
            {
                //write FIELDTYPE_BLOCK_DUP_RANGE
                WRITEFIELD_DATA(data_ptr, FIELDTYPE_BLOCK_DUP_RANGE,
                                &blocknums_range_output[0],
                                blocknums_range_output.size(), data_ptr);
#ifdef _DEBUG
                size_t bytes = DESKTOPPACKET_BLOCKRANGEUSAGE(block_ranges);
                assert(bytes == blocknums_range_output.size() + FIELDVALUE_PREFIX);
#endif
            }
            m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
            m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
            alloced += (uint16_t)v.iov_len;
        }
        return alloced;
    }

    uint8_t DesktopPacket::GetSessionID() const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_SESSIONID_NEW);
        if(!ptr)
            ptr = FindField(FIELDTYPE_SESSIONID_UPD);

        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            return get_uint8(ptr);
        }

        return 0; //invalid ID
    }

    uint16_t DesktopPacket::GetPacketIndex() const
    {
        uint16_t pkt_upd_index;

        if(GetUpdateProperties(NULL, &pkt_upd_index, NULL))
            return pkt_upd_index;
        
        if(GetSessionProperties(NULL, NULL, NULL, NULL,
                                &pkt_upd_index, NULL))
            return pkt_upd_index;

        return INVALID_PACKET_INDEX;
    }

    bool DesktopPacket::GetSessionProperties(uint8_t* session_id, 
                                             uint16_t* width, 
                                             uint16_t* height, 
                                             uint8_t* bmp_mode,
                                             uint16_t* pkt_upd_index,
                                             uint16_t* pkt_upd_count) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_SESSIONID_NEW);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t) + sizeof(uint16_t) +
           sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t))
        {
            ptr = READFIELD_DATAPTR(ptr);

            if(session_id)
                *session_id = get_uint8(ptr);
            ptr += sizeof(uint8_t);

            if(width)
                *width = get_uint16(ptr);
            ptr += sizeof(uint16_t);

            if(height)
                *height = get_uint16(ptr);
            ptr += sizeof(uint16_t);

            if(bmp_mode)
                *bmp_mode = get_uint8(ptr);
            ptr += sizeof(uint8_t);

            if(pkt_upd_index)
                *pkt_upd_index = get_uint16(ptr);
            ptr += sizeof(uint16_t);

            if(pkt_upd_count)
                *pkt_upd_count = get_uint16(ptr);
            ptr += sizeof(uint16_t);

            return true;
        }
        return false;
    }

    bool DesktopPacket::GetUpdateProperties(uint8_t* session_id, 
                                            uint16_t* pkt_upd_index,
                                            uint16_t* pkt_upd_count) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_SESSIONID_UPD);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t) 
            + sizeof(uint16_t) + sizeof(uint16_t))
        {
            ptr = READFIELD_DATAPTR(ptr);

            if(session_id)
                *session_id = get_uint8(ptr);
            ptr += sizeof(uint8_t);

            if(pkt_upd_index)
                *pkt_upd_index = get_uint16(ptr);
            ptr += sizeof(uint16_t);

            if(pkt_upd_count)
                *pkt_upd_count = get_uint16(ptr);
            ptr += sizeof(uint16_t);

            return true;
        }
        return false;
    }

    bool DesktopPacket::GetBlocks(map_block_t& blocks) const
    {
        const uint8_t* blocknums_ptr = FindField(FIELDTYPE_BLOCKNUMS_AND_SIZES);
        if(!blocknums_ptr)
            return false;

        uint16_t blocknums_len = READFIELD_SIZE(blocknums_ptr);
        blocknums_ptr = READFIELD_DATAPTR(blocknums_ptr);

        const uint8_t* blockdata_ptr = FindField(FIELDTYPE_BLOCKS_DATA);
        if(!blockdata_ptr)
            return false;

        uint16_t blockdata_len = READFIELD_SIZE(blockdata_ptr);
        blockdata_ptr = READFIELD_DATAPTR(blockdata_ptr);

        
        vector<uint16_t> blocks_n_sizes;
        const uint8_t* blocks_array = reinterpret_cast<const uint8_t*>(blocknums_ptr);

        ConvertFromUInt12Array(blocks_array, blocknums_len, blocks_n_sizes);

        assert(blocks_n_sizes.size() % 2 == 0);
        if(blocks_n_sizes.size() % 2)
            return false;

        uint16_t byte_pos = 0;
        for(uint16_t i=0;i<blocks_n_sizes.size();i+=2)
        {
            desktop_block bb;
            bb.block_data = reinterpret_cast<const char*>(&blockdata_ptr[byte_pos]);
            bb.block_size = blocks_n_sizes[i+1];

            byte_pos += bb.block_size;

            blocks[blocks_n_sizes[i]] = bb;
        }
        assert(blockdata_ptr + byte_pos == blockdata_ptr + blockdata_len);
        return true;
    }

    bool DesktopPacket::GetBlockFragments(block_frags_t& fragments) const
    {
        const uint8_t* info_ptr = FindField(FIELDTYPE_BLOCKNUMS_FRAGNO_AND_SIZES);
        if(!info_ptr)
            return false;

        const uint8_t* data_ptr = FindField(FIELDTYPE_BLOCKS_FRAG_DATA);
        if(!data_ptr)
            return false;

        uint16_t info_size = READFIELD_SIZE(info_ptr);
        info_ptr = READFIELD_DATAPTR(info_ptr);

        const uint8_t* u8_info_ptr = reinterpret_cast<const uint8_t*>(info_ptr);

        uint16_t data_size = READFIELD_SIZE(data_ptr);
        data_ptr = READFIELD_DATAPTR(data_ptr);

        uint16_t byte_pos = 0;
        for(uint16_t i=0;i<info_size;i+=4)
        {
            block_fragment bf;
            get2_uint12_ptr(u8_info_ptr, bf.block_no, bf.frag_size, u8_info_ptr);
            get_uint4_ptr(u8_info_ptr, bf.frag_no, bf.frag_cnt, u8_info_ptr);
            assert(byte_pos+bf.frag_size<=data_size);
            if(byte_pos+bf.frag_size>data_size) //buffer overflow check
                return false;
            bf.frag_data = reinterpret_cast<const char*>(&data_ptr[byte_pos]);
            byte_pos += bf.frag_size;
            fragments.push_back(bf);
        }
        assert(data_ptr + byte_pos == data_ptr + data_size);
        
        return true;
    }

    bool DesktopPacket::GetDuplicateBlocks(map_dup_blocks_t& dup_blocks) const
    {
        bool ok = false;
        const uint8_t* info_ptr = FindField(FIELDTYPE_BLOCK_DUP);
        if(info_ptr)
        {
            uint16_t info_size = READFIELD_SIZE(info_ptr);
            info_ptr = READFIELD_DATAPTR(info_ptr);

            const uint8_t* u8_info_ptr = reinterpret_cast<const uint8_t*>(info_ptr);
            vector<uint16_t> blocknums_single;
            ConvertFromUInt12Array(u8_info_ptr, info_size, blocknums_single);

            uint16_t block_no = 0xFFF;
            set<uint16_t> blocknums;
            for(size_t i=0;i<blocknums_single.size();i++)
            {
                if(block_no == 0xFFF)
                    block_no = blocknums_single[i];
                else if(blocknums_single[i] == 0xFFF)
                {
                    assert(block_no != 0xFFF);
                    assert(blocknums.size());
                    map_dup_blocks_t::iterator ii = dup_blocks.find(block_no);
                    if(ii != dup_blocks.end())
                        ii->second.insert(blocknums.begin(), blocknums.end());
                    else
                        dup_blocks[block_no] = blocknums;
                    block_no = 0xFFF;
                    blocknums.clear();
                }
                else
                    blocknums.insert(blocknums_single[i]);
            }
            ok = true;
        }
        info_ptr = FindField(FIELDTYPE_BLOCK_DUP_RANGE);
        if(info_ptr)
        {
            uint16_t info_size = READFIELD_SIZE(info_ptr);
            info_ptr = READFIELD_DATAPTR(info_ptr);

            const uint8_t* u8_info_ptr = reinterpret_cast<const uint8_t*>(info_ptr);
            vector<uint16_t> blocknums_range;
            ConvertFromUInt12Array(u8_info_ptr, info_size, blocknums_range);

            for(size_t i=0;i<blocknums_range.size();i+=3)
            {
                set<uint16_t> blocknums;
                for(uint16_t block_no=blocknums_range[i+1];
                    block_no<=blocknums_range[i+2];block_no++)
                    blocknums.insert(block_no);

                map_dup_blocks_t::iterator ii = dup_blocks.find(blocknums_range[i]);
                if(ii != dup_blocks.end())
                    ii->second.insert(blocknums.begin(), blocknums.end());
                else
                    dup_blocks[blocknums_range[i]] = blocknums;
            }
            ok = true;
        }
        return ok;
    }

    const char* DesktopPacket::GetBlock(uint16_t block_no, 
                                        uint16_t& length) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_BLOCKS_DATA);
        if(!ptr)
            return NULL;

        uint16_t total_size = READFIELD_SIZE(ptr);
        ptr = READFIELD_DATAPTR(ptr);

        map_block_t blocks;
        GetBlocks(blocks);

        map_block_t::const_iterator ii = blocks.find(block_no);
        if(ii != blocks.end())
        {
            length = ii->second.block_size;
            return ii->second.block_data;
        }
        return NULL;
    }


    DesktopAckPacket::DesktopAckPacket(uint16_t src_userid, uint32_t time, 
                                       uint16_t owner_userid, uint8_t session_id, 
                                       uint32_t time_ack,
                                       const std::set<uint16_t>& packets_ack,
                                       const packet_range_t& packet_range_ack)
                                       : FieldPacket(PACKETHDR_CHANNEL_ONLY,
                                                     PACKET_KIND_DESKTOP_ACK, 
                                                     src_userid, time)
    {
        int alloc_size = 0;

        //FIELDTYPE_SESSIONID_ACK
        //[sessionid(uint8_t), userid(uint16_t), time_ack(uint32_t)]
        int info_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t);
        alloc_size += FIELDVALUE_PREFIX + info_size;

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);
        
        uint8_t* data_ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        WRITEFIELD_TYPE(data_ptr, FIELDTYPE_SESSIONID_ACK, info_size, data_ptr);
        set_uint8(data_ptr, session_id); data_ptr += 1;
        set_uint16(data_ptr, owner_userid); data_ptr += 2;
        set_uint32(data_ptr, time_ack); data_ptr += 4;

        m_iovec.push_back(v);
#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
        InitCommon(packets_ack, packet_range_ack);
    }

    DesktopAckPacket::DesktopAckPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size) {}

    DesktopAckPacket::DesktopAckPacket(const DesktopAckPacket& packet)
        : FieldPacket(packet) {}

    bool DesktopAckPacket::GetSessionInfo(uint16_t* owner_userid,
                                          uint8_t* session_id, 
                                          uint32_t* upd_time) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_SESSIONID_ACK);
        if(!ptr)
            return false;

        uint16_t field_size = READFIELD_SIZE(ptr);
        if(field_size < sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint32_t))
            return false;

        const uint8_t* field_ptr = reinterpret_cast<const uint8_t*>(READFIELD_DATAPTR(ptr));
        if (session_id)
            *session_id = get_uint8(field_ptr);
        field_ptr += 1;
        if (owner_userid)
            *owner_userid = get_uint16(field_ptr);
        field_ptr += 2;
        if (upd_time)
            *upd_time = get_uint32(field_ptr);
        field_ptr += 4;
        return true;
    }

    void DesktopAckPacket::InitCommon(const std::set<uint16_t>& packets_ack,
                                      const packet_range_t& packet_range_ack)
    {
        //TODO: Ensure ACK packet doesn't become too big
        if(packets_ack.size())
        {
            std::vector<uint16_t> packetnums_input(packets_ack.begin(), 
                                                   packets_ack.end());

            //FIELDTYPE_PACKETS_ACK
            WriteUInt16ArrayToIOVec(packetnums_input, FIELDTYPE_PACKETS_ACK,
                                    m_iovec);

#ifdef ENABLE_ENCRYPTION
            m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
        }

        if(packet_range_ack.size())
        {
            std::vector<uint16_t> packetnums_input;

            packet_range_t::const_iterator ii = packet_range_ack.begin();
            while(ii != packet_range_ack.end())
            {
                assert(ii->first < ii->second);
                packetnums_input.push_back(ii->first);
                packetnums_input.push_back(ii->second);
                ii++;
            }

            //FIELDTYPE_PACKETRANGE_ACK
            WriteUInt16ArrayToIOVec(packetnums_input, FIELDTYPE_PACKETRANGE_ACK,
                                    m_iovec);
#ifdef ENABLE_ENCRYPTION
            m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
        }
    }

    bool DesktopAckPacket::GetPacketsAcked(std::set<uint16_t>& packets_ack) const
    {
        std::vector<uint16_t> packetnums;
        const uint8_t* ptr = FindField(FIELDTYPE_PACKETS_ACK);
        if(ptr)
        {

            //FIELDTYPE_PACKETS_ACK
            ReadUInt16Array(ptr, FIELDTYPE_PACKETS_ACK, packetnums);
            packets_ack.insert(packetnums.begin(), packetnums.end());
        }

        //FIELDTYPE_PACKETRANGE_ACK
        ptr = FindField(FIELDTYPE_PACKETRANGE_ACK);
        if(!ptr)
            return true;
        packetnums.clear();

        ReadUInt16Array(ptr, FIELDTYPE_PACKETRANGE_ACK, packetnums);
        assert(packetnums.size() % 2 == 0);
        //insert ranges
        for(size_t i=0;i<packetnums.size();i+=2)
        {
            assert(packetnums[i+1] > packetnums[i]);
            for(uint16_t b_low=packetnums[i];b_low<=packetnums[i+1];b_low++)
                packets_ack.insert(b_low);
        }
        return true;
    }

    DesktopNakPacket::DesktopNakPacket(uint16_t src_userid, uint32_t time, 
                                       uint8_t session_id)
                                       : FieldPacket(PACKETHDR_CHANNEL_ONLY,
                                                     PACKET_KIND_DESKTOP_NAK, 
                                                     src_userid, time)
    {
        int alloc_size = FIELDVALUE_PREFIX + sizeof(uint8_t); //FIELDTYPE_SESSIONID_NAK

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        //store data indexes
        uint8_t* ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        WRITEFIELD_VALUE_U8(ptr, FIELDTYPE_SESSIONID_NAK, session_id, ptr);

        v.iov_len = (u_long)(ptr - reinterpret_cast<const uint8_t*>(v.iov_base));
        assert(v.iov_len == alloc_size);

        assert(m_iovec.size());

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
    }

    DesktopNakPacket::DesktopNakPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size)
    {
    }

    DesktopNakPacket::DesktopNakPacket(const DesktopNakPacket& packet)
        : FieldPacket(packet)
    {
    }

    uint8_t DesktopNakPacket::GetSessionID() const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_SESSIONID_NAK);
        if(ptr && READFIELD_SIZE(ptr) >= sizeof(uint8_t))
        {
            ptr = READFIELD_DATAPTR(ptr);
            return get_uint8(ptr);
        }

        return 0; //invalid ID
    }

    DesktopCursorPacket::DesktopCursorPacket(uint16_t src_userid, uint32_t time, 
                                             uint8_t session_id, int16_t x, int16_t y)
                                : FieldPacket(PACKETHDR_CHANNEL_ONLY,
                                              PACKET_KIND_DESKTOPCURSOR, 
                                              src_userid, time)
    {
        int alloc_size = 0;

        //FIELDTYPE_MY_CURSORPOS
        //[sessionid(uint8_t), x(uint16_t), y(uint16_t)]

        int field_size = sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t);

        alloc_size += FIELDVALUE_PREFIX + field_size;

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        vector<uint8_t> cursor_session(field_size);
        uint8_t* field_ptr = &cursor_session[0];
        set_uint8_ptr(field_ptr, session_id, field_ptr);
        set_uint16_ptr(field_ptr, x, field_ptr);
        set_uint16_ptr(field_ptr, y, field_ptr);

        WRITEFIELD_DATA(data_buf, FIELDTYPE_MY_CURSORPOS, &cursor_session[0],
                        cursor_session.size(), data_buf); 

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
    }

    DesktopCursorPacket::DesktopCursorPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size)
    {
    }

    DesktopCursorPacket::DesktopCursorPacket(const DesktopCursorPacket& packet)
        : DesktopCursorPacket(packet.GetSrcUserID(), packet.GetTime(), packet.GetStreamID(), packet.GetX(), packet.GetY())
    {
        SetChannel(packet.GetChannel());
    }

    bool DesktopCursorPacket::GetSessionCursor(uint16_t* dest_userid, 
                                               uint8_t* session_id,
                                               int16_t* x,
                                               int16_t* y) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_MY_CURSORPOS);
        if(ptr)
        {
            uint16_t field_size = READFIELD_SIZE(ptr);
            if(field_size < sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t))
                return false;

            if (dest_userid)
                *dest_userid = 0;

            const uint8_t* field_ptr = reinterpret_cast<const uint8_t*>(READFIELD_DATAPTR(ptr));
            if (session_id)
                *session_id = get_uint8(field_ptr);
            field_ptr += 1;
            if (x)
                *x = get_uint16(field_ptr);
            field_ptr += 2;
            if (y)
                *y = get_uint16(field_ptr);
            field_ptr += 2;
            return true;
        }
        ptr = FindField(FIELDTYPE_REMOTE_CURSORPOS);
        if(ptr)
        {
            uint16_t field_size = READFIELD_SIZE(ptr);
            if(field_size < sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint16_t) + sizeof(uint16_t))
                return false;

            const uint8_t* field_ptr = reinterpret_cast<const uint8_t*>(READFIELD_DATAPTR(ptr));
            if (dest_userid)
                *dest_userid = get_uint16(field_ptr);
            field_ptr += 2;
            if (session_id)
                *session_id = get_uint8(field_ptr);
            field_ptr += 1;
            if (x)
                *x = get_uint16(field_ptr);
            field_ptr += 2;
            if (y)
                *y = get_uint16(field_ptr);
            field_ptr += 2;
            return true;
        }
        return false;
    }

    uint16_t DesktopCursorPacket::GetDestUserID() const
    {
        uint16_t dest_userid;
        if(GetSessionCursor(&dest_userid, 0, 0, 0))
            return dest_userid;
        return INVALID_DEST_USERID;
    }

    DesktopInputPacket::DesktopInputPacket(uint16_t src_userid, uint32_t time,
                                           uint8_t session_id, uint8_t packetno,
                                           const std::vector<DesktopInput>& inputs)
                                           : FieldPacket(PACKETHDR_DEST_USER,
                                                         PACKET_KIND_DESKTOPINPUT,
                                                         src_userid, time)
    {
        //FIELDTYPE_REMOTE_INPUT
        //[sessionid(uint8_t), packetno(uint8_t), [[x(uint16_t), y(uint16_t), keycode(uint32_t), keystate(uint32_t)], ...

        int field_size = sizeof(uint8_t) + sizeof(uint8_t) + 
                            (sizeof(uint16_t) + sizeof(uint16_t) +
                            sizeof(uint32_t) + sizeof(uint32_t)) * int(inputs.size());

        int alloc_size = FIELDVALUE_PREFIX + field_size;

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);

        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        vector<uint8_t> buffer(field_size);
        buffer.resize(field_size);
        uint8_t* field_ptr = &buffer[0];
        set_uint8_ptr(field_ptr, session_id, field_ptr);
        set_uint8_ptr(field_ptr, packetno, field_ptr);

        for(size_t i=0;i<inputs.size();i++)
        {
            set_uint16_ptr(field_ptr, inputs[i].x, field_ptr);
            set_uint16_ptr(field_ptr, inputs[i].y, field_ptr);
            set_uint32_ptr(field_ptr, inputs[i].keycode, field_ptr);
            set_uint32_ptr(field_ptr, inputs[i].keystate, field_ptr);
        }

        WRITEFIELD_DATA(data_buf, FIELDTYPE_REMOTE_INPUT, &buffer[0],
                        buffer.size(), data_buf);

        m_iovec.push_back(v);

#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
    }

    DesktopInputPacket::DesktopInputPacket(const char* packet, uint16_t packet_size)
        : FieldPacket(packet, packet_size)
    {
    }

    DesktopInputPacket::DesktopInputPacket(const DesktopInputPacket& packet)
        : DesktopInputPacket(packet.GetSrcUserID(), packet.GetTime(), packet.GetSessionID(),
                             packet.GetPacketNo(), packet.GetDesktopInput())
    {
        SetChannel(packet.GetChannel());
        SetDestUser(packet.GetDestUserID());
    }

    bool DesktopInputPacket::GetSessionInfo(uint8_t* session_id,
                                            uint8_t* packetno) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_REMOTE_INPUT);
        if(ptr)
        {
            uint16_t field_size = READFIELD_SIZE(ptr);
            if(field_size < sizeof(uint8_t) + sizeof(uint8_t))
                return false;

            const uint8_t* field_ptr = READFIELD_DATAPTR(ptr);

            if (session_id)
                *session_id = get_uint8(field_ptr);
            field_ptr += 1;
            if (packetno)
                *packetno = get_uint8(field_ptr);
            field_ptr += 1;
            return true;
        }
        return false;
    }

    uint8_t DesktopInputPacket::GetSessionID() const
    {
        uint8_t session_id = 0;
        GetSessionInfo(&session_id, 0);
        return session_id;
    }

    uint8_t DesktopInputPacket::GetPacketNo(bool* found/* = NULL*/) const
    {
        uint8_t packetno = 0;
        found? *found = GetSessionInfo(0, &packetno) 
             : GetSessionInfo(0, &packetno);
        return packetno;
    }

    bool DesktopInputPacket::GetDesktopInput(std::vector<DesktopInput>& inputs) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_REMOTE_INPUT);
        if(ptr)
        {
            uint16_t field_size = READFIELD_SIZE(ptr);
            if(field_size < sizeof(uint8_t) + sizeof(uint8_t) + 
                            sizeof(uint16_t) + sizeof(uint16_t) + 
                            sizeof(uint32_t) + sizeof(uint32_t))
                return false;

            const uint8_t* field_ptr = reinterpret_cast<const uint8_t*>(READFIELD_DATAPTR(ptr));

            uint8_t session_id = 0;
            get_uint8_ptr(session_id, field_ptr, field_ptr);
            field_size -= 1;
            uint8_t packetno = 0;
            get_uint8_ptr(packetno, field_ptr, field_ptr);
            field_size -= 1;

            while(field_size)
            {
                assert(field_size % 12 == 0);
                if(field_size % 12)
                    return false;

                DesktopInput input;
                get_uint16_ptr(input.x, field_ptr, field_ptr);
                field_size -= 2;
                get_uint16_ptr(input.y, field_ptr, field_ptr);
                field_size -= 2;
                get_uint32_ptr(input.keycode, field_ptr, field_ptr);
                field_size -= 4;
                get_uint32_ptr(input.keystate, field_ptr, field_ptr);
                field_size -= 4;
                
                inputs.push_back(input);
            }
            return true;
        }
        return false;
    }

    std::vector<DesktopInput> DesktopInputPacket::GetDesktopInput() const
    {
        std::vector<DesktopInput> result;
        GetDesktopInput(result);
        return result;
    }


    DesktopInputAckPacket::DesktopInputAckPacket(uint16_t src_userid, uint32_t time, 
                                                 uint8_t session_id, uint8_t packetno)
                                                 : FieldPacket(PACKETHDR_DEST_USER,
                                                               PACKET_KIND_DESKTOPINPUT_ACK,
                                                               src_userid, time)
    {
        int alloc_size = 0;

        //FIELDTYPE_DESKTOPINPUT_ACK
        //[sessionid(uint8_t), packetno(uint8_t)]
        int info_size = sizeof(uint8_t) + sizeof(uint8_t);
        alloc_size += FIELDVALUE_PREFIX + info_size;

        uint8_t* data_buf;
        ACE_NEW(data_buf, uint8_t[alloc_size]);
        
        uint8_t* data_ptr = data_buf;
        iovec v;
        v.iov_base = reinterpret_cast<char*>(data_buf);
        v.iov_len = alloc_size;

        WRITEFIELD_TYPE(data_ptr, FIELDTYPE_DESKTOPINPUT_ACK, info_size, data_ptr);
        set_uint8_ptr(data_ptr, session_id, data_ptr);
        set_uint8_ptr(data_ptr, packetno, data_ptr);

        m_iovec.push_back(v);
#ifdef ENABLE_ENCRYPTION
        m_crypt_sections.insert(uint8_t(m_iovec.size())-1);
#endif
    }

    DesktopInputAckPacket::DesktopInputAckPacket(const DesktopInputAckPacket& packet)
        : DesktopInputAckPacket(packet.GetSrcUserID(), packet.GetTime(),
                                packet.GetSessionID(), packet.GetPacketNo())
    {
        SetChannel(packet.GetChannel());
        SetDestUser(packet.GetDestUserID());
    }
    
    bool DesktopInputAckPacket::GetSessionInfo(uint8_t* session_id,
                                               uint8_t* packetno) const
    {
        const uint8_t* ptr = FindField(FIELDTYPE_DESKTOPINPUT_ACK);
        if(!ptr)
            return false;

        uint16_t field_size = READFIELD_SIZE(ptr);
        if(field_size < sizeof(uint8_t) + sizeof(uint8_t))
            return false;

        const uint8_t* field_ptr = READFIELD_DATAPTR(ptr);
        if (session_id)
            *session_id = get_uint8(field_ptr);
        field_ptr += 1;
        if (packetno)
            *packetno = get_uint8(field_ptr);
        field_ptr += 1;
        return true;
    }

    uint8_t DesktopInputAckPacket::GetSessionID() const
    {
        uint8_t session_id = 0;
        GetSessionInfo(&session_id, 0);
        return session_id;
    }

    uint8_t DesktopInputAckPacket::GetPacketNo(bool* found/* = NULL*/) const
    {
        uint8_t packetno = 0;
        found? *found = GetSessionInfo(0, &packetno) :
               GetSessionInfo(0, &packetno);
        return packetno;
    }

} /* namespace */
