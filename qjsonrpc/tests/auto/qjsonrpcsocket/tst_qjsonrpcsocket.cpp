#include <QLocalServer>
#include <QLocalSocket>

#include <QtCore/QEventLoop>
#include <QtCore/QVariant>
#include <QtTest/QtTest>

#include "json/qjsondocument.h"
#include "qjsonrpcservice_p.h"
#include "qjsonrpcservice.h"
#include "qjsonrpcmessage.h"

class TestQJsonRpcSocket: public QObject
{
    Q_OBJECT  
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void testSocketNoParameters();
    void testSocketMultiparamter();
    void testSocketNotification();
    void testSocketResponse();

private:
    // benchmark parsing speed
    void jsonParsingBenchmark();
};

void TestQJsonRpcSocket::initTestCase()
{
    qRegisterMetaType<QJsonRpcMessage>("QJsonRpcMessage");
}

void TestQJsonRpcSocket::cleanupTestCase()
{
}

void TestQJsonRpcSocket::init()
{
}

void TestQJsonRpcSocket::cleanup()
{
}

void TestQJsonRpcSocket::testSocketNoParameters()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QJsonRpcSocket serviceSocket(&buffer, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));
    QVERIFY(serviceSocket.isValid());

    QJsonRpcMessage request = QJsonRpcMessage::createRequest(QString("test.noParam"));

    QJsonRpcServiceReply *reply;
    reply = serviceSocket.sendMessage(request);
    Q_UNUSED(reply);

    QJsonDocument document = QJsonDocument::fromJson(buffer.data());
    QVERIFY(!document.isEmpty());
    QJsonRpcMessage bufferMessage(document.object());

    QCOMPARE(request.id(), bufferMessage.id());
    QCOMPARE(request.type(), bufferMessage.type());
    QCOMPARE(request.method(), bufferMessage.method());
    QCOMPARE(request.params(), bufferMessage.params());
    QCOMPARE(spyMessageReceived.count(), 0);
}

void TestQJsonRpcSocket::testSocketMultiparamter()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QJsonRpcSocket serviceSocket(&buffer, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));
    QVERIFY(serviceSocket.isValid());

    QJsonRpcMessage request = QJsonRpcMessage::createRequest(QString("test.multiParam"),
                                                             QVariantList() << false << true);

    QJsonRpcServiceReply *reply;
    reply = serviceSocket.sendMessage(request);
    Q_UNUSED(reply);

    QJsonDocument document = QJsonDocument::fromJson(buffer.data());
    QVERIFY(!document.isEmpty());
    QJsonRpcMessage bufferMessage(document.object());

    QCOMPARE(request.id(), bufferMessage.id());
    QCOMPARE(request.type(), bufferMessage.type());
    QCOMPARE(request.method(), bufferMessage.method());
    QCOMPARE(request.params(), bufferMessage.params());
    QCOMPARE(spyMessageReceived.count(), 0);
}

void TestQJsonRpcSocket::testSocketNotification()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QJsonRpcSocket serviceSocket(&buffer, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));
    QVERIFY(serviceSocket.isValid());

    QJsonRpcMessage notification = QJsonRpcMessage::createNotification("test.notify");

    QJsonRpcServiceReply *reply;
    reply = serviceSocket.sendMessage(notification);
    Q_UNUSED(reply);

    QJsonDocument document = QJsonDocument::fromJson(buffer.data());
    QVERIFY(!document.isEmpty());
    QJsonRpcMessage bufferMessage(document.object());

    QCOMPARE(notification.id(), bufferMessage.id());
    QCOMPARE(notification.type(), bufferMessage.type());
    QCOMPARE(notification.method(), bufferMessage.method());
    QCOMPARE(notification.params(), bufferMessage.params());
    QCOMPARE(spyMessageReceived.count(), 0);
}

void TestQJsonRpcSocket::testSocketResponse()
{
    QBuffer buffer;
    buffer.open(QIODevice::ReadWrite);
    QJsonRpcSocket serviceSocket(&buffer, this);
    QSignalSpy spyMessageReceived(&serviceSocket,
                                  SIGNAL(messageReceived(QJsonRpcMessage)));
    QVERIFY(serviceSocket.isValid());

    QJsonRpcMessage response = QJsonRpcMessage::createRequest(QString("test.response"));
    response = response.createResponse(QVariant());

    QJsonRpcServiceReply *reply;
    reply = serviceSocket.sendMessage(response);
    Q_UNUSED(reply);

    QJsonDocument document = QJsonDocument::fromJson(buffer.data());
    QVERIFY(!document.isEmpty());
    QJsonRpcMessage bufferMessage(document.object());

    QCOMPARE(response.id(), bufferMessage.id());
    QCOMPARE(response.type(), bufferMessage.type());
    QCOMPARE(response.method(), bufferMessage.method());
    QCOMPARE(response.params(), bufferMessage.params());
    QCOMPARE(spyMessageReceived.count(), 0);
}


void TestQJsonRpcSocket::jsonParsingBenchmark()
{
    QFile testData(":/testwire.json");
    QVERIFY(testData.open(QIODevice::ReadOnly));
    QByteArray jsonData = testData.readAll();
    QJsonRpcSocketPrivate socketPrivate;

    int messageCount = 0;
    while (!jsonData.isEmpty()) {
        int pos;
        QBENCHMARK {
            pos = socketPrivate.findJsonDocumentEnd(jsonData);
        }

        if (pos > -1) {
            messageCount++;
            jsonData = jsonData.mid(pos + 1);
        } else {
            break;
        }
    }

    QCOMPARE(messageCount, 8);
}

QTEST_MAIN(TestQJsonRpcSocket)
#include "tst_qjsonrpcsocket.moc"
