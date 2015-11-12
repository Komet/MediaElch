#ifndef NETWORKREPLYWATCHER_H
#define NETWORKREPLYWATCHER_H

#include <QNetworkReply>
#include <QObject>
#include <QTimer>

class NetworkReplyWatcher : public QObject
{
    Q_OBJECT
public:
    explicit NetworkReplyWatcher(QObject *parent = 0, QNetworkReply *reply = 0);
    ~NetworkReplyWatcher();
    void setReply(QNetworkReply *reply);

public slots:
    void onTimeout();
    void onProgress();

private:
    QNetworkReply *m_reply;
    QTimer m_timer;
};

#endif // NETWORKREPLYWATCHER_H
