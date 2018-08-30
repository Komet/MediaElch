#include <QtTest>

#include "testIMDb.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    TestIMDb testIMDb;

    return QTest::qExec(&testIMDb, argc, argv);
}
