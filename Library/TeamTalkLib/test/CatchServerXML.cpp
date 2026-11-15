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

#include <catch2/catch_test_macros.hpp>

#include "bin/ttsrv/ServerXML.h"
#include "codec/OpusEncoder.h" // OPUS_APPLICATION_AUDIO/OPUS_APPLICATION_VOIP
#include "myace/MyACE.h"

#include <array>
#include <cstdio>
#include <string>
#include <vector>

#if defined(WIN32)
#undef CreateFile
#endif

using namespace teamtalk;

namespace {
    std::string GetTempFilePath(const std::string& name)
    {
#if defined(WIN32)
        std::array<char, MAX_PATH> tmpPath{};
        GetTempPathA(MAX_PATH, tmpPath.data());
        return std::string(tmpPath.data()) + name;
#else
        return std::string("/tmp/") + name;
#endif
    }

    void RemoveFile(const std::string& path)
    {
        std::remove(path.c_str());
    }
}

TEST_CASE("ServerXML Channels Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_channels.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        statchannels_t channels;

        constexpr auto ROOT_OPUS_SAMPLE_RATE_HZ = 48000;
        constexpr auto ROOT_OPUS_CHANNEL_COUNT = 2;
        constexpr auto ROOT_OPUS_COMPLEXITY = 10;
        constexpr auto ROOT_OPUS_BITRATE_BPS = 64000;
        constexpr auto ROOT_OPUS_FRAME_SIZE_SAMPLES = 1920;
        constexpr auto ROOT_AUDIO_GAIN_LEVEL = 8000;
        constexpr auto ROOT_TRANSMIT_SWITCH_DELAY_MSEC = 500;

        ChannelProp root;
        root.channelid = 1;
        root.parentid = 0;
        root.name = TTEXT("");
        root.passwd = TTEXT("rootpwd");
        root.topic = TTEXT("Root Channel Topic");
        root.diskquota = 51200000;
        root.oppasswd = TTEXT("oppwd");
        root.maxusers = 1000;
        root.chantype = CHANNEL_PERMANENT;
        root.userdata = 12345;
        root.audiocodec.codec = CODEC_OPUS;
        root.audiocodec.opus.samplerate = ROOT_OPUS_SAMPLE_RATE_HZ;
        root.audiocodec.opus.channels = ROOT_OPUS_CHANNEL_COUNT;
        root.audiocodec.opus.application = OPUS_APPLICATION_VOIP;
        root.audiocodec.opus.complexity = ROOT_OPUS_COMPLEXITY;
        root.audiocodec.opus.fec = true;
        root.audiocodec.opus.dtx = false;
        root.audiocodec.opus.bitrate = ROOT_OPUS_BITRATE_BPS;
        root.audiocodec.opus.vbr = true;
        root.audiocodec.opus.vbr_constraint = false;
        root.audiocodec.opus.frame_size = ROOT_OPUS_FRAME_SIZE_SAMPLES;
        root.audiocfg.enable_agc = true;
        root.audiocfg.gain_level = ROOT_AUDIO_GAIN_LEVEL;
        root.transmitusers[STREAMTYPE_VOICE].insert(TRANSMITUSERS_FREEFORALL);
        root.transmitswitchdelay = ROOT_TRANSMIT_SWITCH_DELAY_MSEC;
        root.totvoice = 30000;
        root.totmediafile = 60000;
        channels[root.channelid] = root;

        constexpr auto CHILD_OPUS_SAMPLE_RATE_HZ = 24000;
        constexpr auto CHILD_OPUS_CHANNEL_COUNT = 1;
        constexpr auto CHILD_OPUS_COMPLEXITY = 5;
        constexpr auto CHILD_OPUS_BITRATE_BPS = 32000;
        constexpr auto CHILD_OPUS_FRAME_SIZE_SAMPLES = 960;

        ChannelProp child1;
        child1.channelid = 2;
        child1.parentid = 1;
        child1.name = TTEXT("Sub Channel 1");
        child1.passwd = TTEXT("sub1pwd");
        child1.topic = TTEXT("Sub Channel 1 Topic");
        child1.diskquota = 10240000;
        child1.maxusers = 100;
        child1.chantype = CHANNEL_PERMANENT | CHANNEL_CLASSROOM;
        child1.audiocodec.codec = CODEC_OPUS;
        child1.audiocodec.opus.samplerate = CHILD_OPUS_SAMPLE_RATE_HZ;
        child1.audiocodec.opus.channels = CHILD_OPUS_CHANNEL_COUNT;
        child1.audiocodec.opus.application = OPUS_APPLICATION_AUDIO;
        child1.audiocodec.opus.complexity = CHILD_OPUS_COMPLEXITY;
        child1.audiocodec.opus.fec = false;
        child1.audiocodec.opus.dtx = true;
        child1.audiocodec.opus.bitrate = CHILD_OPUS_BITRATE_BPS;
        child1.audiocodec.opus.vbr = false;
        child1.audiocodec.opus.vbr_constraint = true;
        child1.audiocodec.opus.frame_size = CHILD_OPUS_FRAME_SIZE_SAMPLES;
        channels[child1.channelid] = child1;

        ChannelProp child2;
        child2.channelid = 3;
        child2.parentid = 1;
        child2.name = TTEXT("Sub Channel 2");
        child2.passwd = TTEXT("");
        child2.topic = TTEXT("Open Channel");
        child2.maxusers = 50;
        child2.chantype = CHANNEL_PERMANENT;
        child2.audiocodec.codec = CODEC_NO_CODEC;
        channels[child2.channelid] = child2;

        REQUIRE(xml.SetStaticChannels(channels));
        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        statchannels_t readChannels;
        REQUIRE(xml.GetStaticChannels(readChannels));
        REQUIRE(readChannels.size() == 3);

        REQUIRE(readChannels.contains(1));
        auto& rootRead = readChannels[1];
        REQUIRE(rootRead.parentid == 0);
        REQUIRE(rootRead.passwd == TTEXT("rootpwd"));
        REQUIRE(rootRead.topic == TTEXT("Root Channel Topic"));
        REQUIRE(rootRead.diskquota == 51200000);
        REQUIRE(rootRead.oppasswd == TTEXT("oppwd"));
        REQUIRE(rootRead.maxusers == 1000);
        REQUIRE((rootRead.chantype & CHANNEL_PERMANENT) != CHANNEL_DEFAULT);
        REQUIRE(rootRead.userdata == 12345);
        REQUIRE(rootRead.audiocodec.codec == CODEC_OPUS);
        REQUIRE(rootRead.audiocodec.opus.samplerate == 48000);
        REQUIRE(rootRead.audiocodec.opus.channels == 2);
        REQUIRE(rootRead.audiocodec.opus.application == 2048);
        REQUIRE(rootRead.audiocodec.opus.complexity == 10);
        REQUIRE(rootRead.audiocodec.opus.fec == true);
        REQUIRE(rootRead.audiocodec.opus.dtx == false);
        REQUIRE(rootRead.audiocodec.opus.bitrate == 64000);
        REQUIRE(rootRead.audiocodec.opus.vbr == true);
        REQUIRE(rootRead.audiocodec.opus.vbr_constraint == false);
        REQUIRE(rootRead.audiocodec.opus.frame_size == 1920);
        REQUIRE(rootRead.audiocfg.enable_agc == true);
        REQUIRE(rootRead.audiocfg.gain_level == 8000);
        REQUIRE(rootRead.transmitusers[STREAMTYPE_VOICE].contains(TRANSMITUSERS_FREEFORALL));
        REQUIRE(rootRead.transmitswitchdelay == 500);
        REQUIRE(rootRead.totvoice == 30000);
        REQUIRE(rootRead.totmediafile == 60000);

        REQUIRE(readChannels.contains(2));
        auto& child1Read = readChannels[2];
        REQUIRE(child1Read.parentid == 1);
        REQUIRE(child1Read.name == TTEXT("Sub Channel 1"));
        REQUIRE(child1Read.passwd == TTEXT("sub1pwd"));
        REQUIRE(child1Read.topic == TTEXT("Sub Channel 1 Topic"));
        REQUIRE(child1Read.diskquota == 10240000);
        REQUIRE(child1Read.maxusers == 100);
        REQUIRE((child1Read.chantype & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT);
        REQUIRE(child1Read.audiocodec.codec == CODEC_OPUS);
        REQUIRE(child1Read.audiocodec.opus.samplerate == 24000);
        REQUIRE(child1Read.audiocodec.opus.channels == 1);
        REQUIRE(child1Read.audiocodec.opus.application == 2049);
        REQUIRE(child1Read.audiocodec.opus.complexity == 5);
        REQUIRE(child1Read.audiocodec.opus.fec == false);
        REQUIRE(child1Read.audiocodec.opus.dtx == true);
        REQUIRE(child1Read.audiocodec.opus.bitrate == 32000);
        REQUIRE(child1Read.audiocodec.opus.vbr == false);
        REQUIRE(child1Read.audiocodec.opus.vbr_constraint == true);
        REQUIRE(child1Read.audiocodec.opus.frame_size == 960);

        REQUIRE(readChannels.contains(3));
        auto& child2Read = readChannels[3];
        REQUIRE(child2Read.parentid == 1);
        REQUIRE(child2Read.name == TTEXT("Sub Channel 2"));
        REQUIRE(child2Read.passwd == TTEXT(""));
        REQUIRE(child2Read.audiocodec.codec == CODEC_NO_CODEC);
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML WAV Channel Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_wav_channels.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        ChannelProp root;
        root.channelid = 1;
        root.parentid = 0;
        root.chantype = CHANNEL_PERMANENT;
        root.audiocodec.codec = CODEC_WAV;
        root.audiocodec.wav.samplerate = 192000;
        root.audiocodec.wav.channels = 2;
        root.audiocodec.wav.sampleformat = WAV_SAMPLEFORMAT_UINT8;
        root.audiocodec.wav.txinterval_msec = 20;

        statchannels_t channels;
        channels[root.channelid] = root;
        REQUIRE(xml.SetStaticChannels(channels));
        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        statchannels_t readChannels;
        REQUIRE(xml.GetStaticChannels(readChannels));
        REQUIRE(readChannels.contains(1));
        auto& rootRead = readChannels[1];
        REQUIRE(rootRead.audiocodec.codec == CODEC_WAV);
        REQUIRE(rootRead.audiocodec.wav.samplerate == 192000);
        REQUIRE(rootRead.audiocodec.wav.channels == 2);
        REQUIRE(rootRead.audiocodec.wav.sampleformat == WAV_SAMPLEFORMAT_UINT8);
        REQUIRE(rootRead.audiocodec.wav.txinterval_msec == 20);
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML User Accounts Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_users.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        UserAccount admin;
        admin.username = TTEXT("admin");
        admin.passwd = TTEXT("adminpass123");
        admin.usertype = USERTYPE_ADMIN;
        admin.userrights = USERRIGHT_ALL;
        admin.note = TTEXT("Administrator account");
        admin.userdata = 999;
        admin.init_channel = TTEXT("/");
        admin.audiobpslimit = 128000;
        admin.abuse.n_cmds = 10;
        admin.abuse.cmd_msec = 1000;
        admin.auto_op_channels.insert(1);
        admin.auto_op_channels.insert(2);
        xml.AddNewUser(admin);

        UserAccount guest;
        guest.username = TTEXT("guest");
        guest.passwd = TTEXT("guestpass");
        guest.usertype = USERTYPE_DEFAULT;
        guest.userrights = USERRIGHT_DEFAULT;
        guest.note = TTEXT("Guest user");
        guest.userdata = 0;
        guest.init_channel = TTEXT("/lobby/");
        guest.audiobpslimit = 64000;
        xml.AddNewUser(guest);

        UserAccount special;
        special.username = TTEXT("special_user");
        special.passwd = TTEXT("specialpass");
        special.usertype = USERTYPE_DEFAULT;
        special.userrights = USERRIGHT_TRANSMIT_VOICE | USERRIGHT_TRANSMIT_VIDEOCAPTURE;
        special.note = TTEXT("Special permissions user");
        special.userdata = 42;
        special.audiobpslimit = 0;
        special.abuse.n_cmds = 5;
        special.abuse.cmd_msec = 500;
        xml.AddNewUser(special);

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        UserAccount adminRead;
        REQUIRE(xml.GetUser("admin", adminRead));
        REQUIRE(adminRead.username == TTEXT("admin"));
        REQUIRE(adminRead.passwd == TTEXT("adminpass123"));
        REQUIRE(adminRead.usertype == USERTYPE_ADMIN);
        REQUIRE(adminRead.userrights == USERRIGHT_ALL);
        REQUIRE(adminRead.note == TTEXT("Administrator account"));
        REQUIRE(adminRead.userdata == 999);
        REQUIRE(adminRead.init_channel == TTEXT("/"));
        REQUIRE(adminRead.audiobpslimit == 128000);
        REQUIRE(adminRead.abuse.n_cmds == 10);
        REQUIRE(adminRead.abuse.cmd_msec == 1000);
        REQUIRE(adminRead.auto_op_channels.contains(1));
        REQUIRE(adminRead.auto_op_channels.contains(2));

        UserAccount guestRead;
        REQUIRE(xml.GetUser("guest", guestRead));
        REQUIRE(guestRead.username == TTEXT("guest"));
        REQUIRE(guestRead.passwd == TTEXT("guestpass"));
        REQUIRE(guestRead.usertype == USERTYPE_DEFAULT);
        REQUIRE(guestRead.userrights == USERRIGHT_DEFAULT);
        REQUIRE(guestRead.note == TTEXT("Guest user"));
        REQUIRE(guestRead.init_channel == TTEXT("/lobby/"));
        REQUIRE(guestRead.audiobpslimit == 64000);

        UserAccount specialRead;
        REQUIRE(xml.GetUser("special_user", specialRead));
        REQUIRE(specialRead.username == TTEXT("special_user"));
        REQUIRE(specialRead.passwd == TTEXT("specialpass"));
        REQUIRE((specialRead.userrights & USERRIGHT_TRANSMIT_VOICE) != USERRIGHT_NONE);
        REQUIRE((specialRead.userrights & USERRIGHT_TRANSMIT_VIDEOCAPTURE) != USERRIGHT_NONE);
        REQUIRE(specialRead.abuse.n_cmds == 5);
        REQUIRE(specialRead.abuse.cmd_msec == 500);

        UserAccount nonexistent;
        REQUIRE_FALSE(xml.GetUser("nonexistent", nonexistent));

        REQUIRE(xml.RemoveUser("guest"));
        REQUIRE_FALSE(xml.GetUser("guest", guestRead));
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML Bans Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_bans.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        BannedUser ipBan;
        ipBan.bantype = BANTYPE_IPADDR;
        ipBan.ipaddr = TTEXT("192.168.1.100");
        ipBan.nickname = TTEXT("BadUser");
        ipBan.username = TTEXT("baduser");
        ipBan.chanpath = TTEXT("");
        ipBan.owner = TTEXT("admin");
        xml.AddUserBan(ipBan);

        BannedUser usernameBan;
        usernameBan.bantype = BANTYPE_USERNAME;
        usernameBan.ipaddr = TTEXT("");
        usernameBan.nickname = TTEXT("Spammer");
        usernameBan.username = TTEXT("spammer123");
        usernameBan.chanpath = TTEXT("");
        usernameBan.owner = TTEXT("moderator");
        xml.AddUserBan(usernameBan);

        BannedUser channelBan;
        channelBan.bantype = BANTYPE_CHANNEL;
        channelBan.ipaddr = TTEXT("10.0.0.50");
        channelBan.nickname = TTEXT("Troll");
        channelBan.username = TTEXT("troll");
        channelBan.chanpath = TTEXT("/lobby/");
        channelBan.owner = TTEXT("chanop");
        xml.AddUserBan(channelBan);

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        REQUIRE(xml.GetUserBanCount() == 3);

        BannedUser ban0;
        REQUIRE(xml.GetUserBan(0, ban0));
        REQUIRE(ban0.bantype == BANTYPE_IPADDR);
        REQUIRE(ban0.ipaddr == TTEXT("192.168.1.100"));
        REQUIRE(ban0.nickname == TTEXT("BadUser"));
        REQUIRE(ban0.username == TTEXT("baduser"));
        REQUIRE(ban0.owner == TTEXT("admin"));

        BannedUser ban1;
        REQUIRE(xml.GetUserBan(1, ban1));
        REQUIRE(ban1.bantype == BANTYPE_USERNAME);
        REQUIRE(ban1.username == TTEXT("spammer123"));
        REQUIRE(ban1.nickname == TTEXT("Spammer"));
        REQUIRE(ban1.owner == TTEXT("moderator"));

        BannedUser ban2;
        REQUIRE(xml.GetUserBan(2, ban2));
        REQUIRE(ban2.bantype == BANTYPE_CHANNEL);
        REQUIRE(ban2.chanpath == TTEXT("/lobby/"));
        REQUIRE(ban2.username == TTEXT("troll"));
        REQUIRE(ban2.owner == TTEXT("chanop"));

        std::vector<BannedUser> allBans = xml.GetUserBans();
        REQUIRE(allBans.size() == 3);

        BannedUser checkBan;
        checkBan.bantype = BANTYPE_IPADDR;
        checkBan.ipaddr = TTEXT("192.168.1.100");
        REQUIRE(xml.IsUserBanned(checkBan));

        REQUIRE(xml.RemoveUserBan(ban0));
        REQUIRE(xml.GetUserBanCount() == 2);

        xml.ClearUserBans();
        REQUIRE(xml.GetUserBanCount() == 0);
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML Encryption Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_encryption.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        REQUIRE(xml.SetCertificateFile("server_cert.pem"));
        REQUIRE(xml.SetPrivateKeyFile("server_key.pem"));
        xml.SetCertificateAuthFile("ca_cert.pem");
        xml.SetCertificateAuthDir("/etc/ssl/certs");
        xml.SetCertificateVerify(true);
        xml.SetCertificateVerifyOnce(false);
        xml.SetCertificateVerifyDepth(3);

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        REQUIRE(xml.GetCertificateFile() == "server_cert.pem");
        REQUIRE(xml.GetPrivateKeyFile() == "server_key.pem");
        REQUIRE(xml.GetCertificateAuthFile() == "ca_cert.pem");
        REQUIRE(xml.GetCertificateAuthDir() == "/etc/ssl/certs");
        REQUIRE(xml.GetCertificateVerify(false) == true);
        REQUIRE(xml.GetCertificateVerifyOnce(true) == false);
        REQUIRE(xml.GetCertificateVerifyDepth(0) == 3);
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        REQUIRE(xml.GetCertificateFile().empty());
        REQUIRE(xml.GetPrivateKeyFile().empty());
        REQUIRE(xml.GetCertificateAuthFile().empty());
        REQUIRE(xml.GetCertificateAuthDir().empty());
        REQUIRE(xml.GetCertificateVerify(true) == true);
        REQUIRE(xml.GetCertificateVerifyOnce(false) == false);
        REQUIRE(xml.GetCertificateVerifyDepth(5) == 5);
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML Bind-IP Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_bindip.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        std::vector<std::string> ips;
        ips.push_back("192.168.1.1");
        ips.push_back("10.0.0.1");
        ips.push_back("::1");
        ips.push_back("2001:db8::1");

        REQUIRE(xml.SetBindIPs(ips));
        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        std::vector<std::string> readIps = xml.GetBindIPs();
        REQUIRE(readIps.size() == 4);
        REQUIRE(readIps[0] == "192.168.1.1");
        REQUIRE(readIps[1] == "10.0.0.1");
        REQUIRE(readIps[2] == "::1");
        REQUIRE(readIps[3] == "2001:db8::1");
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        std::vector<std::string> singleIp;
        singleIp.push_back("0.0.0.0");
        REQUIRE(xml.SetBindIPs(singleIp));
        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        std::vector<std::string> readIps = xml.GetBindIPs();
        REQUIRE(readIps.size() == 1);
        REQUIRE(readIps[0] == "0.0.0.0");
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        std::vector<std::string> emptyIps;
        REQUIRE(xml.SetBindIPs(emptyIps));
        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        std::vector<std::string> readIps = xml.GetBindIPs();
        REQUIRE(readIps.empty());
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML File-Storage Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_filestorage.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        REQUIRE(xml.SetFilesRoot("/var/teamtalk/files"));
        REQUIRE(xml.SetMaxDiskUsage(1073741824));
        REQUIRE(xml.SetDefaultDiskQuota(104857600));

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        REQUIRE(xml.GetFilesRoot() == "/var/teamtalk/files");
        REQUIRE(xml.GetMaxDiskUsage() == 1073741824);
        REQUIRE(xml.GetDefaultDiskQuota() == 104857600);
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        REQUIRE(xml.SetFilesRoot("C:\\TeamTalk\\Files"));
        REQUIRE(xml.SetMaxDiskUsage(10737418240LL));
        REQUIRE(xml.SetDefaultDiskQuota(1073741824LL));

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        REQUIRE(xml.GetFilesRoot() == "C:\\TeamTalk\\Files");
        REQUIRE(xml.GetMaxDiskUsage() == 10737418240LL);
        REQUIRE(xml.GetDefaultDiskQuota() == 1073741824LL);
    }

    RemoveFile(xmlFile);
}

TEST_CASE("ServerXML WebLogin Write/Read")
{
    std::string const xmlFile = GetTempFilePath("test_weblogin.xml");
    RemoveFile(xmlFile);

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        xml.SetBearWareWebLogin("testuser@example.com",
                                "abc123def456ghi789jkl012mno345pqr678stu901vwx234yz");

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        std::string username, token;
        REQUIRE(xml.GetBearWareWebLogin(username, token));
        REQUIRE(username == "testuser@example.com");
        REQUIRE(token == "abc123def456ghi789jkl012mno345pqr678stu901vwx234yz");
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        std::string username, token;
        REQUIRE_FALSE(xml.GetBearWareWebLogin(username, token));
        REQUIRE(username.empty());
        REQUIRE(token.empty());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.CreateFile(xmlFile));

        xml.SetBearWareWebLogin("user@bearware.dk", "");

        REQUIRE(xml.SaveFile());
    }

    {
        ServerXML xml("teamtalk");
        REQUIRE(xml.LoadFile(xmlFile));

        std::string username, token;
        REQUIRE_FALSE(xml.GetBearWareWebLogin(username, token));
    }

    RemoveFile(xmlFile);
}
