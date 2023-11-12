#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"

#include "network/NetworkManager.h"
#include "network/NetworkRequest.h"
#include "test/test_helpers.h"

namespace test {

QString musicDownloadSyncOrFail(const QUrl& url, QUrl referrer)
{
    CAPTURE(url);
    bool finished = false;
    QString result;
    QEventLoop loop;

    mediaelch::network::NetworkManager network;

    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");

    if (referrer.isValid()) {
        request.setRawHeader("Referer", referrer.toString().toUtf8());
    }

    QNetworkReply* reply = network.getWithWatcher(request);
    QEventLoop::connect(reply, &QNetworkReply::finished, &loop, [&]() {
        finished = true;
        REQUIRE(reply->error() == QNetworkReply::NoError);
        if (reply->error() == QNetworkReply::NoError) {
            result = QString::fromUtf8(reply->readAll());
        }
        loop.quit();
    });

    if (!finished) {
        loop.exec();
    }

    REQUIRE_FALSE(result.isEmpty());

    return result;
}

} // namespace test
