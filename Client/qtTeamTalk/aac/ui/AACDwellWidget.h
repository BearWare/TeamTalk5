#pragma once

#include <QWidget>
#include <QPointer>

class AACAccessibilityManager;

class AACDwellWidget : public QWidget {
    Q_OBJECT
public:
    explicit AACDwellWidget(AACAccessibilityManager* aac, QWidget* parent = nullptr);

    void setDeepWell(bool enabled);
    bool isDeepWell() const;

public slots:
    void setDwellProgress(float p); // 0.0–1.0

protected:
    void enterEvent(QEnterEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;

    // For subclasses to draw dwell visuals
    float dwellProgress() const { return m_dwellProgress; }
    AACAccessibilityManager* aac() const { return m_aac; }

private:
    AACAccessibilityManager* m_aac = nullptr;
    bool m_deepWell = false;
    float m_dwellProgress = 0.0f;
};
