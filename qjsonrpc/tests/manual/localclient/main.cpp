#include <QCoreApplication>

#include "localclient.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    LocalClient client;
    client.run();
    return app.exec();
}
