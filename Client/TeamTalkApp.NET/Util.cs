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

using System;
using System.Collections.Generic;
using System.Windows.Forms;
using System.Text;
using BearWare;

namespace TeamTalkApp.NET
{
    class Util
    {
        public static bool Equals(VideoCodec c1, VideoCodec c2)
        {
            return c1.nCodec == c2.nCodec &&
                c1.webm_vp8.nRcTargetBitrate == c2.webm_vp8.nRcTargetBitrate;
        }

        public static bool Equals(VideoFormat c1, VideoFormat c2)
        {
            return c1.nFPS_Denominator == c2.nFPS_Denominator &&
                c1.nFPS_Numerator == c2.nFPS_Numerator &&
                c1.nHeight == c2.nHeight && c1.nWidth == c2.nWidth &&
                c1.picFourCC == c2.picFourCC;
        }
    }

    public class Settings
    {
        public SoundSystem soundsystem = SoundSystem.SOUNDSYSTEM_WASAPI;
        public int sndinputid;
        public int sndoutputid;
        public string videoid;
        public VideoCodec codec;
        public VideoFormat capformat;
        public Server server;
        public string nickname;
        public string audiofolder;
        public bool muxed_audio_file;
        public AudioFileFormat aff;

        public Settings()
        {
            codec.nCodec = Codec.WEBM_VP8_CODEC;
            codec.webm_vp8.nRcTargetBitrate = 256;
            codec.webm_vp8.nEncodeDeadline = WebMVP8CodecConstants.WEBM_VPX_DL_REALTIME;

            server.ipaddr = "localhost";
#if ENABLE_TEAMTALKPRO
            server.tcpport = TeamTalk5Pro.DEFAULT_TCPPORT;
            server.udpport = TeamTalk5Pro.DEFAULT_UDPPORT;
#else
            server.tcpport = TeamTalk5.DEFAULT_TCPPORT;
            server.udpport = TeamTalk5.DEFAULT_UDPPORT;
#endif
            server.username = "guest";
            server.password = "guest";

            nickname = "John Doe";
            audiofolder = "";
            muxed_audio_file = false;
        }
    }

    public struct Server
    {
        public string name;
        public string ipaddr;
        public int tcpport;
        public int udpport;
        public bool encrypted;

        public string username;
        public string password;

        public string channel;
        public string chanpasswd;

        public override string ToString()
        {
            return name;
        }
    }
}
