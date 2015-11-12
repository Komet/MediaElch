#include "NetworkReplyWatcher.h"

#include <QDebug>

NetworkReplyWatcher::NetworkReplyWatcher(QObject *parent, QNetworkReply *reply) : QObject(parent)
{
    m_reply = 0;
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    setReply(reply);
}

NetworkReplyWatcher::~NetworkReplyWatcher()
{
}

void NetworkReplyWatcher::setReply(QNetworkReply *reply)
{
    m_reply = reply;
    if (!m_reply)
        return;
    connect(m_reply, SIGNAL(finished()), &m_timer, SLOT(stop()));
    connect(m_reply, SIGNAL(destroyed(QObject*)), this, SLOT(deleteLater()));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onProgress()));
    m_timer.start(3000);
}

void NetworkReplyWatcher::onTimeout()
{
    if (m_reply)
        m_reply->abort();
}

void NetworkReplyWatcher::onProgress()
{
    m_timer.start(3000);
}
