#ifndef QJSONRPCSERVICE_P_H
#define QJSONRPCSERVICE_P_H

#include <QHash>
#include <QHostAddress>
#include <QPointer>
#include <QLocalSocket>
#include <QTcpSocket>

#include "qjsondocument.h"
#include "qjsonrpcservice.h"
#include "qjsonrpc_export.h"

class QJSONRPC_EXPORT QJsonRpcSocketPrivate
{
public:
    QJsonRpcSocketPrivate() : format(QJsonRpcSocket::Compact) {}
    int findJsonDocumentEnd(const QByteArray &jsonData);
    void writeData(const QJsonRpcMessage &message);

    QPointer<QIODevice> device;
    QByteArray buffer;
    QHash<int, QJsonRpcServiceReply*> replies;
    QJsonRpcSocket::WireFormat format;

};

class QJsonRpcServerPrivate
{
public:
    QJsonRpcServerPrivate() : format(QJsonRpcSocket::Compact) {}
    QList<QJsonRpcSocket*> clients;
    QJsonRpcSocket::WireFormat format;

};

class QLocalServer;
class QJsonRpcLocalServerPrivate : public QJsonRpcServerPrivate
{
public:
    QJsonRpcLocalServerPrivate() : server(0) {}
    QLocalServer *server;
    QHash<QLocalSocket*, QJsonRpcSocket*> socketLookup;
};

class QTcpServer;
class QJsonRpcTcpServerPrivate : public QJsonRpcServerPrivate
{
public:
    QJsonRpcTcpServerPrivate() : server(0) {}
    QTcpServer *server;
    QHash<QTcpSocket*, QJsonRpcSocket*> socketLookup;
};

#endif
