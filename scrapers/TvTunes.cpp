#include "TvTunes.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"

TvTunes::TvTunes(QObject *parent) :
    QObject(parent)
{
}

void TvTunes::search(QString searchStr)
{
    searchStr = searchStr.replace(" ", "+");
    searchStr = Helper::instance()->urlEncode(searchStr);

    m_searchStr = searchStr;
    m_queue.clear();
    m_results.clear();

    QUrl url(QString("http://www.televisiontunes.com/search.php?q=%1").arg(searchStr));
    QNetworkRequest request(url);
    QNetworkReply *reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("searchStr", searchStr);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void TvTunes::onSearchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    QList<ScraperSearchResult> results;
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigSearchDone(results);
        return;
    }
    QString msg = QString::fromUtf8(reply->readAll());
    results = parseSearch(msg);
    foreach (ScraperSearchResult res, results)
        m_queue.enqueue(res);
    getNextDownloadUrl(reply->property("searchStr").toString());
}

QList<ScraperSearchResult> TvTunes::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;

    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<div class=\"jp\\-title\">.*<ul>.*<li><a href=\"([^\"]*)\">([^<]*)</a></li>");
    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.id = QString("http://www.televisiontunes.com%1").arg(rx.cap(1));
        result.name = rx.cap(2);
        results.append(result);
        pos += rx.matchedLength();
    }

    return results;
}

void TvTunes::getNextDownloadUrl(QString searchStr)
{
    if (m_queue.size() == 0 && searchStr == m_searchStr) {
        emit sigSearchDone(m_results);
        return;
    }

    ScraperSearchResult res = m_queue.dequeue();
    QNetworkReply *reply = m_qnam.get(QNetworkRequest(QUrl(res.id)));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("searchStr", searchStr);
    reply->setProperty("name", res.name);
    connect(reply, SIGNAL(finished()), this, SLOT(onDownloadUrlFinished()));
}

void TvTunes::onDownloadUrlFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        ScraperSearchResult res;
        res.name = reply->property("name").toString();

        QRegExp rx("<a id=\"download_song\" href=\"([^\"]*)\">");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            res.id = QString("http://www.televisiontunes.com%1").arg(rx.cap(1));
            m_results.append(res);
        }
    }

    getNextDownloadUrl(reply->property("searchStr").toString());
}

