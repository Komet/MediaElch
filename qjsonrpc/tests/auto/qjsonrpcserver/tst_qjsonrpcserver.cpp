#include <QLocalSocket>
#include <QTcpSocket>

#include <QtCore/QEventLoop>
#include <QtCore/QVariant>
#include <QtTest/QtTest>

#include "json/qjsondocument.h"
#include "qjsonrpcservice.h"
#include "qjsonrpcmessage.h"

class TestQJsonRpcServer: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Local Server
    void testLocalNoParameter();
    void testLocalSingleParameter();
    void testLocalMultiparameter();
    void testLocalVariantParameter();
    void testLocalVariantResult();
    void testLocalInvalidArgs();
    void testLocalMethodNotFound();
    void testLocalInvalidRequest();
    void testLocalNotifyConnectedClients();
    void testLocalNumberParameters();
    void testLocalHugeResponse();
    void testLocalComplexMethod();
    void testLocalDefaultParameters();
    void testLocalNotifyServiceSocket();
    void testLocalNoWhitespace();

    // TCP Server
    void testTcpNoParameter();
    void testTcpSingleParameter();
    void testTcpMultiparameter();
    void testTcpVariantParameter();
    void testTcpInvalidArgs();
    void testTcpMethodNotFound();
    void testTcpInvalidRequest();
    void testTcpNotifyConnectedClients();
    void testTcpNumberParameters();
    void testTcpHugeResponse();

private:
    // fix later
    void testLocalListOfInts();

};

class TestService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service")
public:
    TestService(QObject *parent = 0)
        : QJsonRpcService(parent),
          m_called(0)
    {}

    void resetCount() { m_called = 0; }
    int callCount() const {
        return m_called;
    }

public Q_SLOTS:
    void noParam() const {}
    QString singleParam(const QString &string) const { return string; }
    QString multipleParam(const QString &first,
                          const QString &second,
                          const QString &third) const
    {
        return first + second + third;
    }

    void numberParameters(int intParam, double doubleParam, float floatParam)
    {
        Q_UNUSED(intParam)
        Q_UNUSED(doubleParam)
        Q_UNUSED(floatParam)
    }

    bool variantParameter(const QVariant &variantParam) const
    {
        return variantParam.toBool();
    }

    QVariant variantStringResult() {
        return "hello";
    }

    QVariantList variantListResult() {
        return QVariantList() << "one" << 2 << 3.0;
    }

    QVariantMap variantMapResult() {
        QVariantMap result;
        result["one"] = 1;
        result["two"] = 2.0;
        return result;
    }

    void increaseCalled() {
        m_called++;
    }

    bool methodWithListOfInts(const QList<int> &list) {
        if (list.size() < 3)
            return false;

        if (list.at(0) != 300)
            return false;
        if (list.at(1) != 30)
            return false;
        if (list.at(2) != 3)
            return false;
        return true;
    }

private:
    int m_called;
};

void TestQJsonRpcServer::initTestCase()
{
    qRegisterMetaType<QJsonRpcMessage>("QJsonRpcMessage");
}

void TestQJsonRpcServer::cleanupTestCase()
{
}

void TestQJsonRpcServer::init()
{
}

void TestQJsonRpcServer::cleanup()
{
}

void TestQJsonRpcServer::testLocalNoParameter()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected(1000));
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.noParam");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
}

void TestQJsonRpcServer::testLocalSingleParameter()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.singleParam", QString("single"));
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
    QCOMPARE(response.result().toString(), QLatin1String("single"));
}

void TestQJsonRpcServer::testLocalMultiparameter()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.multipleParam",
                                                             QVariantList() << QVariant(QString("a"))
                                                                            << QVariant(QString("b"))
                                                                            << QVariant(QString("c")));
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
    QCOMPARE(response.result().toString(), QLatin1String("abc"));
}

void TestQJsonRpcServer::testLocalVariantParameter()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.variantParameter",
                                                             QVariantList() << QVariant(true));
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
    QVERIFY(response.result() == true);
}

void TestQJsonRpcServer::testLocalVariantResult()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);


    QJsonRpcMessage response = serviceSocket.invokeRemoteMethodBlocking("service.variantStringResult");
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QString stringResult = response.result().toString();
    QCOMPARE(stringResult, QLatin1String("hello"));
}

void TestQJsonRpcServer::testLocalInvalidArgs()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.noParam",
                                                             QVariantList() << false);
    serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVariant message = spyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage error = message.value<QJsonRpcMessage>();
    QCOMPARE(request.id(), error.id());
    QVERIFY(error.errorCode() == QJsonRpc::InvalidParams);
}

void TestQJsonRpcServer::testLocalMethodNotFound()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.doesNotExist");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.isValid());
    QVariant message = spyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage error = message.value<QJsonRpcMessage>();
    QCOMPARE(request.id(), error.id());
    QVERIFY(error.errorCode() == QJsonRpc::MethodNotFound);
}

void TestQJsonRpcServer::testLocalInvalidRequest()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    const char *invalid = "{\"jsonrpc\": \"2.0\", \"id\": 666}";

    QJsonDocument doc = QJsonDocument::fromJson(invalid);
    QJsonRpcMessage request(doc.object());
    serviceSocket.sendMessageBlocking(request);

    QCOMPARE(spyMessageReceived.count(), 1);
    QVariant message = spyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage error = message.value<QJsonRpcMessage>();
    QCOMPARE(request.id(), error.id());
    QVERIFY(error.errorCode() == QJsonRpc::InvalidRequest);
}

void TestQJsonRpcServer::testLocalNoWhitespace()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    TestService *service = new TestService;
    serviceProvider.addService(service);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());

    QByteArray singleNoWhite("{\"jsonrpc\":\"2.0\",\"method\":\"service.increaseCalled\"}");
    QCOMPARE(socket.write(singleNoWhite), (qint64)singleNoWhite.size());
    QTest::qWait(200);
    QCOMPARE(service->callCount(), 1);

    service->resetCount();
    QByteArray multipleNoWhite("{\"jsonrpc\":\"2.0\",\"method\":\"service.increaseCalled\"}{\"jsonrpc\":\"2.0\",\"method\":\"service.increaseCalled\"}{\"jsonrpc\":\"2.0\",\"method\":\"service.increaseCalled\"}");
    QCOMPARE(socket.write(multipleNoWhite), (qint64)multipleNoWhite.size());
    QTest::qWait(200);
    QCOMPARE(service->callCount(), 3);
}

class ServerNotificationHelper : public QObject
{
    Q_OBJECT
public:
    ServerNotificationHelper(const QJsonRpcMessage &message, QJsonRpcServer *provider)
        : m_provider(provider),
          m_notification(message) {}

public Q_SLOTS:
    void activate() {
        m_provider->notifyConnectedClients(m_notification);
    }

private:
    QJsonRpcServer *m_provider;
    QJsonRpcMessage m_notification;

};

void TestQJsonRpcServer::testLocalNotifyConnectedClients()
{
    // Initialize the service provider.
    QEventLoop firstLoop;
    QJsonRpcLocalServer serviceProvider;
    QVERIFY(serviceProvider.listen("test"));
    serviceProvider.addService(new TestService);

    // first client
    QLocalSocket first;
    first.connectToServer("test");
    QVERIFY(first.waitForConnected());
    QJsonRpcSocket firstClient(&first, this);
    QSignalSpy firstSpyMessageReceived(&firstClient,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    // send notification
    QJsonRpcMessage notification = QJsonRpcMessage::createNotification("testNotification");
    connect(&firstClient, SIGNAL(messageReceived(QJsonRpcMessage)), &firstLoop, SLOT(quit()));
    ServerNotificationHelper helper(notification, &serviceProvider);
    QTimer::singleShot(100, &helper, SLOT(activate()));
    firstLoop.exec();

    QCOMPARE(firstSpyMessageReceived.count(), 1);
    QVariant firstMessage = firstSpyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage firstNotification = firstMessage.value<QJsonRpcMessage>();
    QCOMPARE(firstNotification, notification);
}


class TestNumberParamsService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service")
public:
    TestNumberParamsService(QObject *parent = 0)
        : QJsonRpcService(parent), m_called(0) {}

    int callCount() const { return m_called; }

public Q_SLOTS:
    void numberParameters(int intParam, double doubleParam)
    {
        if (intParam == 10 && doubleParam == 3.14159)
            m_called++;
    }

private:
    int m_called;

};

void TestQJsonRpcServer::testLocalNumberParameters()
{
    // Initialize the service provider.
    QEventLoop loop;
    TestNumberParamsService *service = new TestNumberParamsService;
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(service);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.numberParameters", QVariantList() << 10 << 3.14159);
    serviceSocket.sendMessageBlocking(request);
    QCOMPARE(service->callCount(), 1);
}

class TestHugeResponseService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service")
public:
    TestHugeResponseService(QObject *parent = 0)
        : QJsonRpcService(parent) {}

public Q_SLOTS:
    QVariantMap hugeResponse()
    {
        QVariantMap result;
        for (int i = 0; i < 1000; i++) {
            QString key = QString("testKeyForHugeResponse%1").arg(i);
            result[key] = "some sample data to make the response larger";
        }
        return result;
    }
};

void TestQJsonRpcServer::testLocalHugeResponse()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestHugeResponseService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.hugeResponse");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.isValid());
}

class TestComplexMethodService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service.complex.prefix.for")
public:
    TestComplexMethodService(QObject *parent = 0)
        : QJsonRpcService(parent) {}

public Q_SLOTS:
    void testMethod() {}
};

void TestQJsonRpcServer::testLocalComplexMethod()
{
    // Initialize the service provider.
    QEventLoop loop;
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestComplexMethodService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.complex.prefix.for.testMethod");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
}

class TestDefaultParametersService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service")
public:
    TestDefaultParametersService(QObject *parent = 0)
        : QJsonRpcService(parent) {}

public Q_SLOTS:
    QString testMethod(const QString &name = QString()) {
        if (name.isEmpty())
            return "empty string";
        return QString("hello %1").arg(name);
    }

    QString testMethod2(const QString &name = QString(), int year = 2012)
    {
        return QString("%1%2").arg(name).arg(year);
    }
};

void TestQJsonRpcServer::testLocalDefaultParameters()
{
    // Initialize the service provider.
    QEventLoop loop;
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestDefaultParametersService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);

    // test without name
    QJsonRpcMessage noNameRequest = QJsonRpcMessage::createRequest("service.testMethod");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(noNameRequest);
    QVERIFY(response.type() != QJsonRpcMessage::Error);
    QCOMPARE(response.result().toString(), QLatin1String("empty string"));

    // test with name
    QJsonRpcMessage nameRequest = QJsonRpcMessage::createRequest("service.testMethod", QLatin1String("matt"));
    response = serviceSocket.sendMessageBlocking(nameRequest);
    QVERIFY(response.type() != QJsonRpcMessage::Error);
    QCOMPARE(response.result().toString(), QLatin1String("hello matt"));

    // test multiparameter
    QJsonRpcMessage konyRequest = QJsonRpcMessage::createRequest("service.testMethod2", QLatin1String("KONY"));
    response = serviceSocket.sendMessageBlocking(konyRequest);
    QVERIFY(response.type() != QJsonRpcMessage::Error);
    QCOMPARE(response.result().toString(), QLatin1String("KONY2012"));
}

class TestNotifyService : public QJsonRpcService
{
    Q_OBJECT
    Q_CLASSINFO("serviceName", "service")
public:
    TestNotifyService(QObject *parent = 0)
        : QJsonRpcService(parent)
    {
    }

public Q_SLOTS:
    void testMethod() { qDebug() << "text"; }
};

void TestQJsonRpcServer::testLocalNotifyServiceSocket()
{
    // Initialize the service provider.
    QJsonRpcLocalServer serviceProvider;
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());

    QJsonRpcServiceSocket serviceSocket(&socket);
    TestNumberParamsService *service = new TestNumberParamsService;
    serviceSocket.addService(service);
    QCOMPARE(service->callCount(), 0);

    QEventLoop test;
    QTimer::singleShot(10, &test, SLOT(quit()));
    test.exec();
    serviceProvider.notifyConnectedClients("service.numberParameters", QVariantList() << 10 << 3.14159);
    QTimer::singleShot(10, &test, SLOT(quit()));
    test.exec();

    QCOMPARE(service->callCount(), 1);
}

Q_DECLARE_METATYPE(QList<int>)
void TestQJsonRpcServer::testLocalListOfInts()
{
    // Initialize the service provider.
    QEventLoop loop;
    QJsonRpcLocalServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen("test"));

    // Connect to the socket.
    QLocalSocket socket;
    socket.connectToServer("test");
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);

    qRegisterMetaType<QList<int> >("QList<int>");
    QList<int> intList = QList<int>() << 300 << 30 << 3;
    QVariant variant = QVariant::fromValue(intList);

    QJsonRpcMessage intRequest = QJsonRpcMessage::createRequest("service.methodWithListOfInts", variant);
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(intRequest);
    QVERIFY(response.type() != QJsonRpcMessage::Error);
    QVERIFY(response.result().toBool());
}







void TestQJsonRpcServer::testTcpNoParameter()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());

    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.noParam");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
}

void TestQJsonRpcServer::testTcpSingleParameter()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.singleParam", QString("single"));
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
    QCOMPARE(response.result().toString(), QLatin1String("single"));
}

void TestQJsonRpcServer::testTcpMultiparameter()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.multipleParam",
                                                             QVariantList() << QVariant(QString("a"))
                                                                            << QVariant(QString("b"))
                                                                            << QVariant(QString("c")));
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
    QCOMPARE(response.result().toString(), QLatin1String("abc"));
}

void TestQJsonRpcServer::testTcpVariantParameter()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.variantParameter",
                                                             QVariantList() << QVariant(true));
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.errorCode() == QJsonRpc::NoError);
    QCOMPARE(request.id(), response.id());
    QVERIFY(response.result() == true);
}

void TestQJsonRpcServer::testTcpInvalidArgs()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.noParam",
                                                             QVariantList() << false);
    serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVariant message = spyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage error = message.value<QJsonRpcMessage>();
    QCOMPARE(request.id(), error.id());
    QVERIFY(error.errorCode() == QJsonRpc::InvalidParams);
}

void TestQJsonRpcServer::testTcpMethodNotFound()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.doesNotExist");
    serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVariant message = spyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage error = message.value<QJsonRpcMessage>();
    QCOMPARE(request.id(), error.id());
    QVERIFY(error.errorCode() == QJsonRpc::MethodNotFound);
}

void TestQJsonRpcServer::testTcpInvalidRequest()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    const char *invalid = "{\"jsonrpc\": \"2.0\", \"id\": 666}";

    QJsonDocument doc = QJsonDocument::fromJson(invalid);
    QJsonRpcMessage request(doc.object());
    serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVariant message = spyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage error = message.value<QJsonRpcMessage>();
    QCOMPARE(request.id(), error.id());
    QVERIFY(error.errorCode() == QJsonRpc::InvalidRequest);
}

void TestQJsonRpcServer::testTcpNotifyConnectedClients()
{
    // Initialize the service provider.
    QEventLoop firstLoop;
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // first client
    QTcpSocket first;
    first.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(first.waitForConnected());
    QJsonRpcSocket firstClient(&first, this);
    QSignalSpy firstSpyMessageReceived(&firstClient,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    // send notification
    QJsonRpcMessage notification = QJsonRpcMessage::createNotification("testNotification");
    connect(&firstClient, SIGNAL(messageReceived(QJsonRpcMessage)), &firstLoop, SLOT(quit()));
    ServerNotificationHelper helper(notification, &serviceProvider);
    QTimer::singleShot(100, &helper, SLOT(activate()));
    firstLoop.exec();

    QCOMPARE(firstSpyMessageReceived.count(), 1);
    QVariant firstMessage = firstSpyMessageReceived.takeFirst().at(0);
    QJsonRpcMessage firstNotification = firstMessage.value<QJsonRpcMessage>();
    QCOMPARE(firstNotification, notification);
}

void TestQJsonRpcServer::testTcpNumberParameters()
{
    // Initialize the service provider.
    TestNumberParamsService *service = new TestNumberParamsService;
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(service);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.numberParameters", QVariantList() << 10 << 3.14159);
    serviceSocket.sendMessageBlocking(request);
    QCOMPARE(service->callCount(), 1);
}

void TestQJsonRpcServer::testTcpHugeResponse()
{
    // Initialize the service provider.
    QJsonRpcTcpServer serviceProvider;
    serviceProvider.addService(new TestHugeResponseService);
    QVERIFY(serviceProvider.listen(QHostAddress::LocalHost, 5555));

    // Connect to the socket.
    QTcpSocket socket;
    socket.connectToHost(QHostAddress::LocalHost, 5555);
    QVERIFY(socket.waitForConnected());
    QJsonRpcSocket serviceSocket(&socket, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));

    QJsonRpcMessage request = QJsonRpcMessage::createRequest("service.hugeResponse");
    QJsonRpcMessage response = serviceSocket.sendMessageBlocking(request);
    QCOMPARE(spyMessageReceived.count(), 1);
    QVERIFY(response.isValid());
}

QTEST_MAIN(TestQJsonRpcServer)
#include "tst_qjsonrpcserver.moc"
