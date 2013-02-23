#ifndef QJSONRPCMESSAGE_H
#define QJSONRPCMESSAGE_H

#include <QSharedDataPointer>
#include <QVariant>

#if QT_VERSION >= 0x050000
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#else
#include "json/qjsonvalue.h"
#include "json/qjsonobject.h"
#include "json/qjsonarray.h"
#endif

#include "qjsonrpc_export.h"

// error codes defined by spec
namespace QJsonRpc {
    enum ErrorCode {
        NoError         = 0,
        ParseError      = -32700,           // Invalid JSON was received by the server.
                                            // An error occurred on the server while parsing the JSON text.
        InvalidRequest  = -32600,           // The JSON sent is not a valid Request object.
        MethodNotFound  = -32601,           // The method does not exist / is not available.
        InvalidParams   = -32602,           // Invalid method parameter(s).
        InternalError   = -32603,           // Internal JSON-RPC error.
        ServerErrorBase = -32000,           // Reserved for implementation-defined server-errors.
        UserError       = -32099,           // Anything after this is user defined
        TimeoutError    = -32100
    };
}

class QJsonRpcMessagePrivate;
class QJSONRPC_EXPORT QJsonRpcMessage
{
public:
    QJsonRpcMessage();
    QJsonRpcMessage(const QJsonObject &message);
    QJsonRpcMessage(const QJsonRpcMessage &other);
    QJsonRpcMessage &operator=(const QJsonRpcMessage &other);
    ~QJsonRpcMessage();

    enum Type {
        Invalid,
        Request,
        Response,
        Notification,
        Error
    };

    static QJsonRpcMessage createRequest(const QString &method, const QVariantList &params = QVariantList());
    static QJsonRpcMessage createRequest(const QString &method, const QVariant &param);
    static QJsonRpcMessage createNotification(const QString &method, const QVariantList &params = QVariantList());
    static QJsonRpcMessage createNotification(const QString &method, const QVariant &param);
    QJsonRpcMessage createResponse(const QVariant &result) const;
    QJsonRpcMessage createErrorResponse(QJsonRpc::ErrorCode code, const QString &message = QString(),
                                        const QVariant &data = QVariant()) const;

    QJsonRpcMessage::Type type() const;
    bool isValid() const;
    int id() const;

    // request
    QString method() const;
    QVariantList params() const;

    // response
    QVariant result() const;

    // error
    int errorCode() const;
    QString errorMessage() const;
    QVariant errorData() const;

    QJsonObject toObject() const;
    bool operator==(const QJsonRpcMessage &message) const;
    inline bool operator!=(const QJsonRpcMessage &message) const { return !(operator==(message)); }

private:
    friend class QJsonRpcMessagePrivate;
    QSharedDataPointer<QJsonRpcMessagePrivate> d;

};

QJSONRPC_EXPORT QDebug operator<<(QDebug, const QJsonRpcMessage &);
Q_DECLARE_METATYPE(QJsonRpcMessage)

#endif
