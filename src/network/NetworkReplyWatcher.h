#pragma once

#include <QNetworkReply>
#include <QObject>
#include <QTimer>

/// \brief The NetworkReplyWatcher class takes a QNetworkReply* and watches it.
/// A timeout is set which aborts the download if no response was received after N seconds.
///
/// \par Example
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
    NetworkReplyWatcher(QObject* parent, QNetworkReply* reply);
    ~NetworkReplyWatcher() override = default;

public slots:
    void onTimeout();
    void onProgress();

private:
    void setReply(QNetworkReply* reply);

private:
    QNetworkReply* m_reply;
    QTimer m_timer;

    const int m_timeoutMilliseconds = 9000;
};
