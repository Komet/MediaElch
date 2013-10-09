#include "Update.h"

#include <QApplication>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>

Update::Update(QObject *parent) :
    QObject(parent)
{
}

Update *Update::instance(QObject *parent)
{
    static Update *m_instance = 0;
    if (!m_instance)
        m_instance = new Update(parent);
    return m_instance;
}

void Update::checkForUpdate()
{
    QUrl url("http://data.mediaelch.de/version.xml");
    QNetworkReply *reply = m_qnam.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onCheckFinished()));
}

void Update::onCheckFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        QString version;
        if (checkIfNewVersion(msg, version))
            emit sigNewVersion(version);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    reply->deleteLater();
}

bool Update::checkIfNewVersion(QString msg, QString &version)
{
    QString os;
#ifdef Q_OS_MAC
    os = "mac";
#endif
#ifdef Q_OS_WIN
    os = "win";
#endif
#ifdef Q_OS_LINUX
    os = "linux";
#endif

    QString xmlVersion;
    QString codeName;
    QXmlStreamReader xml(msg);
    while (xml.readNextStartElement()) {
        if (xml.name() == "latestVersion_" + os || (xmlVersion.isEmpty() && xml.name() == "latestVersion"))
            xmlVersion = xml.readElementText();
        if (xml.name() == "codeName")
            codeName = xml.readElementText();
    }

    if (xmlVersion.isEmpty())
        return false;

    int mMajor;
    int mMinor;
    int mBugfix;
    int xmlMajor;
    int xmlMinor;
    int xmlBugfix;

    QRegExp rxBig("^([0-9])\\.([0-9])\\.([0-9])");
    QRegExp rxNormal("^([0-9])\\.([0-9])");

    if (rxBig.indexIn(QApplication::applicationVersion()) != -1) {
        mMajor = rxBig.cap(1).toInt();
        mMinor = rxBig.cap(2).toInt();
        mBugfix = rxBig.cap(3).toInt();
    } else if (rxNormal.indexIn(QApplication::applicationVersion()) != -1) {
        mMajor = rxNormal.cap(1).toInt();
        mMinor = rxNormal.cap(2).toInt();
        mBugfix = 0;
    } else {
        return false;
    }

    if (rxBig.indexIn(xmlVersion) != -1) {
        xmlMajor = rxBig.cap(1).toInt();
        xmlMinor = rxBig.cap(2).toInt();
        xmlBugfix = rxBig.cap(3).toInt();
    } else if (rxNormal.indexIn(xmlVersion) != -1) {
        xmlMajor = rxNormal.cap(1).toInt();
        xmlMinor = rxNormal.cap(2).toInt();
        xmlBugfix = 0;
    } else {
        return false;
    }

    version = QString("MediaElch %1 - %2").arg(xmlVersion).arg(codeName);

    if (xmlMajor > mMajor)
        return true;

    if (xmlMajor == mMajor && xmlMinor > mMinor)
        return true;

    if (xmlMajor == mMajor && xmlMinor == mMinor && xmlBugfix > mBugfix)
        return true;

    return false;
}
