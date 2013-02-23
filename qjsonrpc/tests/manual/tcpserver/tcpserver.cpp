#include <QCoreApplication>
#include "testservice.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    TestService service;
    QJsonRpcTcpServer rpcServer;
    rpcServer.addService(&service);
    if (!rpcServer.listen(QHostAddress::LocalHost, 5555)) {
        qDebug() << "can't start tcp server: " << rpcServer.errorString();
        return -1;
    }

    return app.exec();
}
