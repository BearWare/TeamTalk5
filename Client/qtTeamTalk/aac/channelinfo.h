#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include <QString>

struct ChannelInfo {
    int id = 0;
    QString name;
    int userCount = 0;
    bool isPasswordProtected = false;
    QString topic;
};

#endif // CHANNELINFO_H
