#include <QHostAddress>
#include <QTcpSocket>

#include "qjsonrpcservice.h"
#include "json/qjsonvalue.h"
#include "tcpclient.h"

TcpClient::TcpClient(QObject *parent)
    : QObject(parent),
      m_client(0)
{
}

void TcpClient::run()
{
    QTcpSocket *socket = new QTcpSocket(this);
    socket->connectToHost(QHostAddress::LocalHost, 5555);
    if (!socket->waitForConnected()) {
        qDebug() << "could not connect to server: " << socket->errorString();
        return;
    }

    // run tests
    m_client = new QJsonRpcSocket(socket, this);
    QJsonRpcServiceReply *reply = m_client->invokeRemoteMethod("agent.testMethod");
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    reply = m_client->invokeRemoteMethod("agent.testMethodWithParams", "one", false, 10);
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    reply = m_client->invokeRemoteMethod("agent.testMethodWithVariantParams", "one", false, 10, QVariant(2.5));
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    reply = m_client->invokeRemoteMethod("agent.testMethodWithParamsAndReturnValue", "matt");
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));
}

void TcpClient::processResponse()
{
    QJsonRpcServiceReply *reply = static_cast<QJsonRpcServiceReply *>(sender());
    if (!reply) {
        qDebug() << "invalid response received";
        return;
    }

    qDebug() << "response received: " << reply->response();
}
