import Foundation
import TeamTalkC

func teamTalkString<T>(from storage: T) -> String {
    withUnsafeBytes(of: storage) { rawBuffer in
        let buffer = rawBuffer.bindMemory(to: CChar.self)
        guard let baseAddress = buffer.baseAddress else {
            return ""
        }
        return String(cString: baseAddress)
    }
}

func teamTalkSampleRates<T>(from storage: T) -> [Int32] {
    withUnsafeBytes(of: storage) { rawBuffer in
        let buffer = rawBuffer.bindMemory(to: Int32.self)
        return buffer.prefix { $0 != 0 }.map { $0 }
    }
}

func teamTalkArray<T, Element>(
    from storage: T,
    count: Int32,
    as _: Element.Type = Element.self
) -> [Element] {
    withUnsafeBytes(of: storage) { rawBuffer in
        let buffer = rawBuffer.bindMemory(to: Element.self)
        let clampedCount = max(0, min(Int(count), buffer.count))
        return Array(buffer.prefix(clampedCount))
    }
}

func teamTalkPCM16Samples(from rawAudio: UnsafeRawPointer?, sampleCount: Int) -> [Int16] {
    guard let rawAudio, sampleCount > 0 else {
        return []
    }

    let buffer = rawAudio.bindMemory(to: Int16.self, capacity: sampleCount)
    return Array(UnsafeBufferPointer(start: buffer, count: sampleCount))
}

// Bridges the raw `SoundDevice` C struct's fixed-size buffers (device name,
// sample-rate list, ...) to Swift-friendly String/[Int32]/Bool accessors.
// ``TeamTalkSoundDevice`` wraps this extension rather than duplicating it.
public extension SoundDevice {
    var id: Int32 {
        nDeviceID
    }

    var soundDeviceID: TeamTalkSoundDeviceID {
        TeamTalkSoundDeviceID(id)
    }

    var name: String {
        teamTalkString(from: szDeviceName)
    }

    var deviceIdentifier: String {
        teamTalkString(from: szDeviceID)
    }

    var soundSystem: TeamTalkSoundSystem {
        get { TeamTalkSoundSystem(cValue: nSoundSystem) }
        set { nSoundSystem = newValue.cValue }
    }

    var waveDeviceID: Int32 {
        nWaveDeviceID
    }

    var maxInputChannels: Int32 {
        nMaxInputChannels
    }

    var maxOutputChannels: Int32 {
        nMaxOutputChannels
    }

    var supportedInputSampleRates: [Int32] {
        teamTalkSampleRates(from: inputSampleRates)
    }

    var supportedOutputSampleRates: [Int32] {
        teamTalkSampleRates(from: outputSampleRates)
    }

    var defaultSampleRate: Int32 {
        nDefaultSampleRate
    }

    var features: TeamTalkSoundDeviceFeatures {
        get { TeamTalkSoundDeviceFeatures(cValue: uSoundDeviceFeatures) }
        set { uSoundDeviceFeatures = newValue.cValue }
    }

    var isShared: Bool {
        soundDeviceID.isShared
    }

    var physicalDeviceID: TeamTalkSoundDeviceID {
        soundDeviceID.physicalDeviceID
    }

    var supportsEchoCancellation: Bool {
        features.contains(.echoCancellation)
    }

    var supportsAutomaticGainControl: Bool {
        features.contains(.automaticGainControl)
    }

    var supportsDenoise: Bool {
        features.contains(.denoise)
    }

    var supportsDuplexMode: Bool {
        features.contains(.duplexMode)
    }

    var isDefaultCommunicationDevice: Bool {
        features.contains(.defaultCommunicationDevice)
    }

    func hasFeature(_ feature: TeamTalkSoundDeviceFeatures) -> Bool {
        features.contains(feature)
    }
}

/// Bridges the raw `SoundDeviceEffects` C struct's `bEnable*` int flags to
/// Bool accessors. ``TeamTalkSoundDeviceEffects`` wraps this extension
/// rather than duplicating it.
public extension SoundDeviceEffects {
    var automaticGainControlEnabled: Bool {
        get { bEnableAGC != 0 }
        set { bEnableAGC = newValue ? 1 : 0 }
    }

    var denoiseEnabled: Bool {
        get { bEnableDenoise != 0 }
        set { bEnableDenoise = newValue ? 1 : 0 }
    }

    var echoCancellationEnabled: Bool {
        get { bEnableEchoCancellation != 0 }
        set { bEnableEchoCancellation = newValue ? 1 : 0 }
    }

    var enabledEffects: TeamTalkSoundDeviceFeatures {
        get {
            var features: TeamTalkSoundDeviceFeatures = []
            if automaticGainControlEnabled {
                features.insert(.automaticGainControl)
            }
            if denoiseEnabled {
                features.insert(.denoise)
            }
            if echoCancellationEnabled {
                features.insert(.echoCancellation)
            }
            return features
        }
        set {
            automaticGainControlEnabled = newValue.contains(.automaticGainControl)
            denoiseEnabled = newValue.contains(.denoise)
            echoCancellationEnabled = newValue.contains(.echoCancellation)
        }
    }

}
