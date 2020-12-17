#include "Update.h"

#include <QApplication>
#include <QCheckBox>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QXmlStreamReader>

#include "Version.h"
#include "globals/Helper.h"
#include "globals/VersionInfo.h"
#include "network/NetworkRequest.h"
#include "settings/Settings.h"

Update::Update(QObject* parent) : QObject(parent)
{
}

Update* Update::instance(QObject* parent)
{
    static Update* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new Update(parent);
    }
    return m_instance;
}

void Update::checkForUpdate()
{
    // The GitHub repository https://github.com/mediaelch/mediaelch-meta contains
    // all meta data about MediaElch, e.g. the latest version.
    const QUrl url("https://raw.githubusercontent.com/mediaelch/mediaelch-meta/master/version.xml");
    auto request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = m_network.getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &Update::onCheckFinished);
}

void Update::onCheckFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[Updater] Dynamic Cast Failed";
    }
    reply->deleteLater();
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[Updater] Network Error:" << reply->errorString();
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    QString version;
    QString downloadUrl;
    if (checkIfNewVersion(msg, version, downloadUrl)) {
        QString downloadLink = QStringLiteral("<a href=\"%1\">https://mediaelch.github.io</a>")
                                   .arg(QUrl(downloadUrl).isValid() ? downloadUrl : "https://mediaelch.github.io");

        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setWindowTitle(tr("Updates available"));
        msgBox.setText(tr("%1 is now available.<br>Get it now on %2").arg(version).arg(downloadLink));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setIconPixmap(QPixmap(":/img/MediaElch.png").scaledToWidth(64, Qt::SmoothTransformation));

        QCheckBox dontCheck(tr("Don't check for updates"), &msgBox);
        dontCheck.blockSignals(true);
        msgBox.addButton(&dontCheck, QMessageBox::ActionRole);
        msgBox.exec();
        if (dontCheck.checkState() == Qt::Checked) {
            Settings::instance()->setCheckForUpdates(false);
            Settings::instance()->saveSettings();
        }
    }
}

bool Update::checkIfNewVersion(QString xmlString, QString& version, QString& downloadUrl)
{
    mediaelch::VersionInfo versionInfo;
    QString versionName;
    QString released;
    QString downloadUrlOs;
    QString downloadUrlGeneric;

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
    QXmlStreamReader xml(xmlString);

    const auto extractVersion = [&]() {
        while (xml.readNextStartElement()) {
            if (xml.name() == "version") {
                versionInfo = mediaelch::VersionInfo(xml.readElementText());

            } else if (xml.name() == "name") {
                versionName = xml.readElementText();

            } else if (xml.name() == "released") {
                released = xml.readElementText();

            } else if (xml.name() == "downloadUrl") {
                QString system = xml.attributes().value("system").toString();
                if (system == os) {
                    downloadUrlOs = xml.readElementText();
                } else if (system == "generic") {
                    downloadUrlGeneric = xml.readElementText();
                }
            }
        }
    };

    while (xml.readNextStartElement()) {
        if (xml.name() == "latestVersion") {
            extractVersion();
        }
    }

    version = QStringLiteral("MediaElch %1 - %2").arg(versionInfo.toString()).arg(versionName);
    downloadUrl = downloadUrlOs.isEmpty() ? downloadUrlGeneric : downloadUrlOs;

    return versionInfo.isValid() && versionInfo > mediaelch::currentVersion();
}
