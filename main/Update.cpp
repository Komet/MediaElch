#include "Update.h"

#include <QApplication>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include <QUrl>
#include <QXmlStreamReader>
#include "globals/Helper.h"

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

    int result = Helper::instance()->compareVersionNumbers(QApplication::applicationVersion(), xmlVersion);
    version = QString("MediaElch %1 - %2").arg(xmlVersion).arg(codeName);

    return (result == 1);
}
