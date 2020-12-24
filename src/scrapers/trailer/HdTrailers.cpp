#include "HdTrailers.h"

#include <QQueue>
#include <QRegularExpression>

#include "globals/Helper.h"
#include "network/NetworkRequest.h"

namespace mediaelch {
namespace scraper {

HdTrailers::HdTrailers(QObject* parent) :
    m_network{new mediaelch::network::NetworkManager(this)}, m_searchReply{nullptr}, m_loadReply{nullptr}
{
    setParent(parent);
    m_libraryPages.enqueue('0');
    m_libraryPages.enqueue('a');
    m_libraryPages.enqueue('b');
    m_libraryPages.enqueue('c');
    m_libraryPages.enqueue('d');
    m_libraryPages.enqueue('e');
    m_libraryPages.enqueue('f');
    m_libraryPages.enqueue('g');
    m_libraryPages.enqueue('h');
    m_libraryPages.enqueue('i');
    m_libraryPages.enqueue('j');
    m_libraryPages.enqueue('k');
    m_libraryPages.enqueue('l');
    m_libraryPages.enqueue('m');
    m_libraryPages.enqueue('n');
    m_libraryPages.enqueue('o');
    m_libraryPages.enqueue('p');
    m_libraryPages.enqueue('q');
    m_libraryPages.enqueue('r');
    m_libraryPages.enqueue('s');
    m_libraryPages.enqueue('t');
    m_libraryPages.enqueue('u');
    m_libraryPages.enqueue('v');
    m_libraryPages.enqueue('w');
    m_libraryPages.enqueue('x');
    m_libraryPages.enqueue('y');
    m_libraryPages.enqueue('z');
}

QString HdTrailers::name()
{
    return QStringLiteral("HD-Trailers.net");
}

void HdTrailers::searchMovie(QString searchStr)
{
    m_currentSearch = searchStr;

    if (!m_libraryPages.isEmpty()) {
        QNetworkRequest request = mediaelch::network::requestWithDefaults(getLibraryUrl(m_libraryPages.dequeue()));
        m_searchReply = m_network->get(request);
        connect(m_searchReply, &QNetworkReply::finished, this, &HdTrailers::onSearchFinished);

    } else {
        QVector<ScraperSearchResult> results;
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

        QRegularExpression rx("<td class=\"trailer\"><a href=\"([^\"]*)\">([^<]*)</a>",
            QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator matches = rx.globalMatch(msg);

        while (matches.hasNext()) {
            QRegularExpressionMatch match = matches.next();
            m_urls.insert(match.captured(2).replace("&amp;", "&"), QUrl(match.captured(1)));
        }
    }

    m_searchReply->deleteLater();
    searchMovie(m_currentSearch);
}

void HdTrailers::loadMovieTrailers(QString id)
{
    m_loadReply = m_network->get(mediaelch::network::requestWithDefaults(QUrl("https://www.hd-trailers.net" + id)));
    connect(m_loadReply, &QNetworkReply::finished, this, &HdTrailers::onLoadFinished);
}

void HdTrailers::onLoadFinished()
{
    QVector<TrailerResult> trailers;
    if (m_loadReply->error() == QNetworkReply::NoError) {
        QString msg = m_loadReply->readAll();
        trailers = parseTrailers(msg);
    }
    m_loadReply->deleteLater();
    emit sigLoadDone(trailers);
}

QVector<TrailerResult> HdTrailers::parseTrailers(QString html)
{
    QVector<TrailerResult> results;

    QRegularExpression rx("<tr  itemprop=\"trailer\" itemscope itemtype=\"http://schema.org/VideoObject\">.*<td "
                          "class=\"bottomTableName\" rowspan=\"2\"><span class=\"standardTrailerName\" "
                          "itemprop=\"name\">(.*)</span>.*</td>(.*)</tr>",
        QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator matches = rx.globalMatch(html);

    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();

        QRegularExpression rx2("<td class=\"bottomTableResolution\"><a href=\"([^\"]*)\".*>([^<]*)</a></td>",
            QRegularExpression::InvertedGreedinessOption | QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator matches2 = rx2.globalMatch(match.captured(2));
        while (matches2.hasNext()) {
            QRegularExpressionMatch match2 = matches2.next();
            const QString url = match2.captured(1);

            // Hard-coded list of URLs that have been down for a while now. See:
            //  - https://web.archive.org/web/*/http://avideos.5min.com (offline since 2014)
            if (url.contains("5min.com")) {
                continue;
            }

            // HDTrailers requests a JSON from Yahoo and redirects to the URL with
            // correct resolution using JavaScript.
            // TODO: Download the JSON, parse it and show correct results.
            if (url.contains("yahoo-redir.php")) {
                continue;
            }

            TrailerResult r;
            r.trailerUrl = helper::urlDecode(url);
            r.name = QStringLiteral("%2, %1").arg(match.captured(1), match2.captured(2));
            results.append(r);
        }
    }
    return results;
}

QUrl HdTrailers::getLibraryUrl(char library)
{
    return QUrl(QStringLiteral("https://www.hd-trailers.net/library/%1/").arg(library));
}

} // namespace scraper
} // namespace mediaelch
