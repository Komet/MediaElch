#pragma once

#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <chrono>

/// \brief The NetworkReplyWatcher class takes a QNetworkReply* and watches it.
/// A timeout is set which aborts the download if no response was received after N seconds.
///
/// \details Example
/// \code{cpp}
///    QNetworkReply* reply = network.get(request);
///    new NetworkReplyWatcher(this, reply); // will delete itself when the reply is deleted
/// \endcode
class NetworkReplyWatcher : public QObject
{
    Q_OBJECT

public:
    static constexpr char TIMEOUT_PROP[] = "wasTimeout";

public:
    // TODO: Use common Qt pattern: parent last
    NetworkReplyWatcher(QObject* parent, QNetworkReply* reply);
    NetworkReplyWatcher(QObject* parent, QNetworkReply* reply, std::chrono::seconds timeout);

    ~NetworkReplyWatcher() override = default;

public slots:
    void onTimeout();
    void onProgress();

private:
    void setReply(QNetworkReply* reply);

private:
    QNetworkReply* m_reply;
    QTimer m_timer;

    int m_timeoutMilliseconds = 10000; // 10s
};
