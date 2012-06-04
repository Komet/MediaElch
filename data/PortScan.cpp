#include "PortScan.h"

PortScan::PortScan(QObject *parent) :
    QThread(parent)
{
}

void PortScan::setUrl(QUrl url, int timeout)
{
    m_url = url;
    m_timeout = timeout;
}

void PortScan::run()
{
    m_socket = new QTcpSocket();
    m_socket->connectToHost(m_url.host(), m_url.port(), QIODevice::ReadOnly);
    if (m_socket->waitForConnected(m_timeout))
        emit result(true);
    else
        emit result(false);
    m_socket->close();
    m_socket->deleteLater();
}
