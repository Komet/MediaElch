#pragma once

#include <functional>

namespace mediaelch {
namespace scraper {

/// \brief API interface for TheTvDb
class ImdbApi : public QObject
{
    Q_OBJECT

public:
    explicit ImdbApi(QObject* parent = nullptr);
    ~ImdbApi() override = default;

public:
    using ApiCallback = std::function<void(QString, ScraperError)>;

    void sendGetRequest(const Locale& locale, const QUrl& url, ApiCallback callback);

public:
    static QUrl makeFullUrl(const QString& suffix);

private:
    mediaelch::network::NetworkManager m_network;
    WebsiteCache m_cache;
};

} // namespace scraper
} // namespace mediaelch
