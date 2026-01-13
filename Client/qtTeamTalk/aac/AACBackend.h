#ifndef AACBACKEND_H
#define AACBACKEND_H

#include <QObject>
#include <QList>

#include "channelinfo.h"

// Forward-declare the TeamTalk instance type from the C API.
// Replace this with the actual type used in your codebase if different.
struct TTInstance;

class AACBackend : public QObject
{
    Q_OBJECT

public:
    explicit AACBackend(TTInstance* ttInstance, QObject* parent = nullptr);

public slots:
    // Called by AACMainWindow (e.g. when "Refresh channels" is activated)
    void refreshChannels();

signals:
    // Emitted after a refresh, with a snapshot of all channels
    void channelsEnumerated(const QList<ChannelInfo>& channels);

private:
    TTInstance* m_ttInstance;

    // Optional: split enumeration into a dedicated helper
    QList<ChannelInfo> enumerateChannels() const;
};

#endif // AACBACKEND_H
