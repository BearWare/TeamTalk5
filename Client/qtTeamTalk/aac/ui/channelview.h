#pragma once
#include <QWidget>

class QListView;
class QPushButton;
class QLabel;
class QAbstractItemModel;

class ChannelView : public QWidget {
    Q_OBJECT
public:
    explicit ChannelView(QWidget *parent = nullptr);

    void setChannelModel(QAbstractItemModel *model);
    void setServerLabel(const QString &label);

signals:
    void backToServersRequested();
    void joinChannelRequested(const QModelIndex &index);
    void refreshRequested();

private:
    QLabel *m_titleLabel;
    QListView *m_channelList;
    QPushButton *m_joinButton;
    QPushButton *m_backButton;
    QPushButton *m_refreshButton;

    void setupUi();
    void setupConnections();
};
