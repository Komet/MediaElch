#ifndef QJSONRPCMESSAGE_P_H
#define QJSONRPCMESSAGE_P_H

#include <QSharedData>
#include <QSharedPointer>
#include "qjsonrpcmessage.h"

class QJsonRpcMessagePrivate : public QSharedData
{
public:
    QJsonRpcMessagePrivate();
    ~QJsonRpcMessagePrivate();

    static QJsonRpcMessage createBasicRequest(const QString &method, const QVariantList &params);
    QJsonRpcMessage::Type type;
    QJsonObject *object;

    static int uniqueRequestCounter;

};

#endif
