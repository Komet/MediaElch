#include <QLocalSocket>

#include "qjsonrpcservice.h"
#include "json/qjsonvalue.h"
#include "localclient.h"

LocalClient::LocalClient(QObject *parent)
    : QObject(parent),
      m_client(0)
{
}

void LocalClient::run()
{
    QLocalSocket *socket = new QLocalSocket(this);
    socket->connectToServer("/tmp/testservice");
    if (!socket->waitForConnected()) {
        qDebug() << "could not connect to server: " << socket->errorString();
        return;
    }

    m_client = new QJsonRpcSocket(socket, this);
    QJsonRpcServiceReply *reply = m_client->invokeRemoteMethod("agent.testMethod");
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    reply = m_client->invokeRemoteMethod("agent.testMethodWithParams", "one", false, 10);
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    reply = m_client->invokeRemoteMethod("agent.testMethodWithVariantParams", "one", false, 10, QVariant(2.5));
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    reply = m_client->invokeRemoteMethod("agent.testMethodWithParamsAndReturnValue", "matt");
    connect(reply, SIGNAL(finished()), this, SLOT(processResponse()));

    // test bulk messages
    /*
    QJsonRpcMessage first = QJsonRpcMessage::createRequest("agent.testMethodWithParamsAndReturnValue", "testSendMessage");
    m_client->sendMessage(first);

    QJsonRpcMessage second = QJsonRpcMessage::createRequest("agent.testMethodWithParamsAndReturnValue", "testSendMessages1");
    QJsonRpcMessage third = QJsonRpcMessage::createRequest("agent.testMethodWithParamsAndReturnValue", "testSendMessages2");
    m_client->sendMessage(QList<QJsonRpcMessage>() << second << third);
    */
}

void LocalClient::processResponse()
{
    QJsonRpcServiceReply *reply = static_cast<QJsonRpcServiceReply *>(sender());
    if (!reply) {
        qDebug() << "invalid response received";
        return;
    }

    qDebug() << "response received: " << reply->response();
}
