#include "test/scrapers/universalmusicscraper/musicScraperUtils.h"

#include "network/NetworkManager.h"
#include "network/NetworkRequest.h"
#include "test/test_helpers.h"

namespace test {

QString downloadSyncOrFail(const QUrl& url)
{
    CAPTURE(url);
    bool finished = false;
    QString result;
    QEventLoop loop;

    mediaelch::network::NetworkManager network;

    QNetworkRequest request(url);
    mediaelch::network::useFirefoxUserAgent(request);
    request.setRawHeader("Accept-Language", "en-US,en;q=0.5");

    QNetworkReply* reply = network.getWithWatcher(request);
    QEventLoop::connect(reply, &QNetworkReply::finished, &loop, [&]() {
        finished = true;
        CHECK(reply->error() == QNetworkReply::NoError);
        result = QString::fromUtf8(reply->readAll());
        loop.quit();
    });

    if (!finished) {
        loop.exec();
    }
    return result;
}

} // namespace test
