import Foundation
import TeamTalkC

/// One frame of a desktop share: a bitmap plus the session/protocol
/// metadata needed to decode it. Owns its pixel data as `Data`, unlike the
/// raw `DesktopWindow` C struct it wraps, which points at SDK-owned memory
/// only valid until the acquire/release call that produced it returns.
public struct TeamTalkDesktopWindow {
    public let width: Int32
    public let height: Int32
    public let bitmapFormat: TeamTalkBitmapFormat
    public let bytesPerLine: Int32
    public let sessionID: Int32
    public let desktopProtocol: TeamTalkDesktopProtocol
    public let frameBuffer: Data

    public init(
        width: Int32,
        height: Int32,
        bitmapFormat: TeamTalkBitmapFormat,
        bytesPerLine: Int32 = 0,
        sessionID: Int32 = 0,
        desktopProtocol: TeamTalkDesktopProtocol = .zlib,
        frameBuffer: Data
    ) {
        self.width = width
        self.height = height
        self.bitmapFormat = bitmapFormat
        self.bytesPerLine = bytesPerLine
        self.sessionID = sessionID
        self.desktopProtocol = desktopProtocol
        self.frameBuffer = frameBuffer
    }

    public init(_ rawValue: DesktopWindow) {
        self.init(
            width: rawValue.width,
            height: rawValue.height,
            bitmapFormat: rawValue.bitmapFormat,
            bytesPerLine: rawValue.bytesPerLine,
            sessionID: rawValue.sessionID,
            desktopProtocol: rawValue.desktopProtocol,
            frameBuffer: rawValue.frameBufferData
        )
    }

    public var frameBufferSize: Int32 {
        Int32(frameBuffer.count)
    }

    public var sessionIdentifier: TeamTalkDesktopSessionID {
        TeamTalkDesktopSessionID(sessionID)
    }

    public var hasFrameBuffer: Bool {
        !frameBuffer.isEmpty
    }

    public func withUnsafeCValue<Result>(_ body: (DesktopWindow) -> Result) -> Result {
        frameBuffer.withUnsafeBytes { buffer in
            var rawValue = DesktopWindow()
            rawValue.nWidth = width
            rawValue.nHeight = height
            rawValue.bmpFormat = bitmapFormat.cValue
            rawValue.nBytesPerLine = bytesPerLine
            rawValue.nSessionID = sessionID
            rawValue.nProtocol = desktopProtocol.cValue
            rawValue.frameBuffer = buffer.isEmpty ? nil : UnsafeMutableRawPointer(mutating: buffer.baseAddress)
            rawValue.nFrameBufferSize = frameBufferSize
            return body(rawValue)
        }
    }
}

/// One synthetic keyboard/mouse event to forward via
/// `TeamTalkSession.sendDesktopInput(_:to:)` when remote-controlling another
/// user's shared desktop.
public struct TeamTalkDesktopInput {
    /// The maximum number of inputs `sendDesktopInput(_:to:)` accepts in one call.
    public static let maximumCount = Int(TT_DESKTOPINPUT_MAX)

    public let rawValue: DesktopInput

    public init(_ rawValue: DesktopInput) {
        self.rawValue = rawValue
    }

    public init(
        mousePositionX: UInt16? = nil,
        mousePositionY: UInt16? = nil,
        keyCode: TeamTalkDesktopKeyCode = .ignore,
        keyState: TeamTalkDesktopKeyState = .none
    ) {
        var input = DesktopInput()
        input.mousePositionX = mousePositionX
        input.mousePositionY = mousePositionY
        input.keyCode = keyCode
        input.keyState = keyState
        self.rawValue = input
    }

    public var cValue: DesktopInput {
        rawValue
    }

    public var mousePositionX: UInt16? {
        rawValue.mousePositionX
    }

    public var mousePositionY: UInt16? {
        rawValue.mousePositionY
    }

    public var mousePosition: (x: UInt16, y: UInt16)? {
        rawValue.mousePosition
    }

    public var keyCode: TeamTalkDesktopKeyCode {
        rawValue.keyCode
    }

    public var keyState: TeamTalkDesktopKeyState {
        rawValue.keyState
    }

    public var hasMousePosition: Bool {
        rawValue.hasMousePosition
    }

    public var ignoresKeyCode: Bool {
        rawValue.ignoresKeyCode
    }
}

/// One webcam/capture device visible to the OS, as reported by
/// `TeamTalkSession.videoCaptureDevices()`.
public struct TeamTalkVideoCaptureDevice: Identifiable {
    public let rawValue: VideoCaptureDevice

    public init(_ rawValue: VideoCaptureDevice) {
        self.rawValue = rawValue
    }

    public var cValue: VideoCaptureDevice {
        rawValue
    }

    public var id: String {
        deviceIdentifier
    }

    public var deviceIdentifier: String {
        rawValue.deviceIdentifier
    }

    public var name: String {
        rawValue.name
    }

    public var captureAPI: String {
        rawValue.captureAPI
    }

    public var videoFormatsCount: Int32 {
        rawValue.videoFormatsCount
    }

    public var videoFormats: [TeamTalkVideoFormat] {
        rawValue.supportedFormats.map(TeamTalkVideoFormat.init)
    }

    public var defaultVideoFormat: TeamTalkVideoFormat? {
        videoFormats.first
    }

    public var hasVideoFormats: Bool {
        rawValue.hasFormats
    }
}
