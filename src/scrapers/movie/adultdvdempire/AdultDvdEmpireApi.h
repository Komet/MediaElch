#pragma once

#include "network/NetworkManager.h"
#include "scrapers/ScraperError.h"

#include <QObject>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for TheTvDb
class AdultDvdEmpireApi : public QObject
{
    Q_OBJECT

public:
    explicit AdultDvdEmpireApi(QObject* parent = nullptr);
    ~AdultDvdEmpireApi() override = default;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const QUrl& url, ApiCallback callback);
    void searchForMovie(const QString& query, ApiCallback callback);

    void loadMovie(const QString& id, ApiCallback callback);

public:
    static QUrl makeFullUrl(const QString& suffix);

private:
    QUrl makeApiUrl(const QString& suffix, QUrlQuery query) const;
    QUrl makeMovieSearchUrl(const QString& searchStr) const;
    QUrl makeMovieUrl(const QString& id) const;

private:
    mediaelch::network::NetworkManager m_network;
};

} // namespace scraper
} // namespace mediaelch
