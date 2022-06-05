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

#include "utiltts.h"
#include <QAbstractItemModel>
#include <QVector>
#include <QTreeView>

class TTSEventsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    TTSEventsModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
    QModelIndex parent ( const QModelIndex & index ) const override;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;

    void setTTSEvents(TTSEvents ttsactive);
    TTSEvents getTTSEvents();
private:
    QVector<TextToSpeechEvent> m_ttsevents;
    TTSEvents m_ttsselected = TTS_NONE;
};

#endif // TTSEVENTSMODEL_H
