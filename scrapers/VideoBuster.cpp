#include "VideoBuster.h"

VideoBuster::VideoBuster(QObject *parent)
{
    setParent(parent);
}

QNetworkAccessManager *VideoBuster::qnam()
{
    return &m_qnam;
}

QString VideoBuster::name()
{
    return QString("VideoBuster");
}

void VideoBuster::search(QString searchStr)
{
    QUrl url(QString("https://www.videobuster.de/titlesearch.php?tab_search_content=movies&view=title_list_view_option_list&search_title=%1").arg(searchStr));
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

void VideoBuster::searchFinished()
{
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = m_searchReply->readAll();
        results = parseSearch(msg);
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

QList<ScraperSearchResult> VideoBuster::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("class=\"movietip_yes\">([^<]*)</a>.*<a class=\"more\" href=\"([^\"]*)\">.*<label>Produktion:</label>.*([0-9]+)</div>");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name     = rx.cap(1);
        result.id       = rx.cap(2);
        result.released = QDate::fromString(rx.cap(3), "yyyy");
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

void VideoBuster::loadData(QString id, Movie *movie)
{
    m_currentMovie = movie;
    QUrl url(QString("https://www.videobuster.de%1").arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

void VideoBuster::loadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = m_loadReply->readAll();
        parseAndAssignInfos(msg, m_currentMovie);
    }
    m_loadReply->deleteLater();
    m_currentMovie->scraperLoadDone();
}

void VideoBuster::parseAndAssignInfos(QString html, Movie *movie)
{
    movie->clear();
    QRegExp rx;
    rx.setMinimal(true);
    int pos = 0;

    // Title
    rx.setPattern("class=\"name\">([^<]*)</h1>");
    if (rx.indexIn(html) != -1)
        movie->setName(rx.cap(1).trimmed());

    // Original Title
    rx.setPattern("Originaltitel:</div>.*<div class=\"content\">([^<]*)</div>");
    if (rx.indexIn(html) != -1)
        movie->setOriginalName(rx.cap(1).trimmed());

    // Year
    rx.setPattern("Produktion:</div>.*<div class=\"content\">.*([0-9]*)</div>");
    if (rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));

    // Country
    pos = 0;
    rx.setPattern("Produktion:</div>.*<div class=\"content\">(.*)([0-9]+|</div>)");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        movie->addCountry(rx.cap(1).trimmed());
        pos += rx.matchedLength();
    }

    // MPAA
    rx.setPattern("FSK ab ([0-9]+) ");
    if (rx.indexIn(html) != -1)
        movie->setCertification("FSK " + rx.cap(1));

    // Actors
    pos = 0;
    rx.setPattern("class=\"actor_link\">([^<]*)</a>");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        Actor a;
        a.name = rx.cap(1).trimmed();
        movie->addActor(a);
        pos += rx.matchedLength();
    }

    // Studio
    rx.setPattern("Studio:</div>.*<div class=\"content\">([^<]*)</div>");
    if (rx.indexIn(html) != -1)
        movie->addStudio(rx.cap(1).trimmed());

    // Runtime
    rx.setPattern("Laufzeit ca. ([0-9]*) Minuten");
    if (rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).trimmed().toInt());

    // Rating
    rx.setPattern("Gesamtwertung: ([0-9.]+) Sterne  bei ([0-9]*) Stimmen");
    if (rx.indexIn(html) != -1)
        movie->setRating(rx.cap(1).trimmed().toFloat());

    // Genres
    rx.setPattern("<a href='/genrelist.php/.*>([^<]*)</a>");
    if (rx.indexIn(html) != -1)
        movie->addGenre(rx.cap(1).trimmed());

    // Tagline
    rx.setPattern("class=\"long_name\">([^<]*)</p>");
    if (rx.indexIn(html) != -1)
        movie->setTagline(rx.cap(1).trimmed());

    // Overview
    rx.setPattern("<div class=\"txt movie_description\">(.*)(<br />|</div>)");
    if (rx.indexIn(html) != -1)
        movie->setOverview(rx.cap(1).trimmed());

    // Posters
    pos = 0;
    rx.setPattern("src=\"(https://gfx.videobuster.de/archive/resized)/w200/([^\"]*)\"");
    while ((pos = rx.indexIn(html, pos)) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1) + "/w200/" + rx.cap(2);
        p.originalUrl = rx.cap(1) + "/w700/" + rx.cap(2);
        movie->addPoster(p);
        pos += rx.matchedLength();
    }
}

bool VideoBuster::hasSettings()
{
    return false;
}

void VideoBuster::loadSettings()
{

}

void VideoBuster::saveSettings()
{

}

QWidget* VideoBuster::settingsWidget()
{
    return 0;
}
