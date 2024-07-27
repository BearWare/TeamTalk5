#include "utilmedia.h"

#include <QObject>

QString durationToString(quint32 duration_msec, bool include_msec/* = true*/)
{
    quint32 hours = duration_msec / (60 * 60 * 1000);
    quint32 remain = duration_msec - (60 * 60 * 1000 * hours);
    quint32 minutes = remain / (60 * 1000);
    remain -= 60 * 1000 * minutes;
    quint32 seconds = remain / 1000;
    quint32 msec = remain % 1000;

    return include_msec ? QString("%1:%2:%3.%4").arg(hours)
                              .arg(minutes, 2, 10, QChar('0'))
                              .arg(seconds, 2, 10, QChar('0'))
                              .arg(msec, 3, 10, QChar('0')) :
               QString("%1:%2:%3").arg(hours)
                   .arg(minutes, 2, 10, QChar('0'))
                   .arg(seconds, 2, 10, QChar('0'));
}

QString getMediaAudioDescription(const AudioFormat& audioFmt)
{
    // display audio format
    if (audioFmt.nAudioFmt != AFF_NONE)
    {
        QString channels;
        if (audioFmt.nChannels == 2)
            channels = QObject::tr("Stereo");
        else if (audioFmt.nChannels == 1)
            channels = QObject::tr("Mono");
        else
            channels = QObject::tr("%1 audio channels").arg(audioFmt.nChannels);

        return QObject::tr("%1 Hz, %2").arg(audioFmt.nSampleRate).arg(channels);
    }
    else
        return QObject::tr("Unknown format");
}

QString getMediaVideoDescription(const VideoFormat& videoFmt)
{
    // display video format
    double fps = double(videoFmt.nFPS_Numerator) / double(videoFmt.nFPS_Denominator);
    if (videoFmt.picFourCC != FOURCC_NONE)
        return QObject::tr("%1x%2 %3 FPS").arg(videoFmt.nWidth).arg(videoFmt.nHeight).arg(fps, 0, 'f', 1);
    else
        return QObject::tr("Unknown format");
}
