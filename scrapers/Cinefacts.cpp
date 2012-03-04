#include "Cinefacts.h"

Cinefacts::Cinefacts(QObject *parent)
{
    setParent(parent);
}

QString Cinefacts::name()
{
    return QString("Cinefacts");
}

bool Cinefacts::hasSettings()
{
    return false;
}

void Cinefacts::loadSettings()
{
}

void Cinefacts::saveSettings()
{
}

QWidget *Cinefacts::settingsWidget()
{
    return 0;
}

QNetworkAccessManager *Cinefacts::qnam()
{
    return &m_qnam;
}

void Cinefacts::search(QString searchStr)
{
    QUrl url(QString("http://www.cinefacts.de/suche/suche.php?name=%1").arg(searchStr));
    m_searchReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(searchFinished()));
}

void Cinefacts::searchFinished()
{
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = m_searchReply->readAll();
        results = parseSearch(msg);
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

QList<ScraperSearchResult> Cinefacts::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;
    int pos = 0;
    QRegExp rx("<a href=\"/kino/([0-9]*)/(.[^/]*)/filmdetails.html\">[^<]*<b title=\"([^\"]*)\" class=\"headline\".*([0-9]{4})");
    rx.setMinimal(true);
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name     = rx.cap(3);
        result.id       = rx.cap(1) + "/" + rx.cap(2);
        result.released = QDate::fromString(rx.cap(4), "yyyy");
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

void Cinefacts::loadData(QString id, Movie *movie)
{
    m_currentMovie = movie;
    QUrl url(QString("http://www.cinefacts.de/kino/%1/filmdetails.html").arg(id));
    m_loadReply = this->qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(loadFinished()));
}

void Cinefacts::loadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = m_loadReply->readAll();
        parseAndAssignInfos(msg, m_currentMovie);
    } else {
        m_currentMovie->scraperLoadDone();
    }
    m_loadReply->deleteLater();
}

void Cinefacts::parseAndAssignInfos(QString html, Movie *movie)
{
    movie->clear();
    QRegExp rx;
    rx.setMinimal(true);
    int pos = 0;

    // Title
    rx.setPattern("<h1>([^<]*)<");
    if (rx.indexIn(html) != -1)
        movie->setName(rx.cap(1).trimmed());

    // Original Title
    rx.setPattern("<dt class=\"c1\">Originaltitel:</dt>[^<]*<dd class=\"first\">(.[^<]*)</dd>");
    if (rx.indexIn(html) != -1)
        movie->setOriginalName(rx.cap(1).trimmed());

    // Genre
    rx.setPattern("Genre:([^:]*)<dt");
    if (rx.indexIn(html) != -1) {
        QString genres = rx.cap(1);
        pos = 0;
        rx.setPattern(">*[ A-Za-z]([^<>]*)</a>");
        while ((pos = rx.indexIn(genres, pos)) != -1) {
            movie->addGenre(rx.cap(1).trimmed());
            pos += rx.matchedLength();
        }
    }

    // Year
    rx.setPattern("</a> ([0-9]*) </dd>");
    if (rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1).trimmed(), "yyyy"));

    // Country
    rx.setPattern("Produktionsland:([^:]*)<dt");
    if (rx.indexIn(html) != -1) {
        QString countries = rx.cap(1);
        pos = 0;
        rx.setPattern("<a href=\"[^\"]*\">([^<]*)</a>");
        while ((pos = rx.indexIn(countries, pos)) != -1) {
            movie->addCountry(rx.cap(1).trimmed());
            pos += rx.matchedLength();
        }
    }

    // Actors
    rx.setPattern("Darsteller:</td>(.*)</table");
    if (rx.indexIn(html) != -1) {
        QString actors = rx.cap(1);
        pos = 0;
        rx.setPattern(">([^<>]*)</a></td>+[^<]+<[^>]+> als([ A-Za-z]*)&nbsp;");
        while ((pos = rx.indexIn(actors, pos)) != -1) {
            Actor actor;
            actor.name = rx.cap(1).trimmed();
            actor.role = rx.cap(2).trimmed();
            movie->addActor(actor);
            pos += rx.matchedLength();
        }
    }

    // Studio
    rx.setPattern("Verleih:([^\\.]*)\\.");
    if (rx.indexIn(html) != -1)
        movie->addStudio(rx.cap(1).trimmed());

    // MPAA
    rx.setPattern("FSK:</dt>[^>]*>ab ([^<]*) Jahren<");
    if (rx.indexIn(html) != -1)
        movie->setCertification("FSK " + rx.cap(1));

    // Runtime
    rx.setPattern("L.nge:</dt>[^>]*>([0-9]*) Minuten");
    if (rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).trimmed().toInt());

    // Overview
    rx.setPattern("KURZINHALT</h2></li>.*<li[^>]*>(.*)</li>");
    if (rx.indexIn(html) != -1) {
        movie->setOverview(rx.cap(1).trimmed());
    }

    // Posters
    rx.setPattern("<a href=\"/kino/film/([0-9]*)/([^/]*)/plakate.html\">");
    if (rx.indexIn(html) != -1) {
        QUrl posterUrl(QString("http://www.cinefacts.de/kino/film/%1/%2/plakate.html").arg(rx.cap(1)).arg(rx.cap(2)));
        m_posterReply = this->qnam()->get(QNetworkRequest(posterUrl));
        connect(m_posterReply, SIGNAL(finished()), this, SLOT(posterFinished()));
    } else {
        m_currentMovie->scraperLoadDone();
    }
}

void Cinefacts::posterFinished()
{
    m_posterQueue.clear();
    if (m_posterReply->error() == QNetworkReply::NoError ) {
        QString msg = m_posterReply->readAll();
        int pos = 0;
        QRegExp rx("<a href=\"/kino/film/([^\"]+)\">[^<]*<img");
        rx.setMinimal(true);
        while ((pos = rx.indexIn(msg, pos)) != -1) {
            QUrl url(QString("http://www.cinefacts.de/kino/film/%1").arg(rx.cap(1)));
            m_posterQueue.append(url);
            pos += rx.matchedLength();
        }
        startNextPosterDownload();
    } else {
        m_currentMovie->scraperLoadDone();
    }
    m_posterReply->deleteLater();
}

void Cinefacts::startNextPosterDownload()
{
    if (m_posterQueue.isEmpty()) {
        m_currentMovie->scraperLoadDone();
        return;
    }
    QUrl url = m_posterQueue.dequeue();
    m_posterSubReply = qnam()->get(QNetworkRequest(url));
    connect(m_posterSubReply, SIGNAL(finished()), this, SLOT(posterSubFinished()));
}

void Cinefacts::posterSubFinished()
{
    if (m_posterSubReply->error() == QNetworkReply::NoError ) {
        QString msg = m_posterSubReply->readAll();
        QRegExp rx("src=\"/kino/plakat/([^\"]*)\"");
        rx.setMinimal(true);
        if (rx.indexIn(msg) != -1) {
            Poster p;
            p.thumbUrl = QUrl(QString("http://www.cinefacts.de/kino/plakat/%1").arg(rx.cap(1)));
            rx.setPattern("<a rel=\"shadowbox\" href=\"/kino/plakat/([^\"]*)\"");
            if (rx.indexIn(msg) != -1)
                p.originalUrl = QUrl(QString("http://www.cinefacts.de/kino/plakat/%1").arg(rx.cap(1)));
            m_currentMovie->addPoster(p);
        }
    }
    m_posterSubReply->deleteLater();
    startNextPosterDownload();
}
