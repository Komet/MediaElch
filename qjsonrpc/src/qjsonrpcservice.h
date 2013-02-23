#ifndef QJSONRPCSERVICE_H
#define QJSONRPCSERVICE_H

#include <QObject>
#include <QHostAddress>
#include <QPointer>

#include "qjsonrpcmessage.h"

class QJsonRpcSocket;
class QJsonRpcServiceProvider;
class QJSONRPC_EXPORT QJsonRpcService : public QObject
{
    Q_OBJECT
public:
    explicit QJsonRpcService(QObject *parent = 0);

Q_SIGNALS:
    void result(const QJsonRpcMessage &result);
    void notifyConnectedClients(const QJsonRpcMessage &message);
    void notifyConnectedClients(const QString &method, const QVariantList &params = QVariantList());

protected:
    QJsonRpcSocket *senderSocket();

private Q_SLOTS:
    bool dispatch(const QJsonRpcMessage &request);

private:
    void cacheInvokableInfo();
    static int s_qjsonRpcMessageType;
    QMultiHash<QByteArray, int> m_invokableMethodHash;
    QHash<int, QList<int> > m_parameterTypeHash;
    QPointer<QJsonRpcSocket> m_socket;
    friend class QJsonRpcServiceProvider;

};

class QJSONRPC_EXPORT QJsonRpcServiceReply : public QObject
{
    Q_OBJECT
public:
    explicit QJsonRpcServiceReply(QObject *parent = 0);
    QJsonRpcMessage response() const;

Q_SIGNALS:
    void finished();

private:
    QJsonRpcMessage m_response;
    friend class QJsonRpcSocket;
};

class QJsonRpcSocket;
class QJsonRpcServiceProviderPrivate;
class QJSONRPC_EXPORT QJsonRpcServiceProvider
{
public:
    ~QJsonRpcServiceProvider();
    virtual void addService(QJsonRpcService *service);

protected:
    QJsonRpcServiceProvider();
    void processMessage(QJsonRpcSocket *socket, const QJsonRpcMessage &message);
    QHash<QString, QJsonRpcService*> m_services;

};

class QJsonRpcSocketPrivate;
class QJSONRPC_EXPORT QJsonRpcSocket : public QObject
{
    Q_OBJECT
public:
    explicit QJsonRpcSocket(QIODevice *device, QObject *parent = 0);
    ~QJsonRpcSocket();

    enum WireFormat {
        Plain,
        Compact
    };
    WireFormat wireFormat() const;
    void setWireFormat(WireFormat format);

    bool isValid() const;

public Q_SLOTS:
    void notify(const QJsonRpcMessage &message);
    QJsonRpcMessage sendMessageBlocking(const QJsonRpcMessage &message, int msecs = 30000);
    QJsonRpcServiceReply *sendMessage(const QJsonRpcMessage &message);
//  void sendMessage(const QList<QJsonRpcMessage> &bulk);
    QJsonRpcMessage invokeRemoteMethodBlocking(const QString &method, const QVariant &arg1 = QVariant(),
                                              const QVariant &arg2 = QVariant(), const QVariant &arg3 = QVariant(),
                                              const QVariant &arg4 = QVariant(), const QVariant &arg5 = QVariant(),
                                              const QVariant &arg6 = QVariant(), const QVariant &arg7 = QVariant(),
                                              const QVariant &arg8 = QVariant(), const QVariant &arg9 = QVariant(),
                                              const QVariant &arg10 = QVariant());
    QJsonRpcServiceReply *invokeRemoteMethod(const QString &method, const QVariant &arg1 = QVariant(),
                                             const QVariant &arg2 = QVariant(), const QVariant &arg3 = QVariant(),
                                             const QVariant &arg4 = QVariant(), const QVariant &arg5 = QVariant(),
                                             const QVariant &arg6 = QVariant(), const QVariant &arg7 = QVariant(),
                                             const QVariant &arg8 = QVariant(), const QVariant &arg9 = QVariant(),
                                             const QVariant &arg10 = QVariant());

Q_SIGNALS:
    void messageReceived(const QJsonRpcMessage &message);

private Q_SLOTS:
    void processIncomingData();

protected:
    virtual void processRequestMessage(const QJsonRpcMessage &message);

private:
    Q_DECLARE_PRIVATE(QJsonRpcSocket)
    QScopedPointer<QJsonRpcSocketPrivate> d_ptr;

};

class QJSONRPC_EXPORT QJsonRpcServiceSocket : public QJsonRpcSocket,
                                               public QJsonRpcServiceProvider
{
    Q_OBJECT
public:
    explicit QJsonRpcServiceSocket(QIODevice *device, QObject *parent = 0);
    ~QJsonRpcServiceSocket();

private:
    virtual void processRequestMessage(const QJsonRpcMessage &message);

};

class QJsonRpcServerPrivate;
class QJSONRPC_EXPORT QJsonRpcServer : public QObject,
                                        public QJsonRpcServiceProvider
{
    Q_OBJECT
public:
    virtual ~QJsonRpcServer();
    virtual QString errorString() const = 0;
    virtual void addService(QJsonRpcService *service);

    QJsonRpcSocket::WireFormat wireFormat() const;
    void setWireFormat(QJsonRpcSocket::WireFormat format);

public Q_SLOTS:
    void notifyConnectedClients(const QJsonRpcMessage &message);
    void notifyConnectedClients(const QString &method, const QVariantList &params = QVariantList());

private Q_SLOTS:
    virtual void processIncomingConnection() = 0;
    virtual void clientDisconnected() = 0;
    void processMessage(const QJsonRpcMessage &message);

protected:
    explicit QJsonRpcServer(QJsonRpcServerPrivate *dd, QObject *parent);
    Q_DECLARE_PRIVATE(QJsonRpcServer)
    QScopedPointer<QJsonRpcServerPrivate> d_ptr;

};

class QJsonRpcLocalServerPrivate;
class QJSONRPC_EXPORT QJsonRpcLocalServer : public QJsonRpcServer
{
    Q_OBJECT
public:
    explicit QJsonRpcLocalServer(QObject *parent = 0);
    ~QJsonRpcLocalServer();

    QString errorString() const;
    bool listen(const QString &service);

private Q_SLOTS:
    void processIncomingConnection();
    void clientDisconnected();

private:
    Q_DECLARE_PRIVATE(QJsonRpcLocalServer)

};

class QJsonRpcTcpServerPrivate;
class QJSONRPC_EXPORT QJsonRpcTcpServer : public QJsonRpcServer
{
    Q_OBJECT
public:
    explicit QJsonRpcTcpServer(QObject *parent = 0);
    ~QJsonRpcTcpServer();

    QString errorString() const;
    bool listen(const QHostAddress &address, quint16 port);

private Q_SLOTS:
    void processIncomingConnection();
    void clientDisconnected();

private:
    Q_DECLARE_PRIVATE(QJsonRpcTcpServer)

};

#endif

