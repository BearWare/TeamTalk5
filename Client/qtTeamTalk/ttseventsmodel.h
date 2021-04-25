/*
 * Copyright (c) 2005-2016, BearWare.dk
 *
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product
 * documentation is required.
 *
 */

#ifndef TTSEVENTSMODEL_H
#define TTSEVENTSMODEL_H

#include "common.h"
#include <QAbstractItemModel>
#include <QVector>
#include <QTreeView>

class TTSEventsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    TTSEventsModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    void setTTSEvents(TTSEvents ttsactive);
    TTSEvents getTTSEvents();
private:
    QVector<TextToSpeechEvent> m_ttsevents;
    TTSEvents m_ttsselected = TTS_NONE;
};

class TTSEventsTreeView : public QTreeView
{
    Q_OBJECT

public:
    TTSEventsTreeView(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* e) override;

};

#endif // TTSEVENTSMODEL_H
