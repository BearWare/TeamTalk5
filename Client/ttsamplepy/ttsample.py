import TeamTalk5


class TTClient():
    def __init__(self, host, tcpPort=10333, udpPort=10333, nickName="", userName="", password=""):
        self.host = host
        self.tcpPort = tcpPort
        self.udpPort = udpPort
        self.nickName = nickName
        self.userName = userName
        self.password = password
        self.tt = TeamTalk5.TeamTalk()
        self.tt.onConSuccess = self.onConSuccess
        self.tt.onMyselfLoggedIn = self.onMyselfLoggedIn
        self.tt.onMyselfKicked = self.onMyselfKicked
        self.tt.onUserLoggedIn = self.onUserLoggedIn
        self.tt.onUserLoggedOut = self.onUserLoggedOut
        self.tt.onUserUpdate = self.onUserUpdate
        self.tt.onUserJoined = self.onUserJoined
        self.tt.onUserLeft = self.onUserLeft
        self.tt.onChannelNew = self.onChannelNew
        self.tt.onChannelUpdate = self.onChannelUpdate
        self.tt.onChannelRemove = self.onChannelRemove
        self.tt.onTextMessage = self.onTextMessage
        self.tt.onServerUpdate = self.onServerUpdate
        self.tt.onFileNew = self.onFileNew
        self.tt.onFileRemove = self.onFileRemove

    def start(self):
        self.connect()

    def connect(self):
        self.tt.connect(self.host, self.tcpPort, self.udpPort)

    def onConSuccess(self):
        self.tt.doLogin(self.nickName, self.userName, self.password, "ttsamplepy")

    def onConLost(self):
        self.connect()

    def onMyselfLoggedIn(self,userID, userAccount):
        print(f"Hello {userAccount.szUsername}. Your User ID is {userID}")
        channelID = self.tt.getChannelIDFromPath("/testChannel/")
        self.tt.doJoinChannelByID(channelID, "")

    def onMyselfKicked(self, channelID, user):
        print(f"kicked from {channelID} by {user.szUsername}")

    def onUserLoggedIn(self, user):
        print(f"{user.szUsername} with nickname {user.szNickname} has logged in")

    def onUserLoggedOut(self, user):
        print(f"{user.szUsername} with nickname {user.szNickname} has logged out")

    def onUserUpdate(self, user):
        print(f"{user.szUsername}was updated")

    def onUserJoined(self, user):
        channel = self.tt.getChannel(user.nChannelID)
        print(f"{user.szUsername} with nickname {user.szNickname} has joined to channel {channel.szName}")

    def onUserLeft(self, channelID, user):
        channel = self.tt.getChannel(channelID)
        print(f"{user.szUsername} with nickname {user.szNickname} has left channel {channel.szName}")

    def onChannelNew(self, channel):
        print(f"channel {channel.szName} was added")

    def onChannelUpdate(self, channel):
        print(f"channel {channel.szName} was updated")

    def onChannelRemove(self, channel):
        print(f"channel {channel.szName} was removed")

    def onTextMessage(self, message):
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

    def onServerUpdate(self, serverProperties):
        print(f"Welcome to server {serverProperties.szServerName}")

    def onFileNew(self, remoteFile):
        print(f"file {remoteFile.szFileName} was added")

    def onFileRemove(self, remoteFile):
        print(f"file {remoteFile.szFileName} was removed")

if __name__ == "__main__":
    try:
        ttClient = TTClient("localhost", 10333, 10333, "Nickname", "username", "password")
        ttClient.start()
        while True:
            ttClient.tt.runEventLoop()
    except Exception as e:
        print(e)
