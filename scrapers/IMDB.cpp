#include "IMDB.h"

#include <QScriptEngine>
#include <QScriptValueIterator>
#include "settings/Settings.h"

IMDB::IMDB(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Poster
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Writer
                      << MovieScraperInfos::Certification;
}

QNetworkAccessManager *IMDB::qnam()
{
    return &m_qnam;
}

QString IMDB::name()
{
    return QString("IMDB");
}

QMap<QString, QString> IMDB::languages()
{
    return QMap<QString, QString>();
}

QString IMDB::language()
{
    return QString();
}

void IMDB::setLanguage(QString language)
{
    Q_UNUSED(language);
}

bool IMDB::hasSettings()
{
    return false;
}

void IMDB::loadSettings()
{
}

void IMDB::saveSettings()
{
}

QList<int> IMDB::scraperSupports()
{
    return m_scraperSupports;
}

void IMDB::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);
    QUrl url(QString("http://imdbapi.org/?q=%1&type=json&plot=full&episode=0&limit=5&yg=0&mt=M&lang=en-US").arg(encodedSearch));
    QNetworkRequest request(url);
    m_searchReply = qnam()->get(request);
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void IMDB::onSearchFinished()
{
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_searchReply->readAll());
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << m_searchReply->errorString();
    }
    m_searchReply->deleteLater();
    emit searchDone(results);
}

QList<ScraperSearchResult> IMDB::parseSearch(QString json)
{
    QList<ScraperSearchResult> results;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    if (sc.isArray() ) {
        QScriptValueIterator it(sc);
        while (it.hasNext() ) {
            it.next();
            if (it.value().property("imdb_id").toString().isEmpty())
                continue;
            ScraperSearchResult result;
            result.name     = it.value().property("title").toString();
            result.id       = it.value().property("imdb_id").toString();
            result.released = QDate::fromString(it.value().property("year").toString(), "yyyy");
            results.append(result);
        }
    }

    return results;
}

void IMDB::loadData(QString id, Movie *movie, QList<int> infos)
{
    m_currentMovie = movie;
    m_infosToLoad = infos;
    m_currentMovie->clear(infos);
    m_currentMovie->setId(id);

    QUrl url(QString("http://imdbapi.org/?id=%1&type=json&plot=full&episode=0&lang=en-US").arg(id));
    QNetworkRequest request(url);
    m_loadReply = qnam()->get(QNetworkRequest(request));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void IMDB::onLoadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentMovie, m_infosToLoad);
    } else {
        qWarning() << "Network Error (load)" << m_loadReply->errorString();
    }
    m_loadReply->deleteLater();
    m_currentMovie->controller()->scraperLoadDone();
}

void IMDB::parseAndAssignInfos(QString json, Movie *movie, QList<int> infos)
{
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    if (infos.contains(MovieScraperInfos::Title) && sc.property("title").isValid())
        movie->setName(sc.property("title").toString());
    // @todo: get plot_simple
    if (infos.contains(MovieScraperInfos::Overview) && sc.property("plot").isValid() && !sc.property("plot").isNull()) {
        movie->setOverview(sc.property("plot").toString());
        if (Settings::instance()->usePlotForOutline())
            movie->setOutline(sc.property("plot").toString());
    }
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("rating").isValid())
        movie->setRating(sc.property("rating").toNumber());
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("rating_count").isValid())
        movie->setVotes(sc.property("rating_count").toInteger());
    if (infos.contains(MovieScraperInfos::Certification) && sc.property("rated").isValid())
        movie->setCertification(sc.property("rated").toString());
    if (infos.contains(MovieScraperInfos::Released) && sc.property("release_date").isValid())
        movie->setReleased(QDate::fromString(sc.property("release_date").toString(), "yyyyMMdd"));
    if (infos.contains(MovieScraperInfos::Runtime) && sc.property("runtime").isArray()) {
        QScriptValueIterator itR(sc.property("runtime"));
        if (itR.hasNext()) {
            itR.next();
            QRegExp rx("(\\d+).*");
            if (rx.indexIn(itR.value().toString(), 0) != -1)
                movie->setRuntime(rx.cap(1).toInt());
        }
    }


    /* if (itC.hasNext()) within the loop might look wrong but
     * this way the last item is filtered out (contains the number of entries)
     */
    if (infos.contains(MovieScraperInfos::Genres) && sc.property("genres").isArray()) {
        QScriptValueIterator itC(sc.property("genres"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                movie->addGenre(itC.value().toString());
        }
    }

    if (infos.contains(MovieScraperInfos::Director) && sc.property("directors").isArray()) {
        QStringList directors;
        QScriptValueIterator itC(sc.property("directors"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                directors << itC.value().toString();
        }
        movie->setDirector(directors.join(", "));
    }

    if (infos.contains(MovieScraperInfos::Writer) && sc.property("writers").isArray()) {
        QStringList writers;
        QScriptValueIterator itC(sc.property("writers"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                writers << itC.value().toString();
        }
        movie->setWriter(writers.join(", "));
    }

    if (infos.contains(MovieScraperInfos::Countries) && sc.property("country").isArray()) {
        QScriptValueIterator itC(sc.property("country"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                movie->addCountry(itC.value().toString());
        }
    }

    if (infos.contains(MovieScraperInfos::Actors) && sc.property("actors").isArray()) {
        QScriptValueIterator itC(sc.property("actors"));
        while (itC.hasNext()) {
            itC.next();
            Actor a;
            a.name = itC.value().toString();
            if (itC.hasNext())
                movie->addActor(a);
        }
    }

    if (infos.contains(MovieScraperInfos::Poster) && sc.property("poster").isValid()) {
        Poster p;
        p.originalUrl = sc.property("poster").toString();
        p.thumbUrl = sc.property("poster").toString();
        movie->addPoster(p);
    }
}
