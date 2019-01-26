#pragma once

#include <QNetworkReply>
#include <QObject>
#include <QTimer>

class NetworkReplyWatcher : public QObject
{
    Q_OBJECT
public:
    explicit NetworkReplyWatcher(QObject *parent = nullptr, QNetworkReply *reply = nullptr);
    ~NetworkReplyWatcher() override = default;
    void setReply(QNetworkReply *reply);

public slots:
    void onTimeout();
    void onProgress();

private:
    QNetworkReply *m_reply;
    QTimer m_timer;
};
