import TeamTalk5
from TeamTalk5 import ttstr


class TTClient:
    def __init__(self, host, tcpPort=10333, udpPort=10333, nickName="", userName="", password=""):
        self.host = host
        self.tcpPort = tcpPort
        self.udpPort = udpPort
        self.nickName = nickName
        self.userName = userName
        self.password = password
        self.tt = TeamTalk5.TeamTalk()
        self.tt.onConnectSuccess = self.onConnectSuccess
        self.tt.onCmdMyselfLoggedIn = self.onCmdMyselfLoggedIn
        self.tt.onCmdMyselfKickedFromChannel = self.onCmdMyselfKickedFromChannel
        self.tt.onCmdUserLoggedIn = self.onCmdUserLoggedIn
        self.tt.onCmdUserLoggedOut = self.onCmdUserLoggedOut
        self.tt.onCmdUserUpdate = self.onCmdUserUpdate
        self.tt.onCmdUserJoinedChannel = self.onCmdUserJoinedChannel
        self.tt.onCmdUserLeftChannel = self.onCmdUserLeftChannel
        self.tt.onCmdChannelNew = self.onCmdChannelNew
        self.tt.onCmdChannelUpdate = self.onCmdChannelUpdate
        self.tt.onCmdChannelRemove = self.onCmdChannelRemove
        self.tt.onCmdUserTextMessage = self.onCmdUserTextMessage
        self.tt.onCmdServerUpdate = self.onCmdServerUpdate
        self.tt.onCmdFileNew = self.onCmdFileNew
        self.tt.onCmdFileRemove = self.onCmdFileRemove

    def start(self):
        self.connect()

    def connect(self):
        self.tt.connect(self.host, self.tcpPort, self.udpPort)

    def onConnectSuccess(self):
        self.tt.doLogin(self.nickName, self.userName, self.password, ttstr("ttsamplepy"))

    def onConnectionLost(self):
        self.connect()

    def onCmdMyselfLoggedIn(self, userID, userAccount):
        print(f"Hello {userAccount.szUsername}. Your User ID is {userID}")
        channelID = self.tt.getChannelIDFromPath(ttstr("/testChannel/"))
        self.tt.doJoinChannelByID(channelID, ttstr(""))

    def onCmdMyselfKickedFromChannel(self, channelID, user):
        print(f"kicked from {channelID} by {user.szUsername}")

    def onCmdUserLoggedIn(self, user):
        print(f"{user.szUsername} with nickname {user.szNickname} has logged in")

    def onCmdUserLoggedOut(self, user):
        print(f"{user.szUsername} with nickname {user.szNickname} has logged out")

    def onCmdUserUpdate(self, user):
        print(f"{user.szUsername}was updated")

    def onCmdUserJoinedChannel(self, user):
        channel = self.tt.getChannel(user.nChannelID)
        print(f"{user.szUsername} with nickname {user.szNickname} has joined to channel {channel.szName}")

    def onCmdUserLeftChannel(self, channelID, user):
        channel = self.tt.getChannel(channelID)
        print(f"{user.szUsername} with nickname {user.szNickname} has left channel {channel.szName}")

    def onCmdChannelNew(self, channel):
        print(f"channel {channel.szName} was added")

    def onCmdChannelUpdate(self, channel):
        print(f"channel {channel.szName} was updated")

    def onCmdChannelRemove(self, channel):
        print(f"channel {channel.szName} was removed")

    def onCmdUserTextMessage(self, message):
        msgType = message.nMsgType
        if msgType == TeamTalk5.TextMsgType.MSGTYPE_USER:
            self.onUserMessage(message.nFromUserID, message.szFromUsername, message.szMessage)
        if msgType == TeamTalk5.TextMsgType.MSGTYPE_CHANNEL:
            self.onChannelMessage(message.nFromUserID, message.szFromUsername, message.nChannelID, message.szMessage)
        if msgType == TeamTalk5.TextMsgType.MSGTYPE_BROADCAST:
            self.onBroadcastMessage(message.nFromUserID, message.szFromUsername, message.szMessage)

    def onUserMessage(self, fromUserID, fromUserName, msgText):
        print(f"User message from userid: {fromUserID}, username: {fromUserName} {msgText}")

    def onChannelMessage(self, fromUserID, fromUserName, channelID, msgText):
        print(f"Channel message in channelid {channelID} from userid {fromUserID} username: {fromUserName} {msgText}")

    def onBroadcastMessage(self, fromUserID, fromUserName, msgText):
        print(f"Broadcast message from userid: {fromUserID}, username: {fromUserName} {msgText}")

    def onCmdServerUpdate(self, serverProperties):
        print(f"Welcome to server {serverProperties.szServerName}")

    def onCmdFileNew(self, remoteFile):
        print(f"file {remoteFile.szFileName} was added")

    def onCmdFileRemove(self, remoteFile):
        print(f"file {remoteFile.szFileName} was removed")


if __name__ == "__main__":
    try:
        ttClient = TTClient(ttstr("localhost"), 10333, 10333, ttstr("Nickname"), ttstr("username"), ttstr("password"))
        ttClient.start()
        while True:
            ttClient.tt.runEventLoop()
    except Exception as e:
        print(e)
