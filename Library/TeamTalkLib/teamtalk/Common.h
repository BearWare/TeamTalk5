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

#ifndef COMMON_H
#define COMMON_H

#include <myace/MyACE.h>
#include <ace/INET_Addr.h>
#include <ace/Date_Time.h>
#include <ace/Time_Value.h>

#include <regex>

#include <TeamTalkDefs.h>
#include "PacketLayout.h"
#include <codec/MediaUtil.h>

namespace teamtalk {

    struct ServerProperties
    {
        ACE_TString systemid;
        ACE_TString version;
        bool autosave;
        ACE_TString motd;
        ACE_TString servername;
        int maxusers;
        int maxloginattempts; //max login attempts with wrong password
        int max_logins_per_ipaddr;
        int logindelay = 0; // msec before IP-address can log in again
        ACE_INT64 diskquota; //max bytes for each channel to store files
        ACE_INT64 maxdiskusage; //max bytes to use for storage of files
        int usertimeout;
        int voicetxlimit;
        int videotxlimit;
        int mediafiletxlimit;
        int desktoptxlimit;
        int totaltxlimit;
        ServerProperties();
    };

    struct ServerStats
    {
        ACE_INT64 total_bytessent;
        ACE_INT64 total_bytesreceived;
        ACE_INT64 last_bytessent;
        ACE_INT64 last_bytesreceived;
        ACE_INT64 avg_bytessent;
        ACE_INT64 avg_bytesreceived;
        ACE_INT64 voice_bytesreceived;
        ACE_INT64 voice_bytessent;
        ACE_INT64 last_voice_bytessent;
        ACE_INT64 vidcap_bytesreceived;
        ACE_INT64 vidcap_bytessent;
        ACE_INT64 last_vidcap_bytessent;
        ACE_INT64 mediafile_bytesreceived;
        ACE_INT64 mediafile_bytessent;
        ACE_INT64 last_mediafile_bytessent;
        ACE_INT64 desktop_bytesreceived;
        ACE_INT64 desktop_bytessent;
        ACE_INT64 last_desktop_bytessent;
        ACE_INT64 files_bytesreceived;
        ACE_INT64 files_bytessent;

        int userspeak;
        int usersservered;
        //uptime
        ACE_Time_Value starttime;
        
        ACE_INT64 packets_received = 0, packets_sent = 0; // only used internally

        ServerStats()
            : total_bytessent(0), total_bytesreceived(0), last_bytessent(0)
            , last_bytesreceived(0), avg_bytessent(0), avg_bytesreceived(0)
            , voice_bytesreceived(0), voice_bytessent(0)
            , vidcap_bytesreceived(0), vidcap_bytessent(0)
            , last_voice_bytessent(0), last_vidcap_bytessent(0)
            , mediafile_bytesreceived(0), desktop_bytesreceived(0)
            , desktop_bytessent(0), last_desktop_bytessent(0)
            , mediafile_bytessent(0), last_mediafile_bytessent(0), userspeak(0)
            , usersservered(0), files_bytesreceived(0), files_bytessent(0)
        {}
    };

    /* Remember to updated DLL header file when modifying this */
    enum MsgType
    {
        TTNoneMsg       = 0,
        TTUserMsg       = 1,
        TTChannelMsg    = 2,
        TTBroadcastMsg  = 3,
        TTCustomMsg     = 4
    };

    //text messages for server and client
    struct TextMessage
    {
        MsgType msgType;
        int from_userid;
        ACE_TString from_username;
        int to_userid;
        ACE_TString content;
        int channelid;
        TextMessage() : msgType(TTNoneMsg)
        {
            channelid = from_userid = to_userid = 0;
        }
    };

    /* Remember to updated DLL header file when modifying this */
    enum UserType
    {
        USERTYPE_NONE         = 0x00,
        USERTYPE_DEFAULT      = 0x01,
        USERTYPE_ADMIN        = 0x02,
    };

    typedef ACE_UINT32 UserTypes;

    /* Remember to updated DLL header file when modifying this */
    enum UserRight
    {
        USERRIGHT_NONE                                  = 0x00000000,
        USERRIGHT_MULTI_LOGIN                           = 0x00000001,
        USERRIGHT_VIEW_ALL_USERS                        = 0x00000002,
        USERRIGHT_CREATE_TEMPORARY_CHANNEL              = 0x00000004,
        USERRIGHT_MODIFY_CHANNELS                       = 0x00000008,
        USERRIGHT_TEXTMESSAGE_BROADCAST                 = 0x00000010,
        USERRIGHT_KICK_USERS                            = 0x00000020,
        USERRIGHT_BAN_USERS                             = 0x00000040,
        USERRIGHT_MOVE_USERS                            = 0x00000080,
        USERRIGHT_OPERATOR_ENABLE                       = 0x00000100,
        USERRIGHT_UPLOAD_FILES                          = 0x00000200,
        USERRIGHT_DOWNLOAD_FILES                        = 0x00000400,
        USERRIGHT_UPDATE_SERVERPROPERTIES               = 0x00000800,
        USERRIGHT_TRANSMIT_VOICE                        = 0x00001000,
        USERRIGHT_TRANSMIT_VIDEOCAPTURE                 = 0x00002000,
        USERRIGHT_TRANSMIT_DESKTOP                      = 0x00004000,
        USERRIGHT_TRANSMIT_DESKTOPINPUT                 = 0x00008000,
        USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO              = 0x00010000,
        USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO              = 0x00020000,
        USERRIGHT_TRANSMIT_MEDIAFILE                    = USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |
                                                          USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO,
        USERRIGHT_LOCKED_NICKNAME                       = 0x00040000,
        USERRIGHT_LOCKED_STATUS                         = 0x00080000,
        USERRIGHT_RECORD_VOICE                          = 0x00100000,

        USERRIGHT_DEFAULT = USERRIGHT_MULTI_LOGIN |
                            USERRIGHT_VIEW_ALL_USERS |
                            USERRIGHT_CREATE_TEMPORARY_CHANNEL |
                            USERRIGHT_UPLOAD_FILES |
                            USERRIGHT_DOWNLOAD_FILES |
                            USERRIGHT_TRANSMIT_VOICE |
                            USERRIGHT_TRANSMIT_VIDEOCAPTURE |
                            USERRIGHT_TRANSMIT_DESKTOP |
                            USERRIGHT_TRANSMIT_DESKTOPINPUT |
                            USERRIGHT_TRANSMIT_MEDIAFILE,
                               

        USERRIGHT_ALL                       = 0x0013FFFF,
        USERRIGHT_KNOWN_MASK                = 0x001FFFFF
    };
    
    typedef ACE_UINT32 UserRights;

    struct Abuse
    {
        int n_cmds;
        int cmd_msec;
        Abuse() : n_cmds(0), cmd_msec(0) {}

        std::vector<int> toParam() const
        {
            std::vector<int> flood;
            flood.push_back(n_cmds);
            flood.push_back(cmd_msec);
            return flood;
        }
        void fromParam(const std::vector<int>& flood)
        {
            if(flood.size()>=2)
            {
                n_cmds = flood[0];
                cmd_msec = flood[1];
            }
        }
    };

    struct UserAccount
    {
        ACE_TString username;
        ACE_TString passwd;
        ACE_TString note;
        UserTypes usertype;
        UserRights userrights; /* Mask of UserRight */
        int userdata;
        ACE_TString init_channel;
        intset_t auto_op_channels;
        int audiobpslimit;
        Abuse abuse;
        ACE_TString nickname; /* TODO: add to TT API */
        
        UserAccount()
        {
            userrights = USERRIGHT_NONE;
            usertype = USERTYPE_NONE;
            userdata = 0;
            audiobpslimit = 0;
        }
    };
    typedef std::vector<UserAccount> useraccounts_t;

    enum BanType
    {
        BANTYPE_NONE            = 0x00,
        BANTYPE_CHANNEL         = 0x01,
        BANTYPE_IPADDR          = 0x02,
        BANTYPE_USERNAME        = 0x04,
        BANTYPE_DEFAULT = BANTYPE_IPADDR,
    };
    typedef ACE_UINT32 BanTypes;

    struct BannedUser
    {
        BanTypes bantype;
        ACE_TString ipaddr;
        ACE_TString chanpath;
        ACE_Time_Value bantime;
        ACE_TString nickname;
        ACE_TString username;
        BannedUser() : bantype(BANTYPE_NONE) { bantime = ACE_OS::gettimeofday(); }

        bool Same(const BannedUser& user) const
        {
            bool same = user.bantype == this->bantype;
            if (bantype & BANTYPE_IPADDR)
                same &= user.ipaddr == this->ipaddr;
            if (bantype & BANTYPE_CHANNEL)
                same &= user.chanpath == this->chanpath;
            if (bantype & BANTYPE_USERNAME)
                same &= user.username == this->username;
            return same;
        }

        bool Match(const BannedUser& user) const
        {
            bool match = true;
            if((bantype & BANTYPE_IPADDR) && ipaddr.length())
            {
                ACE_TString rgx = ACE_TEXT("^") + ipaddr + ACE_TEXT("$");
#if defined(UNICODE)
                match &= std::regex_search(user.ipaddr.c_str(), std::wregex(rgx.c_str()));
#else
                // equality not nescessary when debian7 is no longer supported
                match &= std::regex_search(user.ipaddr.c_str(), std::regex(rgx.c_str())) ||
                    user.ipaddr == ipaddr;
#endif
            }
            if((bantype & BANTYPE_USERNAME))
                match &= username == user.username;
            if((bantype & BANTYPE_CHANNEL))
                match &= chanpath == user.chanpath;
            match &= bantype != BANTYPE_NONE;
            return match;
        }
    };

    typedef std::vector<BannedUser> bannedusers_t;

    ACE_Date_Time StringToDate(const ACE_TString& str_date);
    ACE_TString DateToString(const ACE_Time_Value& tv);


    enum StreamType //ensure DLL compliance
    {
        STREAMTYPE_NONE                     = 0x00000000,
        STREAMTYPE_VOICE                    = 0x00000001,
        STREAMTYPE_VIDEOCAPTURE             = 0x00000002,
        STREAMTYPE_MEDIAFILE_AUDIO          = 0x00000004,
        STREAMTYPE_MEDIAFILE_VIDEO          = 0x00000008,
        STREAMTYPE_MEDIAFILE                = STREAMTYPE_MEDIAFILE_AUDIO |
                                              STREAMTYPE_MEDIAFILE_VIDEO,
        STREAMTYPE_DESKTOP                  = 0x00000010,
        STREAMTYPE_DESKTOPINPUT             = 0x00000020,
    };

    typedef ACE_UINT32 StreamTypes;


    enum FileTransferStatus
    {
        FILETRANSFER_CLOSED     = 0,
        FILETRANSFER_ERROR      = 1,
        FILETRANSFER_ACTIVE     = 2,
        FILETRANSFER_FINISHED   = 3,
    };

    struct FileTransfer
    {
        FileTransferStatus status;
        int channelid;
        ACE_TString localfile;
        ACE_TString filename;
        int userid;
        ACE_INT64 filesize;
        ACE_INT64 transferred;
        int transferid;
        bool inbound;
        FileTransfer()
        {
            status = FILETRANSFER_CLOSED;
            filesize = transferred = 0;
            userid = transferid = 0;
            inbound = true;
        }
    };

    /* Remember to updated DLL header file when modifying this */
    enum Codec
    {
        CODEC_NO_CODEC                  = 0,
        CODEC_SPEEX                     = 1,
        CODEC_SPEEX_VBR                 = 2,
        CODEC_OPUS                      = 3,
        CODEC_WEBM_VP8                  = 128,
    };

    struct SpeexCodec
    {
        int bandmode;
        int quality;
        int frames_per_packet;
        bool sim_stereo;
    };
    
    struct SpeexVBRCodec
    {
        int bandmode;
        int vbr_quality;
        int bitrate;
        int max_bitrate;
        bool dtx;
        int frames_per_packet;
        bool sim_stereo;
    };

    struct OpusCodec
    {
        int samplerate;
        int channels;
        int application;
        int complexity;
        bool fec;
        bool dtx;
        int bitrate;
        bool vbr;
        bool vbr_constraint;
        int frame_size;
        int frames_per_packet;
    };

    struct AudioCodec
    {
        Codec codec;
        union
        {
            SpeexCodec speex;
            SpeexVBRCodec speex_vbr;
            OpusCodec opus;
        };
        AudioCodec()
        {
            //ensure that codecs can be compared using memcmp
            memset(this, 0, sizeof(AudioCodec));
            codec = CODEC_NO_CODEC;
        }
        inline bool operator==(const AudioCodec& ch) const
        {
            switch (codec)
            {
            case CODEC_SPEEX :
                return memcmp(&speex, &ch.speex, sizeof(speex)) == 0;
            case CODEC_SPEEX_VBR :
                return memcmp(&speex_vbr, &ch.speex_vbr, sizeof(speex_vbr)) == 0;
            case CODEC_OPUS :
                return memcmp(&opus, &ch.opus, sizeof(opus)) == 0;
            case CODEC_NO_CODEC :
                return codec == ch.codec;
            case CODEC_WEBM_VP8 :
                return false;
            }
            return false;
        }
        inline bool operator!=(const AudioCodec& ch) const
        {
            return (ch == *this) == false;//memcmp(this, &ch, sizeof(ch));
        }
    };

    struct AudioConfig
    {
        bool enable_agc;
        int gain_level;
        AudioConfig()
            : enable_agc(false)
            , gain_level(0) { }
    };

    struct SpeexDSP
    {
        bool enable_agc;
        int agc_gainlevel;
        int agc_maxincdbsec;
        int agc_maxdecdbsec;
        int agc_maxgaindb;
        bool enable_denoise;
        int maxnoisesuppressdb;
        bool enable_aec;
        int aec_suppress_level;
        int aec_suppress_active;

        SpeexDSP()
        {
            enable_agc = false;
            agc_gainlevel = 0;
            agc_maxincdbsec = 0;
            agc_maxdecdbsec = 0;
            agc_maxgaindb = 0;
            enable_denoise = false;
            maxnoisesuppressdb = 0;
            enable_aec = false;
            aec_suppress_level = 0;
            aec_suppress_active = 0;
        }
    };

    struct TTAudioPreprocessor
    {
        int gainlevel = GAIN_NORMAL;
        bool muteleft = false;
        bool muteright = false;

        TTAudioPreprocessor() { }
    };

    enum AudioPreprocessorType
    {
        AUDIOPREPROCESSOR_NONE      = 0,
        AUDIOPREPROCESSOR_SPEEXDSP  = 1,
        AUDIOPREPROCESSOR_TEAMTALK  = 2
    };
    
    struct AudioPreprocessor
    {
        AudioPreprocessorType preprocessor = AUDIOPREPROCESSOR_NONE;
        union
        {
            SpeexDSP speexdsp;
            TTAudioPreprocessor ttpreprocessor;
        };
        AudioPreprocessor() {}
    };

    struct WebMVP8Codec
    {
        int rc_target_bitrate; /* 0 = 256 kbit/sec */
        unsigned long encode_deadline; /* 0 = VPX_DL_BEST_QUALITY */
    };

    struct VideoCodec
    {
        Codec codec;
        union
        {
            WebMVP8Codec webm_vp8;
        };
        VideoCodec() : webm_vp8()
        {
            codec = CODEC_NO_CODEC;
        }
    };

    struct RemoteFile
    {
        int channelid;
        int fileid;
        ACE_TString filename;
        ACE_TString internalname;
        ACE_INT64 filesize;
        ACE_TString username;
        RemoteFile() : channelid(0), fileid(0) { }
    };

    typedef std::vector< RemoteFile > files_t;

    enum RGBMode
    {
        BMP_NONE            = 0,
        BMP_RGB8_PALETTE    = 1,
        BMP_RGB16_555       = 2,
        BMP_RGB24           = 3,
        BMP_RGB32           = 4
    };

    enum DesktopProtocol
    {
        DESKTOPPROTOCOL_NONE    = 0,
        DESKTOPPROTOCOL_ZLIB_1  = 1
    };

    struct DesktopWindow
    {
        int session_id;
        int width;
        int height;
        RGBMode rgb_mode;
        DesktopProtocol desktop_protocol;
        DesktopWindow()
            : session_id(0)
            , width(0)
            , height(0)
            , rgb_mode(BMP_NONE)
            , desktop_protocol(DESKTOPPROTOCOL_NONE)
        {}
        DesktopWindow(int s_id, int w, int h, RGBMode mode, 
                      DesktopProtocol dskproto)
            : session_id(s_id)
            , width(w)
            , height(h)
            , rgb_mode(mode)
            , desktop_protocol(dskproto)
        {}
    };


    /* Remember to updated DLL header file when modifying this */
    enum ChannelType
    {
        CHANNEL_DEFAULT             = 0x0000,
        CHANNEL_PERMANENT           = 0x0001,
        CHANNEL_SOLO_TRANSMIT       = 0x0002,
        CHANNEL_CLASSROOM           = 0x0004,
        CHANNEL_OPERATOR_RECVONLY   = 0x0008,
        CHANNEL_NO_VOICEACTIVATION  = 0x0010,
        CHANNEL_NO_RECORDING        = 0x0020
    };

    typedef ACE_UINT32 ChannelTypes;

    typedef std::map< StreamType, std::set<int> > transmitusers_t;

    struct ChannelProp
    {
        ACE_TString name;
        ACE_TString passwd;
        ACE_TString topic;
        ACE_TString oppasswd;
        ACE_INT64 diskquota;
        int maxusers;
        bool bProtected;
        std::set<int> setops;
        int channelid;
        int parentid;
        AudioCodec audiocodec;
        AudioConfig audiocfg;
        files_t files;
        ChannelTypes chantype;
        ACE_UINT32 chankey;
        int userdata;
        transmitusers_t transmitusers;
        std::vector<int> transmitqueue;
        bannedusers_t bans;
        std::set<int> GetTransmitUsers(StreamType st) const
        {
            if(transmitusers.find(st) != transmitusers.end())
                return transmitusers.at(st);
            return std::set<int>();
        }

        ChannelProp()
        {
            bProtected = false;
            channelid = parentid = 0;
            diskquota = 0;
            chankey = 0;
            userdata = 0;
            maxusers = MAX_USERS_IN_CHANNEL;
            memset(&audiocodec, 0, sizeof(audiocodec));
            audiocodec.codec = CODEC_NO_CODEC;
            chantype = CHANNEL_DEFAULT;

            // ensure we can use std::map<>.at()
            transmitusers[STREAMTYPE_VOICE] = std::set<int>();
            transmitusers[STREAMTYPE_VIDEOCAPTURE] = std::set<int>();
            transmitusers[STREAMTYPE_DESKTOP] = std::set<int>();
            transmitusers[STREAMTYPE_MEDIAFILE] = std::set<int>();
        }
    };

    enum KeyState
    {
        KEYSTATE_NONE       = 0x00000000,
        KEYSTATE_DOWN       = 0x00000001,
        KEYSTATE_UP         = 0x00000002,
    };

    typedef ACE_UINT32 KeyStateMask;

    struct DesktopInput
    {
        ACE_UINT16 x;
        ACE_UINT16 y;
        ACE_UINT32 keycode;
        KeyStateMask keystate;
        DesktopInput()
        : x(-1)
        , y(-1)
        , keycode(-1)
        , keystate(KEYSTATE_NONE)
        { }
    };

    /* Remember to updated DLL header file when modifying this */
    enum AudioFileFormat
    {
        AFF_NONE                 = 0,
        AFF_CHANNELCODEC_FORMAT  = 1,
        AFF_WAVE_FORMAT          = 2,
        AFF_MP3_16KBIT_FORMAT    = 3,
        AFF_MP3_32KBIT_FORMAT    = 4,
        AFF_MP3_64KBIT_FORMAT    = 5,
        AFF_MP3_128KBIT_FORMAT   = 6,
        AFF_MP3_256KBIT_FORMAT   = 7,
    };

    int AFFToMP3Bitrate(AudioFileFormat aff);

    /* Remember to updated DLL header file when modifying this */
    enum MediaFileStatus
    {
        MFS_CLOSED      = 0,
        MFS_ERROR       = 1,
        MFS_STARTED     = 2,
        MFS_FINISHED    = 3,
        MFS_ABORTED     = 4,
        MFS_PAUSED      = 5,
        MFS_PLAYING     = 6
    };

    /* Remember to updated DLL header file when modifying this.
     * If more than 16 bits ServerUser subscription model will be broken. */
    enum
    {
        SUBSCRIBE_NONE                                  = 0x00000000,
        SUBSCRIBE_USER_MSG                              = 0x00000001,
        SUBSCRIBE_CHANNEL_MSG                           = 0x00000002,
        SUBSCRIBE_BROADCAST_MSG                         = 0x00000004,
        SUBSCRIBE_CUSTOM_MSG                            = 0x00000008,

        SUBSCRIBE_VOICE                                 = 0x00000010,
        SUBSCRIBE_VIDEOCAPTURE                          = 0x00000020,
        SUBSCRIBE_DESKTOP                               = 0x00000040,
        SUBSCRIBE_DESKTOPINPUT                          = 0x00000080,
        SUBSCRIBE_MEDIAFILE                             = 0x00000100,

        SUBSCRIBE_ALL                                   = 0x000001FF,

        SUBSCRIBE_LOCAL_DEFAULT = (SUBSCRIBE_USER_MSG |
                                   SUBSCRIBE_CHANNEL_MSG |
                                   SUBSCRIBE_BROADCAST_MSG |
                                   SUBSCRIBE_CUSTOM_MSG |
                                   SUBSCRIBE_MEDIAFILE),
        SUBSCRIBE_PEER_DEFAULT = (SUBSCRIBE_ALL & ~SUBSCRIBE_DESKTOPINPUT),


        SUBSCRIBE_INTERCEPT_USER_MSG                    = 0x00010000,
        SUBSCRIBE_INTERCEPT_CHANNEL_MSG                 = 0x00020000,
        /*SUBSCRIBE_INTERCEPT_BROADCAST_MSG             = 0x00040000, */
        SUBSCRIBE_INTERCEPT_CUSTOM_MSG                  = 0x00080000,

        SUBSCRIBE_INTERCEPT_VOICE                       = 0x00100000,
        SUBSCRIBE_INTERCEPT_VIDEOCAPTURE                = 0x00200000,
        SUBSCRIBE_INTERCEPT_DESKTOP                     = 0x00400000,
        /*SUBSCRIBE_INTERCEPT_DESKTOPINPUT              = 0x00800000, */
        SUBSCRIBE_INTERCEPT_MEDIAFILE                   = 0x01000000,

        SUBSCRIBE_INTERCEPT_ALL                         = 0x017B0000,
    };

    typedef ACE_UINT32 Subscriptions;

    std::vector<int> ConvertFrameSizes(const std::vector<uint16_t>& in);
    int SumFrameSizes(const std::vector<uint16_t>& in);
    std::vector<uint16_t> ConvertFrameSizes(const std::vector<int>& in);
    int SumFrameSizes(const std::vector<int>& in);

#define TRANSMITUSERS_FREEFORALL 0xFFF

#define PACKETNO_GEQ(a,b) ((int16_t)((a)-(b)) >= 0)
#define STREAMID_GT(a,b) ((int8_t)((a)-(b)) > 0)
#define SESSIONID_GEQ(a,b) ((int8_t)((a)-(b)) >= 0)
}
#endif
