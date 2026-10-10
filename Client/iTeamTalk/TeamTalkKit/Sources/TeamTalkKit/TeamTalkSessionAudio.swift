import Foundation
import TeamTalkC

// Throughout this file, `xInfo()`/`xInfo(...)` returns the raw C struct
// straight from the SDK, while the plain-named sibling wraps it in a typed
// Swift model. Prefer the typed sibling; the raw variant exists for callers
// that need to pass the struct straight back into another C-level call.
extension TeamTalkSession {
/// All sound devices currently visible to the OS, in raw SDK form. Prefer
/// ``soundDevices()``.
public func soundDevicesInfo() -> [SoundDevice] {
    var count: Int32 = 0
    guard TT_GetSoundDevices(nil, &count) != 0, count > 0 else {
        return []
    }

    var devices = Array(repeating: SoundDevice(), count: Int(count))
    var deviceCount = count
    let didRead = devices.withUnsafeMutableBufferPointer { buffer in
        TT_GetSoundDevices(buffer.baseAddress, &deviceCount) != 0
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

/// All sound devices currently visible to the OS.
public func soundDevices() -> [TeamTalkSoundDevice] {
    soundDevicesInfo().map(TeamTalkSoundDevice.init)
}

/// The OS-preferred input/output device IDs, or `nil` if the SDK couldn't
/// determine them.
public func defaultSoundDevices() -> (input: TeamTalkSoundDeviceID, output: TeamTalkSoundDeviceID)? {
    var inputDeviceID: Int32 = 0
    var outputDeviceID: Int32 = 0
    guard TT_GetDefaultSoundDevices(&inputDeviceID, &outputDeviceID) != 0 else {
        return nil
    }
    return (TeamTalkSoundDeviceID(inputDeviceID), TeamTalkSoundDeviceID(outputDeviceID))
}

/// Like ``defaultSoundDevices()``, restricted to devices under a specific
/// backend (e.g. Core Audio vs. a specific driver API).
public func defaultSoundDevices(for soundSystem: TeamTalkSoundSystem) -> (input: TeamTalkSoundDeviceID, output: TeamTalkSoundDeviceID)? {
    var inputDeviceID: Int32 = 0
    var outputDeviceID: Int32 = 0
    guard TT_GetDefaultSoundDevicesEx(soundSystem.cValue, &inputDeviceID, &outputDeviceID) != 0 else {
        return nil
    }
    return (TeamTalkSoundDeviceID(inputDeviceID), TeamTalkSoundDeviceID(outputDeviceID))
}

/// Tears down and reinitializes the OS sound backend. Useful after a device
/// hot-plug/unplug the SDK didn't pick up on its own.
@discardableResult
public func restartSoundSystem() -> Bool {
    TT_RestartSoundSystem() != 0
}

/// Raw platform audio effect state (e.g. AGC, echo cancellation) currently
/// applied to the open sound devices. Prefer ``soundDeviceEffects()``.
public func soundDeviceEffectsInfo() -> SoundDeviceEffects? {
    guard let instance else {
        return nil
    }

    var effects = SoundDeviceEffects()
    guard TT_GetSoundDeviceEffects(instance, &effects) != 0 else {
        return nil
    }
    return effects
}

/// Platform audio effect state currently applied to the open sound devices.
public func soundDeviceEffects() -> TeamTalkSoundDeviceEffects? {
    soundDeviceEffectsInfo().map(TeamTalkSoundDeviceEffects.init)
}

@discardableResult
public func setSoundDeviceEffects(_ effects: inout SoundDeviceEffects) -> Bool {
    guard let instance else {
        return false
    }

    return TT_SetSoundDeviceEffects(instance, &effects) != 0
}

@discardableResult
public func setSoundDeviceEffects(_ configuration: TeamTalkSoundDeviceEffectsConfiguration) -> Bool {
    var effects = configuration.cValue
    return setSoundDeviceEffects(&effects)
}

@discardableResult
public func setSoundDeviceEffects(_ effects: TeamTalkSoundDeviceEffects) -> Bool {
    var rawEffects = effects.cValue
    return setSoundDeviceEffects(&rawEffects)
}

/// The raw preprocessor (noise suppression, AGC, echo cancellation, ...)
/// currently applied to the sound input device. Prefer ``soundInputPreprocessor()``.
public func soundInputPreprocessorInfo() -> AudioPreprocessor? {
    guard let instance else {
        return nil
    }

    var preprocessor = AudioPreprocessor()
    guard TT_GetSoundInputPreprocessEx(instance, &preprocessor) != 0 else {
        return nil
    }
    return preprocessor
}

/// The preprocessor (noise suppression, AGC, echo cancellation, ...)
/// currently applied to the sound input device.
public func soundInputPreprocessor() -> TeamTalkAudioPreprocessorConfiguration? {
    soundInputPreprocessorInfo().map(TeamTalkAudioPreprocessorConfiguration.init)
}

@discardableResult
public func setSoundInputPreprocessor(_ preprocessor: inout AudioPreprocessor) -> Bool {
    guard let instance else {
        return false
    }

    return TT_SetSoundInputPreprocessEx(instance, &preprocessor) != 0
}

@discardableResult
public func setSoundInputPreprocessor(_ configuration: TeamTalkAudioPreprocessorConfiguration) -> Bool {
    var preprocessor = configuration.cValue
    return setSoundInputPreprocessor(&preprocessor)
}

@discardableResult
public func setSoundInputPreprocessor(_ type: TeamTalkAudioPreprocessorType) -> Bool {
    setSoundInputPreprocessor(TeamTalkAudioPreprocessorConfiguration(type: type))
}

/// Opens the input device in "shared" mode, where this process cooperates
/// with other processes using the same device instead of claiming exclusive
/// access — needed on platforms where multiple apps must share one mic.
@discardableResult
public func initSoundInputSharedDevice(sampleRate: Int32, channels: Int32, frameSize: Int32) -> Bool {
    TT_InitSoundInputSharedDevice(sampleRate, channels, frameSize) != 0
}

@discardableResult
public func initSoundInputSharedDevice(_ configuration: TeamTalkSharedSoundDeviceConfiguration) -> Bool {
    initSoundInputSharedDevice(
        sampleRate: configuration.sampleRate,
        channels: configuration.channels,
        frameSize: configuration.frameSize
    )
}

/// Opens the output device in "shared" mode. See ``initSoundInputSharedDevice(sampleRate:channels:frameSize:)``.
@discardableResult
public func initSoundOutputSharedDevice(sampleRate: Int32, channels: Int32, frameSize: Int32) -> Bool {
    TT_InitSoundOutputSharedDevice(sampleRate, channels, frameSize) != 0
}

@discardableResult
public func initSoundOutputSharedDevice(_ configuration: TeamTalkSharedSoundDeviceConfiguration) -> Bool {
    initSoundOutputSharedDevice(
        sampleRate: configuration.sampleRate,
        channels: configuration.channels,
        frameSize: configuration.frameSize
    )
}

/// Opens an input and output device together in duplex mode, so the SDK can
/// use their combined clock for echo cancellation between the two.
@discardableResult
public func initSoundDuplexDevices(inputDeviceID: Int32, outputDeviceID: Int32) -> Bool {
    guard let instance else {
        return false
    }

    return TT_InitSoundDuplexDevices(instance, inputDeviceID, outputDeviceID) != 0
}

@discardableResult
public func initSoundDuplexDevices(inputDeviceID: TeamTalkSoundDeviceID, outputDeviceID: TeamTalkSoundDeviceID) -> Bool {
    initSoundDuplexDevices(inputDeviceID: inputDeviceID.cValue, outputDeviceID: outputDeviceID.cValue)
}

@discardableResult
public func initSoundDuplexDevices(_ configuration: TeamTalkSoundDuplexConfiguration) -> Bool {
    initSoundDuplexDevices(
        inputDeviceID: configuration.inputDeviceRawID,
        outputDeviceID: configuration.outputDeviceRawID
    )
}

/// Closes devices opened by `initSoundDuplexDevices`.
@discardableResult
public func closeSoundDuplexDevices() -> Bool {
    guard let instance else {
        return false
    }

    return TT_CloseSoundDuplexDevices(instance) != 0
}

}
