#include <QCoreApplication>
#include <QDesktopServices>
#include <QLocalServer>
#include <QFile>
#include <QDir>

#include "testservice.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QString serviceName;
#if defined(Q_OS_WIN)
    QDir tempDirectory(QDesktopServices::storageLocation(QDesktopServices::TempLocation));
    serviceName = tempDirectory.absoluteFilePath("testservice");
#else
    serviceName = "/tmp/testservice";
#endif

    if (QFile::exists(serviceName)) {
        if (!QFile::remove(serviceName)) {
            qDebug() << "couldn't delete temporary service";
            return -1;
        }
    }

    TestService service;
    QJsonRpcLocalServer rpcServer;
    rpcServer.addService(&service);
    if (!rpcServer.listen(serviceName)) {
        qDebug() << "could not start server: " << rpcServer.errorString();
        return -1;
    }

    return app.exec();
}
