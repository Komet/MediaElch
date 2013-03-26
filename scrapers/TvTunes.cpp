#include "TvTunes.h"

#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegExp>
#include "globals/Helper.h"

TvTunes::TvTunes(QObject *parent) :
    QObject(parent)
{
}

void TvTunes::search(QString searchStr)
{
    searchStr = searchStr.replace(" ", "+");
    searchStr = Helper::urlEncode(searchStr);

    QUrl url(QString("http://www.televisiontunes.com/search.php?searWords=%1&search=").arg(searchStr));
    QNetworkRequest request(url);
    QNetworkReply *reply = m_qnam.get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void TvTunes::onSearchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    QList<ScraperSearchResult> results;
    if (reply->error() != QNetworkReply::NoError ) {
        qWarning() << "Network Error" << reply->errorString();
        emit sigSearchDone(results);
        return;
    }
    QString msg = QString::fromUtf8(reply->readAll());
    results = parseSearch(msg);
    emit sigSearchDone(results);
}

QList<ScraperSearchResult> TvTunes::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;

    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("[0-9]*\\.&nbsp;<a href=\"http://www.televisiontunes.com/(.*).html\">(.*)</a>");
    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.id = QString("http://www.televisiontunes.com/download.php?f=%1").arg(rx.cap(1));
        result.name = rx.cap(2);
        results.append(result);
        pos += rx.matchedLength();
    }

    return results;
}
