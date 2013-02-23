#ifndef QJSONRPCSERVICESCRIPT_H
#define QJSONRPCSERVICESCRIPT_H

#include <QObject>
#include <QVariant>
#include <QScriptClass>
#include <QScriptable>

class QJsonRpcSocket;
class QJsonRpcServiceSocketPrototype : public QObject,
                                       protected QScriptable
{
    Q_OBJECT
public:
    QJsonRpcServiceSocketPrototype(QObject *parent = 0);
    ~QJsonRpcServiceSocketPrototype();

public Q_SLOTS:
    void connectToLocalService(const QString &service);
    QVariant invokeRemoteMethod(const QString &method, const QVariant &arg1 = QVariant(),
                                const QVariant &arg2 = QVariant(), const QVariant &arg3 = QVariant(),
                                const QVariant &arg4 = QVariant(), const QVariant &arg5 = QVariant(),
                                const QVariant &arg6 = QVariant(), const QVariant &arg7 = QVariant(),
                                const QVariant &arg8 = QVariant(), const QVariant &arg9 = QVariant(),
                                const QVariant &arg10 = QVariant());

private:
    QJsonRpcSocket *m_socket;

};

#endif

