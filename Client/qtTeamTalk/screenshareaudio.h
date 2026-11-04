// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHAREAUDIO_H
#define SCREENSHAREAUDIO_H

#include <memory>
#include <QString>
#include <QList>
#include <QPair>

typedef void TTInstance;

class ScreenShareAudio
{
public:
    ScreenShareAudio();
    ~ScreenShareAudio();

    bool start(TTInstance* instance,
               unsigned long sharedProcessId = 0,
               bool includeSharedProcess = false,
               bool useReaStream = false,
               const QString& reaStreamIdentifier = QString());
    static unsigned long processIdForWindow(void* window);
    void stop();
    bool isRunning() const;
    void setVolume(int volume);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

#endif
