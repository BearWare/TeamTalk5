//
//  SwiftUIRows.swift
//  iTeamTalk
//
//  Created by Codex on 16/04/2026.
//  Copyright © 2026 BearWare.dk. All rights reserved.
//

import SwiftUI

struct TeamTalkToggleRow: View {
    let title: String
    let subtitle: String?
    @Binding var isOn: Bool

    init(title: String, subtitle: String? = nil, isOn: Binding<Bool>) {
        self.title = title
        self.subtitle = subtitle
        self._isOn = isOn
    }

    var body: some View {
        Toggle(isOn: $isOn) {
            VStack(alignment: .leading, spacing: 2) {
                Text(title)
                    .font(.body)
                if let subtitle {
                    Text(subtitle)
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
        }
        .accessibilityLabel(title)
    }
}

struct TeamTalkTextFieldRow: View {
    let title: String
    let placeholder: String
    let isSecure: Bool
    let isEnabled: Bool
    @Binding var text: String

    init(title: String,
         placeholder: String = NSLocalizedString("Type text here", comment: "text field hint"),
         text: Binding<String>,
         isSecure: Bool = false,
         isEnabled: Bool = true) {
        self.title = title
        self.placeholder = placeholder
        self._text = text
        self.isSecure = isSecure
        self.isEnabled = isEnabled
    }

    var body: some View {
        HStack(spacing: 12) {
            Text(title)
                .font(.body)
            Spacer(minLength: 16)
            Group {
                if isSecure {
                    SecureField(placeholder, text: $text)
                } else {
                    TextField(placeholder, text: $text)
                }
            }
            .multilineTextAlignment(.trailing)
            .textInputAutocapitalization(.never)
            .autocorrectionDisabled(true)
            .disabled(!isEnabled)
        }
        .accessibilityElement(children: .combine)
    }
}

struct TeamTalkValueRow: View {
    let title: String
    let subtitle: String?
    let value: String?

    init(title: String, subtitle: String? = nil, value: String? = nil) {
        self.title = title
        self.subtitle = subtitle
        self.value = value
    }

    var body: some View {
        HStack(spacing: 12) {
            VStack(alignment: .leading, spacing: 2) {
                Text(title)
                    .font(.body)
                if let subtitle {
                    Text(subtitle)
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer(minLength: 16)
            if let value {
                Text(value)
                    .font(.body)
                    .foregroundStyle(.secondary)
                    .multilineTextAlignment(.trailing)
            }
        }
    }
}

struct TeamTalkSliderRow: View {
    let title: String
    let range: ClosedRange<Double>
    let step: Double?
    let unit: String?
    let displayValue: ((Double) -> String)?
    @Binding var value: Double

    init(title: String,
         value: Binding<Double>,
         range: ClosedRange<Double>,
         step: Double? = nil,
         unit: String? = nil,
         displayValue: ((Double) -> String)? = nil) {
        self.title = title
        self._value = value
        self.range = range
        self.step = step
        self.unit = unit
        self.displayValue = displayValue
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack(spacing: 12) {
                Text(title)
                    .font(.body)
                Spacer(minLength: 16)
                Text(valueText)
                    .font(.body.monospacedDigit())
                    .foregroundStyle(.secondary)
            }
            if let step {
                Slider(value: $value, in: range, step: step)
            } else {
                Slider(value: $value, in: range)
            }
        }
        .accessibilityElement(children: .combine)
    }
    
    private var valueText: String {
        if let displayValue {
            return displayValue(value)
        }
        let number = "\(Int(value.rounded()))"
        if let unit {
            return "\(number) \(unit)"
        }
        return number
    }
}

struct TeamTalkStepperRow: View {
    let title: String
    let unit: String?
    let range: ClosedRange<Double>
    let step: Double
    @Binding var value: Double

    init(title: String,
         value: Binding<Double>,
         range: ClosedRange<Double>,
         step: Double,
         unit: String? = nil) {
        self.title = title
        self._value = value
        self.range = range
        self.step = step
        self.unit = unit
    }

    var body: some View {
        Stepper(value: $value, in: range, step: step) {
            HStack(spacing: 12) {
                Text(title)
                    .font(.body)
                Spacer(minLength: 16)
                Text(valueText)
                    .font(.body.monospacedDigit())
                    .foregroundStyle(.secondary)
            }
        }
    }

    private var valueText: String {
        let number = "\(Int(value.rounded()))"
        if let unit {
            return "\(number) \(unit)"
        }
        return number
    }
}

struct TeamTalkSegmentedRow: View {
    let title: String
    let values: [String]
    @Binding var selectedIndex: Int

    init(title: String, values: [String], selectedIndex: Binding<Int>) {
        self.title = title
        self.values = values
        self._selectedIndex = selectedIndex
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(title)
                .font(.body)
            Picker(title, selection: $selectedIndex) {
                ForEach(values.indices, id: \.self) { index in
                    Text(values[index]).tag(index)
                }
            }
            .pickerStyle(.segmented)
        }
    }
}

struct TeamTalkActionRow: View {
    let title: String
    let role: ButtonRole?
    let action: () -> Void

    init(title: String, role: ButtonRole? = nil, action: @escaping () -> Void) {
        self.title = title
        self.role = role
        self.action = action
    }

    var body: some View {
        Button(role: role, action: action) {
            Text(title)
                .frame(maxWidth: .infinity, alignment: .center)
        }
    }
}

struct TeamTalkServerRow: View {
    let title: String
    let subtitle: String
    let iconName: String
    let iconAccessibilityLabel: String
    let actionTitle: String
    let action: () -> Void

    var body: some View {
        HStack(spacing: 10) {
            Image(iconName)
                .resizable()
                .frame(width: 36, height: 36)
                .accessibilityLabel(iconAccessibilityLabel)

            VStack(alignment: .leading, spacing: 2) {
                Text(title)
                    .font(.body)
                    .lineLimit(1)
                Text(subtitle)
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                    .lineLimit(2)
            }

            Spacer(minLength: 12)

            Button(actionTitle, action: action)
                .buttonStyle(.bordered)
        }
        .accessibilityElement(children: .contain)
    }
}

struct TeamTalkIconActionRow: View {
    let title: String
    let subtitle: String?
    let iconName: String
    let iconAccessibilityLabel: String
    let actionTitle: String?
    let actionImageName: String?
    let isDimmed: Bool
    let action: (() -> Void)?

    init(title: String,
         subtitle: String? = nil,
         iconName: String,
         iconAccessibilityLabel: String,
         actionTitle: String? = nil,
         actionImageName: String? = nil,
         isDimmed: Bool = false,
         action: (() -> Void)? = nil) {
        self.title = title
        self.subtitle = subtitle
        self.iconName = iconName
        self.iconAccessibilityLabel = iconAccessibilityLabel
        self.actionTitle = actionTitle
        self.actionImageName = actionImageName
        self.isDimmed = isDimmed
        self.action = action
    }

    var body: some View {
        HStack(spacing: 10) {
            Image(iconName)
                .resizable()
                .frame(width: 36, height: 36)
                .accessibilityLabel(iconAccessibilityLabel)

            VStack(alignment: .leading, spacing: 2) {
                Text(title)
                    .font(.body)
                    .foregroundStyle(isDimmed ? .secondary : .primary)
                    .lineLimit(1)
                if let subtitle {
                    Text(subtitle)
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                        .lineLimit(2)
                }
            }

            Spacer(minLength: 12)

            if let action {
                Button(action: action) {
                    if let actionImageName {
                        Image(actionImageName)
                            .resizable()
                            .frame(width: 24, height: 24)
                    } else if let actionTitle {
                        Text(actionTitle)
                    }
                }
                .buttonStyle(.borderless)
                .accessibilityLabel(actionTitle ?? title)
            }
        }
        .accessibilityElement(children: .contain)
    }
}

struct TeamTalkMessageRow: View {
    let message: MyTextMessage

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(headerText)
                .font(.footnote.weight(.semibold))
                .foregroundStyle(.secondary)
            Text(message.message)
                .font(.body)
                .textSelection(.enabled)
                .fixedSize(horizontal: false, vertical: true)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .padding(.vertical, 6)
        .background(message.backgroundColor)
        .accessibilityElement(children: .ignore)
        .accessibilityLabel(message.message)
        .accessibilityHint(accessibilityHint)
    }
}

extension MyTextMessage {
    var backgroundColor: Color {
        switch msgtype {
        case .PRIV_IM, .CHAN_IM:
            return Color(red: 1.0, green: 0.627, blue: 0.882)
        case .PRIV_IM_MYSELF, .CHAN_IM_MYSELF:
            return Color(red: 0.54, green: 0.82, blue: 0.94)
        case .BCAST:
            return Color(red: 0.831, green: 0.376, blue: 1.0)
        case .LOGMSG:
            return Color(red: 0.86, green: 0.86, blue: 0.86)
        }
    }
}

private extension TeamTalkMessageRow {
    private var headerText: String {
        switch message.msgtype {
        case .PRIV_IM, .PRIV_IM_MYSELF, .CHAN_IM, .CHAN_IM_MYSELF, .BCAST:
            return "\(limitText(message.nickname)), \(timeText)"
        case .LOGMSG:
            return timeText
        }
    }

    private var accessibilityHint: String {
        "\(headerText). \(messageTypeText)"
    }

    private var messageTypeText: String {
        switch message.msgtype {
        case .PRIV_IM, .PRIV_IM_MYSELF:
            return NSLocalizedString("Private message", comment: "text message type")
        case .CHAN_IM, .CHAN_IM_MYSELF:
            return NSLocalizedString("Channel message", comment: "text message type")
        case .BCAST:
            return NSLocalizedString("Broadcast message", comment: "text message type")
        case .LOGMSG:
            return NSLocalizedString("Log message", comment: "text message type")
        }
    }

    private var timeText: String {
        let formatter = DateFormatter()
        formatter.locale = Locale.current
        formatter.dateFormat = "HH:mm:ss"
        return formatter.string(from: message.date)
    }
}
