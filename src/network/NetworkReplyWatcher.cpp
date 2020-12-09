#include "network/NetworkReplyWatcher.h"

#include <QDebug>

constexpr char NetworkReplyWatcher::TIMEOUT_PROP[];

NetworkReplyWatcher::NetworkReplyWatcher(QObject* parent, QNetworkReply* reply) : QObject(parent), m_reply{nullptr}
{
    connect(&m_timer, &QTimer::timeout, this, &NetworkReplyWatcher::onTimeout);
    setReply(reply);
}

void NetworkReplyWatcher::setReply(QNetworkReply* reply)
{
    m_reply = reply;
    if (m_reply == nullptr) {
        return;
    }
    connect(m_reply, &QNetworkReply::finished, &m_timer, &QTimer::stop);
    connect(m_reply, &QObject::destroyed, this, &QObject::deleteLater);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &NetworkReplyWatcher::onProgress);
    m_timer.start(m_timeoutMilliseconds);
}

void NetworkReplyWatcher::onTimeout()
{
    if (m_reply != nullptr) {
        m_reply->setProperty(NetworkReplyWatcher::TIMEOUT_PROP, true);
        m_reply->abort();
    }
}

void NetworkReplyWatcher::onProgress()
{
    m_timer.start(m_timeoutMilliseconds);
}
