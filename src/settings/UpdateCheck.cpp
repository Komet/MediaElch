#include "UpdateCheck.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QUrl>
#include <QXmlStreamReader>

#include "Version.h"
#include "globals/VersionInfo.h"
#include "log/Log.h"
#include "network/NetworkRequest.h"
#include "ui/main/Update.h"
#include "utils/Meta.h"

namespace mediaelch {

UpdateCheck::UpdateCheck(QObject* parent) : QObject(parent)
{
}

void UpdateCheck::checkForUpdate()
{
    // The GitHub repository https://github.com/mediaelch/mediaelch-meta contains
    // all metadata about MediaElch, e.g. the latest version.
    const QUrl url("https://raw.githubusercontent.com/mediaelch/mediaelch-meta/master/version.xml");
    auto request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = m_network.getWithWatcher(request);
    connect(reply, &QNetworkReply::finished, this, &UpdateCheck::onVersionXmlDownloaded);
}


UpdateCheck::Result UpdateCheck::createUpdateCheckResult(const QString& xmlString)
{
    Result result;

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
            if (xml.name() == QLatin1String("version")) {
                versionInfo = mediaelch::VersionInfo(xml.readElementText());

            } else if (xml.name() == QLatin1String("name")) {
                versionName = xml.readElementText();

            } else if (xml.name() == QLatin1String("released")) {
                released = xml.readElementText();

            } else if (xml.name() == QLatin1String("downloadUrl")) {
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
        if (xml.name() == QLatin1String("latestVersion")) {
            extractVersion();
        }
    }

    QUrl downloadUrl{downloadUrlOs.isEmpty() ? downloadUrlGeneric : downloadUrlOs};
    if (!downloadUrl.isValid()) {
        downloadUrl = QUrl{"https://mediaelch.github.io"};
    }

    result.versionName = QStringLiteral("MediaElch %1 - %2").arg(versionInfo.toString()).arg(versionName);
    result.downloadUrl = downloadUrl;
    result.releaseDate = released;
    result.isNewVersionAvailable = versionInfo.isValid() && versionInfo > mediaelch::currentVersion();
    return result;
}

void UpdateCheck::onVersionXmlDownloaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    auto dls = makeDeleteLaterScope(reply);
    if (reply == nullptr) {
        qCCritical(generic) << "[Updater] Dynamic Cast Failed";
        return;
    }
    if (reply->error() != QNetworkReply::NoError) {
        qCWarning(generic) << "[Updater] Network Error:" << reply->errorString();
        return;
    }

    QString xml = QString::fromUtf8(reply->readAll());

    Result result = createUpdateCheckResult(xml);
    emit updateCheckFinished(result);
}


} // namespace mediaelch
