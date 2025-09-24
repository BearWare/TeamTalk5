import Foundation
import CallKit
import AVFoundation

// Manages automatic status updates when a system call (cellular / FaceTime / CallKit-integrated VoIP) is active.
final class CallStatusManager: NSObject {
    static let shared = CallStatusManager()

    private let callObserver = CXCallObserver()
    private var isAwayBecauseOfCall = false
    private var previousStatusMode: UINT32 = 0
    private var previousStatusMsg: String = ""
    private var cachedHasCall = false

    private override init() { super.init() }

    func start() {
        callObserver.setDelegate(self, queue: .main)
        NotificationCenter.default.addObserver(self,
                                               selector: #selector(handleAudioInterruption(_:)),
                                               name: AVAudioSession.interruptionNotification,
                                               object: nil)
        evaluateCalls(callObserver.calls) // initial state
    }

    func recheck() {
        evaluateCalls(callObserver.calls)
    }

    @objc private func handleAudioInterruption(_ note: Notification) {
        guard shouldAutoAway() else { return }
        guard let info = note.userInfo,
              let raw = info[AVAudioSessionInterruptionTypeKey] as? UInt,
              let type = AVAudioSession.InterruptionType(rawValue: raw) else { return }
        switch type {
        case .began:
            // If CallKit didn't yet report, still mark away optimistically
            setAwayIfNeeded(reason: "Audio interruption")
        case .ended:
            // Re-evaluate via CallKit; may restore
            evaluateCalls(callObserver.calls)
        @unknown default:
            break
        }
    }

    private func shouldAutoAway() -> Bool {
        let defaults = UserDefaults.standard
        if defaults.object(forKey: PREF_GENERAL_AUTO_CALL_AWAY) == nil {
            return DEFAULT_AUTO_CALL_AWAY
        }
        return defaults.bool(forKey: PREF_GENERAL_AUTO_CALL_AWAY)
    }

    private func setAwayIfNeeded(reason: String) {
        guard shouldAutoAway(), !isAwayBecauseOfCall else { return }
        // Capture current mode if available. If API exposes retrieval, use it. Otherwise assume AVAILABLE (0).
        previousStatusMode = 0
        previousStatusMsg = ""
        if TT_GetFlags(ttInst) & CLIENT_CONNECTED.rawValue != 0 {
            TT_DoChangeStatus(ttInst, INT32(StatusMode.STATUSMODE_AWAY.rawValue), NSLocalizedString("On a call", comment: "status"))
            isAwayBecauseOfCall = true
        }
    }

    private func restoreIfNeeded() {
        guard isAwayBecauseOfCall else { return }
        if TT_GetFlags(ttInst) & CLIENT_CONNECTED.rawValue != 0 {
            TT_DoChangeStatus(ttInst, INT32(previousStatusMode), previousStatusMsg)
        }
        isAwayBecauseOfCall = false
    }

    private func evaluateCalls(_ calls: [CXCall]) {
        guard shouldAutoAway() else { return }
        let active = calls.contains { !$0.hasEnded }
        if active {
            cachedHasCall = true
            setAwayIfNeeded(reason: "Call active")
        } else {
            cachedHasCall = false
            // Slight delay in case a consecutive call begins immediately
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.4) { [weak self] in
                guard let self else { return }
                if !self.callObserver.calls.contains(where: { !$0.hasEnded }) { self.restoreIfNeeded() }
            }
        }
    }
}

extension CallStatusManager: CXCallObserverDelegate {
    func callObserver(_ callObserver: CXCallObserver, callChanged call: CXCall) {
        evaluateCalls(callObserver.calls)
    }
}
