import Foundation
import TeamTalkC

extension TeamTalkSession {
/// Transmits a captured screen/window image as this client's desktop share,
/// starting or updating the desktop stream everyone else in the channel sees.
public func sendDesktopWindow(
    _ desktopWindow: DesktopWindow,
    convertTo bitmapFormat: TeamTalkBitmapFormat = .none
) -> Int32 {
    guard let instance else {
        return -1
    }

    var desktopWindow = desktopWindow
    return TT_SendDesktopWindow(instance, &desktopWindow, bitmapFormat.cValue)
}

@discardableResult
public func sendDesktopWindow(
    _ desktopWindow: TeamTalkDesktopWindow,
    convertTo bitmapFormat: TeamTalkBitmapFormat = .none
) -> Int32 {
    guard let instance else {
        return -1
    }

    return desktopWindow.withUnsafeCValue { rawValue in
        var rawValue = rawValue
        return TT_SendDesktopWindow(instance, &rawValue, bitmapFormat.cValue)
    }
}

@discardableResult
public func closeDesktopWindow() -> Bool {
    guard let instance else {
        return false
    }

    return TT_CloseDesktopWindow(instance) != 0
}

/// Updates the cursor position overlaid on this client's desktop share.
@discardableResult
public func sendDesktopCursorPosition(x: UInt16, y: UInt16) -> Bool {
    guard let instance else {
        return false
    }

    return TT_SendDesktopCursorPosition(instance, x, y) != 0
}

@discardableResult
internal func sendDesktopInput(userID: Int32, inputs: [DesktopInput]) -> Bool {
    guard let instance, !inputs.isEmpty, inputs.count <= Int(TT_DESKTOPINPUT_MAX) else {
        return false
    }

    return inputs.withUnsafeBufferPointer { buffer in
        guard let baseAddress = buffer.baseAddress else {
            return false
        }
        return TT_SendDesktopInput(instance, userID, baseAddress, Int32(buffer.count)) != 0
    }
}

/// Remote-controls `user`'s shared desktop by forwarding synthetic
/// keyboard/mouse input, if that user has granted desktop-input access.
@discardableResult
public func sendDesktopInput(_ inputs: [TeamTalkDesktopInput], to user: TeamTalkUser) -> Bool {
    sendDesktopInput(userID: user.userID.cValue, inputs: inputs.map(\.cValue))
}

internal func withAcquiredDesktopWindow<Result>(
    userID: Int32,
    _ body: (DesktopWindow) throws -> Result
) rethrows -> Result? {
    guard let instance, let desktopWindow = TT_AcquireUserDesktopWindow(instance, userID) else {
        return nil
    }

    defer {
        _ = TT_ReleaseUserDesktopWindow(instance, desktopWindow)
    }
    return try body(desktopWindow.pointee)
}

public func withAcquiredDesktopWindow<Result>(
    for user: TeamTalkUser,
    _ body: (DesktopWindow) throws -> Result
) rethrows -> Result? {
    try withAcquiredDesktopWindow(userID: user.userID.cValue, body)
}

internal func withAcquiredDesktopWindow<Result>(
    userID: Int32,
    convertTo bitmapFormat: TeamTalkBitmapFormat,
    _ body: (DesktopWindow) throws -> Result
) rethrows -> Result? {
    guard let instance, let desktopWindow = TT_AcquireUserDesktopWindowEx(instance, userID, bitmapFormat.cValue) else {
        return nil
    }

    defer {
        _ = TT_ReleaseUserDesktopWindow(instance, desktopWindow)
    }
    return try body(desktopWindow.pointee)
}

public func withAcquiredDesktopWindow<Result>(
    for user: TeamTalkUser,
    convertTo bitmapFormat: TeamTalkBitmapFormat,
    _ body: (DesktopWindow) throws -> Result
) rethrows -> Result? {
    try withAcquiredDesktopWindow(userID: user.userID.cValue, convertTo: bitmapFormat, body)
}

/// The latest desktop-share frame `user` has sent, copied into a safe,
/// owned value.
public func acquireDesktopWindow(for user: TeamTalkUser) -> TeamTalkDesktopWindow? {
    withAcquiredDesktopWindow(userID: user.userID.cValue) { TeamTalkDesktopWindow($0) }
}

public func acquireDesktopWindow(
    for user: TeamTalkUser,
    convertTo bitmapFormat: TeamTalkBitmapFormat
) -> TeamTalkDesktopWindow? {
    withAcquiredDesktopWindow(userID: user.userID.cValue, convertTo: bitmapFormat) { TeamTalkDesktopWindow($0) }
}

/// All webcams/capture devices currently visible to the OS, in raw SDK form.
/// Prefer ``videoCaptureDevices()``.
public func videoCaptureDevicesInfo() -> [VideoCaptureDevice] {
    var count: Int32 = 0
    guard TT_GetVideoCaptureDevices(nil, &count) != 0, count > 0 else {
        return []
    }

    var devices = Array(repeating: VideoCaptureDevice(), count: Int(count))
    var deviceCount = count
    let didRead = devices.withUnsafeMutableBufferPointer { buffer in
        TT_GetVideoCaptureDevices(buffer.baseAddress, &deviceCount) != 0
    }
    guard didRead else {
        return []
    }

    let returnedCount = max(0, min(Int(deviceCount), devices.count))
    if returnedCount < devices.count {
        devices.removeLast(devices.count - returnedCount)
    }
    return devices
}

/// All webcams/capture devices currently visible to the OS.
public func videoCaptureDevices() -> [TeamTalkVideoCaptureDevice] {
    videoCaptureDevicesInfo().map(TeamTalkVideoCaptureDevice.init)
}

@discardableResult
internal func initVideoCaptureDevice(deviceID: String, format: VideoFormat) -> Bool {
    guard let instance else {
        return false
    }

    var format = format
    return TT_InitVideoCaptureDevice(instance, deviceID, &format) != 0
}

@discardableResult
public func initVideoCaptureDevice(_ device: TeamTalkVideoCaptureDevice, format: TeamTalkVideoFormat) -> Bool {
    initVideoCaptureDevice(deviceID: device.deviceIdentifier, format: format.cValue)
}

@discardableResult
public func closeVideoCaptureDevice() -> Bool {
    guard let instance else {
        return false
    }

    return TT_CloseVideoCaptureDevice(instance) != 0
}

}
