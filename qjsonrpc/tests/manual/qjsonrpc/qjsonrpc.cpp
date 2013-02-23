#include <QCoreApplication>
#include <QStringList>
#include <QLocalSocket>
#include <QTcpSocket>
#include <QScopedPointer>
#include <QUrl>

#include "qjsonrpcservice.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QStringList args = app.arguments();
    QString appName = args.takeFirst();
    bool notification = args.contains("-n");
    if (notification)
        args.removeAll("-n");

    if (args.size() < 2) {
        qDebug("usage: %s [-n] <service> <method> <arguments>", appName.toLocal8Bit().data());
        return -1;
    }

    // try to process socket
    QIODevice *device = 0;
    QScopedPointer<QIODevice> devicePtr(device);
    QString service = args.takeFirst();
    QUrl serviceUrl = QUrl::fromUserInput(service);
    QHostAddress serviceAddress(serviceUrl.host());
    if (serviceAddress.isNull()) {
        QLocalSocket *localSocket = new QLocalSocket;
        device = localSocket;
        localSocket->connectToServer(service);
        if (!localSocket->waitForConnected(5000)) {
            qDebug("could not connect to service: %s", service.toLocal8Bit().data());
            return -1;
        }
    } else {
        QTcpSocket *tcpSocket = new QTcpSocket;
        device = tcpSocket;
        int servicePort = serviceUrl.port() ? serviceUrl.port() : 5555;
        tcpSocket->connectToHost(serviceAddress, servicePort);
        if (!tcpSocket->waitForConnected(5000)) {
            qDebug("could not connect to host at %s:%d", serviceUrl.host().toLocal8Bit().data(),
                   servicePort);
            return -1;
        }
    }

    QJsonRpcSocket socket(device);
    QString method = args.takeFirst();
    QVariantList arguments;
    foreach (QString arg, args)
        arguments.append(arg);

    QJsonRpcMessage request = notification ? QJsonRpcMessage::createNotification(method, arguments) :
                                             QJsonRpcMessage::createRequest(method, arguments);
    QJsonRpcMessage response = socket.sendMessageBlocking(request, 5000);
    if (response.type() == QJsonRpcMessage::Error) {
        qDebug("error(%d): %s", response.errorCode(), response.errorMessage().toLocal8Bit().data());
        return -1;
    }

    qDebug() << response.result();
}
