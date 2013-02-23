#include <QDebug>
#include "testservice.h"

TestService::TestService(QObject *parent)
    : QJsonRpcService(parent)
{
}

void TestService::testMethod()
{
    qDebug() << Q_FUNC_INFO << "called" << endl;
}

void TestService::testMethodWithParams(const QString &first, bool second, double third)
{
    qDebug() << Q_FUNC_INFO << "called with parameters: " << endl
             << " first: " << first << endl
             << "second: " << second << endl
             << " third: " << third << endl;
}

void TestService::testMethodWithVariantParams(const QString &first, bool second, double third, const QVariant &fourth)
{
    qDebug() << Q_FUNC_INFO << "called with variant parameters: " << endl
             << " first: " << first << endl
             << "second: " << second << endl
             << " third: " << third << endl
             << "fourth: " << fourth << endl;
}

QString TestService::testMethodWithParamsAndReturnValue(const QString &name)
{
    qDebug() << Q_FUNC_INFO << "called" << endl;
    return QString("Hello %1").arg(name);
}

void TestService::testMethodWithDefaultParameter(const QString &first, const QString &second)
{
    qDebug() << Q_FUNC_INFO << endl
             << "first: " << first << endl
             << (second.isEmpty() ? "not defined, default parameter" : second) << endl;
}

