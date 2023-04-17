import TeamTalk5
from TeamTalk5 import TextMsgType, buildTextMessage, ttstr

def test_ttypes():
    # Run DBG_SIZEOF() on all structs
    TeamTalk5.AudioCodec()
    TeamTalk5.BannedUser()
    TeamTalk5.VideoFormat()
    TeamTalk5.OpusCodec()
    TeamTalk5.Channel()
    TeamTalk5.ClientStatistics()
    TeamTalk5.RemoteFile()
    TeamTalk5.FileTransfer()
    # mfs = MediaFileStatus
    TeamTalk5.ServerProperties()
    TeamTalk5.ServerStatistics()
    TeamTalk5.SoundDevice()
    TeamTalk5.SpeexCodec()
    TeamTalk5.TextMessage()
    TeamTalk5.User()
    TeamTalk5.UserAccount()
    TeamTalk5.UserStatistics()
    TeamTalk5.VideoCaptureDevice()
    TeamTalk5.VideoCodec()
    TeamTalk5.AudioConfig()
    TeamTalk5.SpeexVBRCodec()
    TeamTalk5.VideoFrame()
    TeamTalk5.AudioBlock()
    TeamTalk5.AudioFormat()
    TeamTalk5.MediaFileInfo()
    TeamTalk5.ClientErrorMsg()
    TeamTalk5.DesktopInput()
    TeamTalk5.SpeexDSP()
    TeamTalk5.AudioPreprocessor()
    TeamTalk5.TTAudioPreprocessor()
    TeamTalk5.MediaFilePlayback()
    TeamTalk5.ClientKeepAlive()
    TeamTalk5.AudioInputProgress()
    TeamTalk5.JitterConfig()
    TeamTalk5.WebRTCAudioPreprocessor()
    TeamTalk5.EncryptionContext()

def test_textmessagelength():
    allchars = ''.join(['%c' % x for x in range(97, 97+26)])
    content = ""
    for _ in range(0, 500):
        content += allchars

    msgs = buildTextMessage(content, nMsgType = TextMsgType.MSGTYPE_USER,
                            szFromUsername = "hest",
                            nChannelID = 0, nToUserID = 55)
    for m in msgs[0:len(msgs)-2]:
        assert m.nMsgType == TextMsgType.MSGTYPE_USER
        assert m.nChannelID == 0
        assert m.szFromUsername == ttstr("hest")
        assert m.nToUserID == 55
        assert m.bMore == True
    assert msgs[-1].bMore == False
    result = ""
    for m in msgs:
        result += ttstr(m.szMessage)
    assert content == result
