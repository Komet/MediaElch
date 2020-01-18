#pragma once

#include <QNetworkReply>
#include <QObject>
#include <QTimer>

/**
 * @brief The NetworkReplyWatcher class takes a QNetworkReply* and watches it.
 * A timeout is set which aborts the download if no response was received after N seconds.
 *
 * @example
 *   new NetworkReplyWatcher(this, reply) // will delete itself when the reply is deleted
 */
class NetworkReplyWatcher : public QObject
{
    Q_OBJECT
public:
    explicit NetworkReplyWatcher(QObject* parent = nullptr, QNetworkReply* reply = nullptr);
    ~NetworkReplyWatcher() override = default;
    void setReply(QNetworkReply* reply);

public slots:
    void onTimeout();
    void onProgress();

private:
    QNetworkReply* m_reply;
    QTimer m_timer;

    const int m_timeoutMilliseconds = 6000;
};
