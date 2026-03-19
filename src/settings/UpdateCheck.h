#pragma once

#include "network/NetworkManager.h"

#include <QObject>
#include <QString>
#include <QUrl>

namespace mediaelch {

class UpdateCheck : public QObject
{
    Q_OBJECT

public:
    UpdateCheck(QObject* parent = nullptr);

    void checkForUpdate();

public:
    struct Result
    {
        QString versionName;
        QUrl downloadUrl;
        QString releaseDate;
        bool isNewVersionAvailable{false};
    };

signals:
    void updateCheckFinished(Result result);

private:
    void onVersionXmlDownloaded();
    Result createUpdateCheckResult(const QString& xmlString);

private:
    mediaelch::network::NetworkManager m_network;
};

} // namespace mediaelch
