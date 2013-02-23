#include <QCoreApplication>

#include "tcpclient.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    TcpClient client;
    client.run();
    return app.exec();
}
