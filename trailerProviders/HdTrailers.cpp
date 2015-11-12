#include "HdTrailers.h"

#include <QRegExp>
#include "globals/Helper.h"

HdTrailers::HdTrailers(QObject *parent)
{
    setParent(parent);
    m_qnam = new QNetworkAccessManager(this);
    m_libraryPages.enqueue("0");
    m_libraryPages.enqueue("a");
    m_libraryPages.enqueue("b");
    m_libraryPages.enqueue("c");
    m_libraryPages.enqueue("d");
    m_libraryPages.enqueue("e");
    m_libraryPages.enqueue("f");
    m_libraryPages.enqueue("g");
    m_libraryPages.enqueue("h");
    m_libraryPages.enqueue("i");
    m_libraryPages.enqueue("j");
    m_libraryPages.enqueue("k");
    m_libraryPages.enqueue("l");
    m_libraryPages.enqueue("m");
    m_libraryPages.enqueue("n");
    m_libraryPages.enqueue("o");
    m_libraryPages.enqueue("p");
    m_libraryPages.enqueue("q");
    m_libraryPages.enqueue("r");
    m_libraryPages.enqueue("s");
    m_libraryPages.enqueue("t");
    m_libraryPages.enqueue("u");
    m_libraryPages.enqueue("v");
    m_libraryPages.enqueue("w");
    m_libraryPages.enqueue("x");
    m_libraryPages.enqueue("y");
    m_libraryPages.enqueue("z");
}

QString HdTrailers::name()
{
    return QString("HD-Trailers.net");
}

void HdTrailers::searchMovie(QString searchStr)
{
    m_currentSearch = searchStr;

    if (!m_libraryPages.isEmpty()) {
        QUrl url(QString("http://www.hd-trailers.net/library/%1/").arg(m_libraryPages.dequeue()));
        QNetworkRequest request(url);
        m_searchReply = m_qnam->get(request);
        connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
    } else {
        QList<ScraperSearchResult> results;
        QMapIterator<QString, QUrl> it(m_urls);
        while (it.hasNext()) {
            it.next();
            if (it.key().contains(searchStr, Qt::CaseInsensitive)) {
                ScraperSearchResult r;
                r.id = it.value().toString();
                r.name = it.key();
                results.append(r);
            }
        }
        emit sigSearchDone(results);
    }
}

void HdTrailers::onSearchFinished()
{
    if (m_searchReply->error() == QNetworkReply::NoError) {
        QString msg = m_searchReply->readAll();

        int pos = 0;
        QRegExp rx("<td class=\"trailer\"><a href=\"([^\"]*)\">([^<]*)</a>");
        rx.setMinimal(true);
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            m_urls.insertMulti(rx.cap(2), QUrl(rx.cap(1)));
            pos += rx.matchedLength();
        }
    }

    m_searchReply->deleteLater();
    searchMovie(m_currentSearch);
}

void HdTrailers::loadMovieTrailers(QString id)
{
    m_loadReply = m_qnam->get(QNetworkRequest(QUrl("http://www.hd-trailers.net" + id)));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void HdTrailers::onLoadFinished()
{
    QList<TrailerResult> trailers;
    if (m_loadReply->error() == QNetworkReply::NoError) {
        QString msg = m_loadReply->readAll();
        trailers = parseTrailers(msg);
    }
    m_loadReply->deleteLater();
    emit sigLoadDone(trailers);
}

QList<TrailerResult> HdTrailers::parseTrailers(QString html)
{
    QList<TrailerResult> results;

    int pos = 0;
    QRegExp rx("<tr itemprop=\"trailer\" itemscope itemtype=\"http://schema.org/VideoObject\">.*<td class=\"bottomTableName\" rowspan=\"2\"><span class=\"standardTrailerName\" itemprop=\"name\">(.*)</span>.*</td>(.*)</tr>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        QRegExp rx2("<td class=\"bottomTableResolution\"><a href=\"([^\"]*)\".*>([^<]*)</a></td>");
        rx2.setMinimal(true);
        int pos2 = 0;
        while ((pos2 = rx2.indexIn(rx.cap(2), pos2)) != -1) {
            pos2 += rx2.matchedLength();
            TrailerResult r;
            r.trailerUrl = Helper::instance()->urlDecode(rx2.cap(1));
            r.name = QString("%2, %1").arg(rx.cap(1)).arg(rx2.cap(2));
            results.append(r);
        }
        pos += rx.matchedLength();
    }
    return results;
}
