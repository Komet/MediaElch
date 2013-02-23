#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include "qjsonrpcmessage.h"

class QJsonRpcSocket;
class TcpClient : public QObject
{
    Q_OBJECT
public:
    TcpClient(QObject *parent = 0);
    void run();

private Q_SLOTS:
    void processResponse();

private:
    QJsonRpcSocket *m_client;

};

#endif
