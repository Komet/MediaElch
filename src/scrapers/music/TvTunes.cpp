#include "TvTunes.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>

#include "globals/Helper.h"
#include "network/NetworkRequest.h"

namespace mediaelch {
namespace scraper {

TvTunes::TvTunes(QObject* parent) : QObject(parent)
{
}

void TvTunes::search(QString searchStr)
{
    qInfo() << "[TvTunes] Search for show:" << searchStr;

    searchStr = searchStr.replace(" ", "+");
    searchStr = helper::urlEncode(searchStr);

    m_searchStr = searchStr;
    m_queue.clear();
    m_results.clear();

    QUrl url(QStringLiteral("https://www.televisiontunes.com/search.php?q=%1").arg(searchStr));
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    QNetworkReply* reply = m_network.getWithWatcher(request);
    reply->setProperty("searchStr", searchStr);
    connect(reply, &QNetworkReply::finished, this, &TvTunes::onSearchFinished);
}

void TvTunes::onSearchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    QVector<ScraperSearchResult> results;
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[TvTunes] Network Error:" << reply->errorString();
        emit sigSearchDone(results);
        return;
    }
    QString msg = QString::fromUtf8(reply->readAll());
    results = parseSearch(msg);
    for (const ScraperSearchResult& res : results) {
        m_queue.enqueue(res);
    }
    getNextDownloadUrl(reply->property("searchStr").toString());
}

QVector<ScraperSearchResult> TvTunes::parseSearch(QString html)
{
    QVector<ScraperSearchResult> results;

    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<div class=\"jp\\-title\">.*<ul>.*<li><a href=\"([^\"]*)\">([^<]*)</a></li>");
    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.id = QString("https://www.televisiontunes.com%1").arg(rx.cap(1));
        result.name = rx.cap(2);
        results.append(result);
        pos += rx.matchedLength();
        // Limit the result set to about 50 items.
        if (results.size() >= 50) {
            break;
        }
    }

    return results;
}

void TvTunes::getNextDownloadUrl(QString searchStr)
{
    if (m_queue.isEmpty()) {
        emit sigSearchDone(m_results);
        return;
    }

    ScraperSearchResult res = m_queue.dequeue();
    QNetworkReply* reply = m_network.getWithWatcher(mediaelch::network::requestWithDefaults(QUrl(res.id)));
    reply->setProperty("searchStr", searchStr);
    reply->setProperty("name", res.name);
    connect(reply, &QNetworkReply::finished, this, &TvTunes::onDownloadUrlFinished);
}

void TvTunes::onDownloadUrlFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        ScraperSearchResult res;
        res.name = reply->property("name").toString();

        QRegExp rx("<a id=\"download_song\" href=\"([^\"]*)\">");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            res.id = QString("https://www.televisiontunes.com%1").arg(rx.cap(1));
            m_results.append(res);
        }
    }

    getNextDownloadUrl(reply->property("searchStr").toString());
}

} // namespace scraper
} // namespace mediaelch
