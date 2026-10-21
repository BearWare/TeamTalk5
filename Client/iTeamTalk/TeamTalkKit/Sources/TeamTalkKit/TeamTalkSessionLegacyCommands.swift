import Foundation
import TeamTalkC

extension TeamTalkSession {
@discardableResult
internal func join(channel: inout Channel) -> Int32 {
    TT_DoJoinChannel(instance, &channel)
}

@discardableResult
internal func joinChannel(id channelID: Int32, password: String = "") -> Int32 {
    TT_DoJoinChannelByID(instance, channelID, password)
}

@discardableResult
internal func joinChannel(id channelID: TeamTalkChannelID, password: String = "") -> Int32 {
    joinChannel(id: channelID.cValue, password: password)
}

@discardableResult
internal func update(channel: inout Channel) -> Int32 {
    TT_DoUpdateChannel(instance, &channel)
}

@discardableResult
internal func removeChannel(id channelID: Int32) -> Int32 {
    TT_DoRemoveChannel(instance, channelID)
}

@discardableResult
internal func removeChannel(id channelID: TeamTalkChannelID) -> Int32 {
    removeChannel(id: channelID.cValue)
}

@discardableResult
internal func changeNickname(_ nickname: String) -> Int32 {
    TT_DoChangeNickname(instance, nickname)
}

@discardableResult
internal func changeStatus(mode: Int32, message: String = "") -> Int32 {
    TT_DoChangeStatus(instance, mode, message)
}

@discardableResult
internal func kickUser(id userID: Int32, fromChannelID channelID: Int32) -> Int32 {
    TT_DoKickUser(instance, userID, channelID)
}

@discardableResult
internal func kickUser(id userID: TeamTalkUserID, fromChannelID channelID: TeamTalkChannelID) -> Int32 {
    kickUser(id: userID.cValue, fromChannelID: channelID.cValue)
}

@discardableResult
internal func banUser(id userID: Int32, fromChannelID channelID: Int32) -> Int32 {
    TT_DoBanUser(instance, userID, channelID)
}

@discardableResult
internal func banUser(id userID: TeamTalkUserID, fromChannelID channelID: TeamTalkChannelID) -> Int32 {
    banUser(id: userID.cValue, fromChannelID: channelID.cValue)
}

@discardableResult
internal func moveUser(id userID: Int32, toChannelID channelID: Int32) -> Int32 {
    TT_DoMoveUser(instance, userID, channelID)
}

@discardableResult
internal func moveUser(id userID: TeamTalkUserID, toChannelID channelID: TeamTalkChannelID) -> Int32 {
    moveUser(id: userID.cValue, toChannelID: channelID.cValue)
}

@discardableResult
internal func sendTextMessage(_ message: inout TextMessage) -> Int32 {
    TT_DoTextMessage(instance, &message)
}

@discardableResult
internal func sendTextMessage(_ message: TextMessage, content: String) -> Bool {
    TeamTalkTextMessageFactory.messages(from: message, content: content).reduce(true) { sent, textMessage in
        var textMessage = textMessage
        return sent && sendTextMessage(&textMessage) > 0
    }
}

@discardableResult
internal func uploadFile(at localURL: URL, toChannelID channelID: Int32) -> Int32 {
    guard let instance else {
        return -1
    }

    return TT_DoSendFile(instance, channelID, localURL.path)
}

@discardableResult
internal func uploadFile(at localURL: URL, toChannelID channelID: TeamTalkChannelID) -> Int32 {
    uploadFile(at: localURL, toChannelID: channelID.cValue)
}

@discardableResult
internal func downloadFile(channelID: Int32, fileID: Int32, to localURL: URL) -> Int32 {
    guard let instance else {
        return -1
    }

    return TT_DoRecvFile(instance, channelID, fileID, localURL.path)
}

@discardableResult
internal func downloadFile(channelID: TeamTalkChannelID, fileID: TeamTalkFileID, to localURL: URL) -> Int32 {
    downloadFile(channelID: channelID.cValue, fileID: fileID.cValue, to: localURL)
}

@discardableResult
internal func deleteFile(channelID: Int32, fileID: Int32) -> Int32 {
    guard let instance else {
        return -1
    }

    return TT_DoDeleteFile(instance, channelID, fileID)
}

@discardableResult
internal func deleteFile(channelID: TeamTalkChannelID, fileID: TeamTalkFileID) -> Int32 {
    deleteFile(channelID: channelID.cValue, fileID: fileID.cValue)
}

internal func fileTransferInfo(id transferID: Int32) -> FileTransfer? {
    guard let instance else {
        return nil
    }

    var transfer = FileTransfer()
    guard TT_GetFileTransferInfo(instance, transferID, &transfer) != 0 else {
        return nil
    }
    return transfer
}

internal func fileTransferInfo(id transferID: TeamTalkTransferID) -> FileTransfer? {
    fileTransferInfo(id: transferID.cValue)
}

internal func fileTransfer(id transferID: Int32) -> TeamTalkFileTransfer? {
    fileTransferInfo(id: transferID).map(TeamTalkFileTransfer.init)
}

internal func fileTransfer(id transferID: TeamTalkTransferID) -> TeamTalkFileTransfer? {
    fileTransfer(id: transferID.cValue)
}

@discardableResult
internal func cancelFileTransfer(id transferID: Int32) -> Bool {
    guard let instance else {
        return false
    }

    return TT_CancelFileTransfer(instance, transferID) != 0
}

@discardableResult
internal func cancelFileTransfer(id transferID: TeamTalkTransferID) -> Bool {
    cancelFileTransfer(id: transferID.cValue)
}

}
