#include "IMDB.h"

#include <QScriptEngine>
#include <QScriptValueIterator>
#include <QWidget>
#include "data/Storage.h"
#include "globals/Helper.h"
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

QString IMDB::identifier()
{
    return QString("imdb");
}

bool IMDB::isAdult()
{
    return false;
}

bool IMDB::hasSettings()
{
    return false;
}

QWidget *IMDB::settingsWidget()
{
    return 0;
}

void IMDB::loadSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

void IMDB::saveSettings(QSettings &settings)
{
    Q_UNUSED(settings);
}

QList<int> IMDB::scraperSupports()
{
    return m_scraperSupports;
}

QList<int> IMDB::scraperNativelySupports()
{
    return m_scraperSupports;
}

void IMDB::search(QString searchStr)
{
    QString encodedSearch = Helper::urlEncode(searchStr);
    QUrl url;

    QRegExp rx("^tt\\d+$");
    if (rx.exactMatch(searchStr))
        url = QUrl::fromEncoded(QString("http://mymovieapi.com/?id=%1&type=json&plot=full&episode=0&limit=5&yg=0&mt=M&lang=en-US").arg(searchStr).toUtf8());
    else
        url = QUrl::fromEncoded(QString("http://mymovieapi.com/?title=%1&type=json&plot=full&episode=0&limit=5&yg=0&mt=M&lang=en-US").arg(encodedSearch).toUtf8());
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void IMDB::onSearchFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    reply->deleteLater();
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
            result.name     = Helper::urlDecode(it.value().property("title").toString());
            result.id       = it.value().property("imdb_id").toString();
            result.released = QDate::fromString(it.value().property("year").toString(), "yyyy");
            results.append(result);
        }
    } else {
        if (!sc.property("imdb_id").toString().isEmpty()) {
            ScraperSearchResult result;
            result.name     = Helper::urlDecode(sc.property("title").toString());
            result.id       = sc.property("imdb_id").toString();
            result.released = QDate::fromString(sc.property("year").toString(), "yyyy");
            results.append(result);
        }
    }

    return results;
}

void IMDB::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);
    movie->setId(ids.values().first());

    QUrl url(QString("http://mymovieapi.com/?id=%1&type=json&plot=full&episode=0&lang=en-US").arg(ids.values().first()));
    QNetworkRequest request(url);
    request.setRawHeader("Accept", "application/json");
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void IMDB::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    reply->deleteLater();
    movie->controller()->scraperLoadDone(this);
}

void IMDB::parseAndAssignInfos(QString json, Movie *movie, QList<int> infos)
{
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    if (infos.contains(MovieScraperInfos::Title) && sc.property("title").isValid())
        movie->setName(Helper::urlDecode(sc.property("title").toString()));
    if (infos.contains(MovieScraperInfos::Overview) && sc.property("plot").isValid() && !sc.property("plot").isNull()) {
        movie->setOverview(sc.property("plot").toString());
        if (sc.property("plot_simple").isValid() && !sc.property("plot_simple").isNull() && !sc.property("plot_simple").toString().isEmpty())
            movie->setOutline(sc.property("plot_simple").toString());
        else if (Settings::instance()->usePlotForOutline())
            movie->setOutline(sc.property("plot").toString());
    }
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("rating").isValid())
        movie->setRating(sc.property("rating").toNumber());
    if (infos.contains(MovieScraperInfos::Rating) && sc.property("rating_count").isValid())
        movie->setVotes(sc.property("rating_count").toInteger());
    if (infos.contains(MovieScraperInfos::Certification) && sc.property("rated").isValid())
        movie->setCertification(Helper::mapCertification(sc.property("rated").toString()));
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
                movie->addGenre(Helper::mapGenre(Helper::urlDecode(itC.value().toString())));
        }
    }

    if (infos.contains(MovieScraperInfos::Director) && sc.property("directors").isArray()) {
        QStringList directors;
        QScriptValueIterator itC(sc.property("directors"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                directors << Helper::urlDecode(itC.value().toString());
        }
        movie->setDirector(directors.join(", "));
    }

    if (infos.contains(MovieScraperInfos::Writer) && sc.property("writers").isArray()) {
        QStringList writers;
        QScriptValueIterator itC(sc.property("writers"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                writers << Helper::urlDecode(itC.value().toString());
        }
        movie->setWriter(writers.join(", "));
    }

    if (infos.contains(MovieScraperInfos::Countries) && sc.property("country").isArray()) {
        QScriptValueIterator itC(sc.property("country"));
        while (itC.hasNext()) {
            itC.next();
            if (itC.hasNext())
                movie->addCountry(Helper::mapCountry(Helper::urlDecode(itC.value().toString())));
        }
    }

    if (infos.contains(MovieScraperInfos::Actors) && sc.property("actors").isArray()) {
        QScriptValueIterator itC(sc.property("actors"));
        while (itC.hasNext()) {
            itC.next();
            Actor a;
            a.name = Helper::urlDecode(itC.value().toString());
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
