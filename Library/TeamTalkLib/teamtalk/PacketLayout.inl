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

template < typename PACKETTYPE, uint8_t PACKET_KIND_CRYPT, uint8_t PACKET_KIND_DECRYPTED >
CryptPacket< PACKETTYPE, PACKET_KIND_CRYPT, PACKET_KIND_DECRYPTED >::CryptPacket(const char* packet, uint16_t packet_size)
: FieldPacket(packet, packet_size)
{
    assert((packet[PACKET_INDEX_KIND] & PACKET_MASK_KIND) == PACKET_KIND_CRYPT);
}

struct cipher_guard
{
    EVP_CIPHER_CTX* ctx;
    cipher_guard(EVP_CIPHER_CTX* c) : ctx(c) {}
    ~cipher_guard() { EVP_CIPHER_CTX_free(ctx); }
};

template < typename PACKETTYPE, uint8_t PACKET_KIND_CRYPT, uint8_t PACKET_KIND_DECRYPTED >
CryptPacket< PACKETTYPE, PACKET_KIND_CRYPT, PACKET_KIND_DECRYPTED >::CryptPacket(const PACKETTYPE& p, 
                                                          const uint8_t* cryptkey)
                         : FieldPacket(p.GetHdrType(), PACKET_KIND_CRYPT, p.GetSrcUserID(), p.GetTime())
{
    int buffers = 0;
    const iovec* v_data = p.GetPacket(buffers);
    assert(buffers >= 2);

    const std::set<uint8_t>& crypt_sections = p.GetCryptSections();
    std::set<uint8_t>::const_iterator c_ii = crypt_sections.begin();
    assert(crypt_sections.size()); //nothing to encrypt ?!?

    int data_len = 0;
    while(c_ii != crypt_sections.end())
    {
        assert(*c_ii < buffers);
        data_len += v_data[*c_ii].iov_len;
        c_ii++;
    }

    const EVP_CIPHER* cf = EVP_aes_256_cbc();
    int alloc_size = FIELDVALUE_PREFIX + data_len + 2 /*crc16*/ + EVP_CIPHER_block_size(cf);
    char* field_buf;
    ACE_NEW(field_buf, char[alloc_size]);
    char* encrypt_buf = &field_buf[FIELDVALUE_PREFIX]; //make room for field-prefix

    assert(alloc_size - FIELDVALUE_PREFIX >= data_len + 2 /*crc16*/ + EVP_CIPHER_block_size(cf));

    int status = 0;
    int encrypt_len = 0, tmpLen = 0;
    EVP_CIPHER_CTX* aesEncCtx = EVP_CIPHER_CTX_new();
    cipher_guard g(aesEncCtx);
    EVP_CIPHER_CTX_init(aesEncCtx);
    status = EVP_EncryptInit(aesEncCtx, cf, cryptkey, NULL);
    assert(status == 1);
    //status = EVP_CIPHER_CTX_set_padding(aesEncCtx, 0);
    //assert(status == 1);

    assert(EVP_CIPHER_CTX_block_size(aesEncCtx) == EVP_CIPHER_block_size(cf));

    //crc32 of all the buffers to be encrypted
    uint32_t crc32 = 0;

    //encrypt the iovec's sections
    c_ii = crypt_sections.begin();
    while(c_ii != crypt_sections.end())
    {
        crc32 = ACE::crc32(v_data[*c_ii].iov_base, v_data[*c_ii].iov_len, crc32);
        tmpLen = 0;
        status = EVP_EncryptUpdate(aesEncCtx, 
                                   reinterpret_cast<uint8_t*>(&encrypt_buf[encrypt_len]), 
                                   &tmpLen, 
                                   reinterpret_cast<const uint8_t*>(v_data[*c_ii].iov_base), 
                                   v_data[*c_ii].iov_len);
        assert(status == 1);
        encrypt_len += tmpLen;
        assert(encrypt_len <= alloc_size - FIELDVALUE_PREFIX);
        c_ii++;
    }

    //insert crc which can be used to check for proper decryption
    uint16_t crc16 = crc32 & 0xFFFF;
    tmpLen = 0;
    status = EVP_EncryptUpdate(aesEncCtx, 
                               reinterpret_cast<uint8_t*>(&encrypt_buf[encrypt_len]), 
                               &tmpLen, 
                               reinterpret_cast<const uint8_t*>(&crc16), 
                               2);
    assert(status == 1);
    encrypt_len += tmpLen;
    assert(encrypt_len <= alloc_size - FIELDVALUE_PREFIX);
    tmpLen = 0;
    status = EVP_EncryptFinal(aesEncCtx, reinterpret_cast<uint8_t*>(&encrypt_buf[encrypt_len]), 
                              &tmpLen);
    assert(status == 1);
    encrypt_len += tmpLen;
    assert(encrypt_len <= alloc_size - FIELDVALUE_PREFIX);
    status = EVP_CIPHER_CTX_cleanup(aesEncCtx);
    assert(status == 1);

    //MYTRACE(ACE_TEXT("Encrypted %d bytes with key crc 0x%08x, crypt data crc32 0x%08x\n"),
    //        encrypt_len, ACE::crc32(cryptkey, CRYPTKEY_SIZE), 
    //        ACE::crc32(encrypt_buf, encrypt_len));

    char* ptr = field_buf;
    WRITEFIELD_TYPE(field_buf, FIELDTYPE_CRYPTDATA, encrypt_len, ptr);

    iovec v;
    v.iov_base = field_buf;
    v.iov_len = FIELDVALUE_PREFIX + encrypt_len;

    m_iovec.push_back(v);

    //copy FieldPacket settings
    if(p.GetDestUserID())
        this->SetDestUser(p.GetDestUserID());
    if(p.GetChannel())
        this->SetChannel(p.GetChannel());
}

template < typename PACKETTYPE, uint8_t PACKET_KIND_CRYPT, uint8_t PACKET_KIND_DECRYPTED >
PACKETTYPE* CryptPacket< PACKETTYPE, PACKET_KIND_CRYPT, PACKET_KIND_DECRYPTED >::Decrypt(const uint8_t* decryptkey) const
{
    const uint8_t* encrypt_ptr = FindField(FIELDTYPE_CRYPTDATA);
    if(!encrypt_ptr)
        return NULL;

    uint16_t encrypt_len = READFIELD_SIZE(encrypt_ptr);
    encrypt_ptr = READFIELD_DATAPTR(encrypt_ptr);

    //MYTRACE(ACE_TEXT("Decrypt %d bytes with key crc 0x%08x, crypt data crc32 0x%08x\n"),
    //        (int)encrypt_len,
    //        ACE::crc32(decryptkey, CRYPTKEY_SIZE),
    //        ACE::crc32(encrypt_ptr, encrypt_len));

    const EVP_CIPHER* cf = EVP_aes_256_cbc();
    char* decrypt_buf;
    int alloc_size = encrypt_len + EVP_CIPHER_block_size(cf);
    ACE_NEW_RETURN(decrypt_buf, char[alloc_size], NULL);

    int status = 0;
    int decrypt_len = 0, tmpLen = 0;
    EVP_CIPHER_CTX* aesDecCtx = EVP_CIPHER_CTX_new();
    cipher_guard g(aesDecCtx);
    EVP_CIPHER_CTX_init(aesDecCtx);
    status = EVP_DecryptInit(aesDecCtx, cf, decryptkey, NULL);
    assert(status == 1);
    //status = EVP_CIPHER_CTX_set_padding(aesDecCtx, 0);
    //assert(status == 1);
    status = EVP_DecryptUpdate(aesDecCtx, reinterpret_cast<uint8_t*>(decrypt_buf), 
                               &tmpLen, reinterpret_cast<const uint8_t*>(encrypt_ptr), 
                               encrypt_len);
    decrypt_len += tmpLen;
    assert(decrypt_len <= alloc_size);
    tmpLen = 0;
    status = EVP_DecryptFinal(aesDecCtx, 
                              reinterpret_cast<uint8_t*>(&decrypt_buf[decrypt_len]), 
                              &tmpLen);
    decrypt_len += tmpLen;
    assert(decrypt_len <= alloc_size);
    EVP_CIPHER_CTX_cleanup(aesDecCtx);

    //crc16 is last 2 bytes of decrypted data chunk
    const char* ptr = decrypt_buf;
    ptr += decrypt_len - 2;
    uint32_t crc32 = ACE::crc32(decrypt_buf, decrypt_len - 2);
    uint16_t crc16 = crc32 & 0xFFFF;
    if(get_uint16(ptr) != crc16)
    {
        MYTRACE(ACE_TEXT("Invalid CRC for packet %d from #%d\n"), PACKET_KIND_CRYPT, GetSrcUserID()); 
        delete [] decrypt_buf;
        return NULL;
    }
    iovec v;
    v.iov_base = decrypt_buf;
    v.iov_len = decrypt_len - 2;

    PACKETTYPE* p;
    ACE_NEW_NORETURN(p, PACKETTYPE(PACKET_KIND_DECRYPTED, *this, v));
    if(!p)
    {
        delete [] decrypt_buf;
        return NULL;
    }
    return p;
}

