/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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
            server.tcpport = TeamTalk.DEFAULT_TCPPORT;
            server.udpport = TeamTalk.DEFAULT_UDPPORT;
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
