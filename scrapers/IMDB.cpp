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
                      << MovieScraperInfos::Director
                      << MovieScraperInfos::Writer
                      << MovieScraperInfos::Genres
                      << MovieScraperInfos::Tags
                      << MovieScraperInfos::Released
                      << MovieScraperInfos::Certification
                      << MovieScraperInfos::Runtime
                      << MovieScraperInfos::Overview
                      << MovieScraperInfos::Rating
                      << MovieScraperInfos::Tagline
                      << MovieScraperInfos::Studios
                      << MovieScraperInfos::Countries
                      << MovieScraperInfos::Actors
                      << MovieScraperInfos::Poster;
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
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);

    QRegExp rx("^tt\\d+$");
    if (rx.exactMatch(searchStr)) {
        QUrl url = QUrl(QString("http://www.imdb.com/title/%1/").arg(searchStr).toUtf8());
        QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
        connect(reply, SIGNAL(finished()), this, SLOT(onSearchIdFinished()));
    } else {
        QUrl url = QUrl::fromEncoded(QString("http://www.imdb.com/find?s=tt&ttype=ft&ref_=fn_ft&q=%1").arg(encodedSearch).toUtf8());
        QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
        connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
    }
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

void IMDB::onSearchIdFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        ScraperSearchResult result;
        QRegExp rx;
        rx.setMinimal(true);
        rx.setPattern("<h1 class=\"header\"> <span class=\"itemprop\" itemprop=\"name\">(.*)</span>[^<]*<span class=\"nobr\">\\(<a href=\"[^\"]*\" >([0-9]*)</a>\\)</span>");
        if (rx.indexIn(msg) != -1) {
            result.name = rx.cap(1);
            result.released = QDate::fromString(rx.cap(2), "yyyy");
        } else {
            rx.setPattern("<h1 class=\"header\"> <span class=\"itemprop\" itemprop=\"name\">(.*)</span>[^<]*<span class=\"nobr\">\\(([0-9]*)\\)</span>");
            if (rx.indexIn(msg) != -1) {
                result.name = rx.cap(1);
                result.released = QDate::fromString(rx.cap(2), "yyyy");
            }
        }

        rx.setPattern("<link rel=\"canonical\" href=\"http://www.imdb.com/title/(.*)/\" />");
        if (rx.indexIn(msg) != -1)
            result.id = rx.cap(1);

        if (!result.id.isEmpty() && !result.name.isEmpty())
            results.append(result);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    reply->deleteLater();
    emit searchDone(results);
}

QList<ScraperSearchResult> IMDB::parseSearch(QString html)
{
    QList<ScraperSearchResult> results;

    QRegExp rx("<td class=\"result_text\"> <a href=\"/title/(.*)/[^\"]*\" >([^<]*)</a> \\(([0-9]*)\\) </td>");
    rx.setMinimal(true);
    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        ScraperSearchResult result;
        result.name = rx.cap(2);
        result.id = rx.cap(1);
        result.released = QDate::fromString(rx.cap(3), "yyyy");
        results.append(result);
        pos += rx.matchedLength();
    }
    return results;
}

void IMDB::loadData(QMap<ScraperInterface*, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);
    movie->setId(ids.values().first());

    QUrl url = QUrl(QString("http://www.imdb.com/title/%1/").arg(ids.values().first()).toUtf8());
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Accept-Language", "en;q=0.8");
    QNetworkReply *reply = qnam()->get(request);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void IMDB::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
        QString posterUrl = parsePosters(msg);
        if (infos.contains(MovieScraperInfos::Poster) && !posterUrl.isEmpty()) {
            QNetworkReply *reply = qnam()->get(QNetworkRequest(posterUrl));
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, SIGNAL(finished()), this, SLOT(onPosterLoadFinished()));
        } else {
            movie->controller()->scraperLoadDone(this);
        }
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
}

void IMDB::onPosterLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignPoster(msg, movie, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

void IMDB::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern("<h1 class=\"header\">[^<]*<span class=\"itemprop\" itemprop=\"name\">([^<]*)</span>");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1));

    rx.setPattern("<div class=\"txt-block\" itemprop=\"director\" itemscope itemtype=\"http://schema.org/Person\">[^<]*"
                  "<h4 class=\"inline\">Director:</h4>[^<]"
                  "<a href=\"[^\"]*\" itemprop='url'><span class=\"itemprop\" itemprop=\"name\">([^<]*)</span></a>");
    if (infos.contains(MovieScraperInfos::Director) && rx.indexIn(html) != -1)
        movie->setDirector(rx.cap(1));

    rx.setPattern("<div class=\"txt-block\" itemprop=\"creator\" itemscope itemtype=\"http://schema.org/Person\">[^<]*"
                  "<h4 class=\"inline\">Writer:</h4>[^<]"
                  "<a href=\"[^\"]*\" itemprop='url'><span class=\"itemprop\" itemprop=\"name\">([^<]*)</span></a>");
    if (infos.contains(MovieScraperInfos::Writer) && rx.indexIn(html) != -1)
        movie->setWriter(rx.cap(1));

    rx.setPattern("<div class=\"see-more inline canwrap\" itemprop=\"genre\">[^<]*<h4 class=\"inline\">Genres:</h4>(.*)</div>");
    if (infos.contains(MovieScraperInfos::Genres) && rx.indexIn(html) != -1) {
        QString genres = rx.cap(1);
        rx.setPattern("<a href=\"[^\"]*\" >([^<]*)</a>");
        int pos = 0;
        while ((pos = rx.indexIn(genres, pos)) != -1) {
            movie->addGenre(Helper::instance()->mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern("<h4 class=\"inline\">Taglines:</h4>(.*)<span");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1)
        movie->setTagline(rx.cap(1).trimmed());

    rx.setPattern("<div class=\"see-more inline canwrap\" itemprop=\"keywords\">(.*)</div>");
    if (infos.contains(MovieScraperInfos::Tags) && rx.indexIn(html) != -1) {
        QString keywords = rx.cap(1);
        rx.setPattern("<span class=\"itemprop\" itemprop=\"keywords\">([^<]*)</span>");
        int pos = 0;
        while ((pos = rx.indexIn(keywords, pos)) != -1) {
            movie->addTag(rx.cap(1).trimmed());
            pos += rx.matchedLength();
        }
    }

    rx.setPattern("<a href=\"[^\"]*\" title=\"See all release dates\" >[^<]*<meta itemprop=\"datePublished\" content=\"([^\"]*)\" />");
    if (infos.contains(MovieScraperInfos::Released) && rx.indexIn(html) != -1)
        movie->setReleased(QDate::fromString(rx.cap(1), "yyyy-MM-dd"));

    rx.setPattern("itemprop=\"contentRating\" content=\"([^\"]*)\"></span>");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification(Helper::instance()->mapCertification(rx.cap(1)));

    rx.setPattern("<time itemprop=\"duration\" datetime=\"PT([0-9]+)M\" >");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).toInt());

    rx.setPattern("<p itemprop=\"description\">([^<]*)</p>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1)
        movie->setOutline(rx.cap(1).trimmed());

    rx.setPattern("<div class=\"inline canwrap\" itemprop=\"description\">[^<]*<p>([^<]*)");
    rx.setMinimal(false);
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1)
        movie->setOverview(rx.cap(1).trimmed());
    rx.setMinimal(true);

    rx.setPattern("<div class=\"star-box-details\" itemtype=\"http://schema.org/AggregateRating\" itemscope itemprop=\"aggregateRating\">(.*)</div>");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
        if (rx.indexIn(content) != -1)
            movie->setRating(rx.cap(1).trimmed().replace(",", ".").toFloat());

        rx.setPattern("<span itemprop=\"ratingCount\">(.*)</span>");
        if (rx.indexIn(content) != -1)
            movie->setVotes(rx.cap(1).replace(",", "").replace(".", "").toInt());
    }

    rx.setPattern("<strong>Top 250 #([0-9]+)</strong>");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1)
        movie->setTop250(rx.cap(1).toInt());

    rx.setPattern("<h3>Company Credits</h3>[^<]*<div class=\"txt-block\">(.*)</div>");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern("<span class=\"itemprop\" itemprop=\"name\">([^<]*)</span>");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            movie->addStudio(Helper::instance()->mapStudio(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern("<div class=\"txt-block\">[^<]*<h4 class=\"inline\">Country:</h4>(.*)</div>");
    if (infos.contains(MovieScraperInfos::Countries) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern("<a href=\"[^\"]*\" itemprop='url'>([^<]*)</a>");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            movie->addCountry(Helper::instance()->mapCountry(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern("<table class=\"cast_list\">(.*)</table>");
    if (infos.contains(MovieScraperInfos::Actors) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern("<tr class=\"[^\"]*\">[^<]*<td class=\"primary_photo\">[^<]*<a href=\"[^\"]*\" ><img [^<]*loadlate=\"([^\"]*)\"[^<]* /></a>[^<]*</td>[^<]*"
                      "<td class=\"itemprop\" itemprop=\"actor\" itemscope itemtype=\"http://schema.org/Person\">[^<]*<a href=\"[^\"]*\" itemprop='url'> <span class=\"itemprop\" itemprop=\"name\">([^<]*)</span>.*"
                      "<a href=\"/character/[^\"]*\" >([^<]*)</a>");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            Actor a;
            QRegExp aRx("http://ia.media-imdb.com/images/(.*)/(.*)._V(.*)_S(.*)([0-9]*)_CR[0-9]*,[0-9]*,[0-9]*,[0-9]*_.jpg");
            aRx.setMinimal(true);
            if (aRx.indexIn(rx.cap(1)) != -1)
                a.thumb = "http://ia.media-imdb.com/images/" + aRx.cap(1) + "/" + aRx.cap(2) + "._V" + aRx.cap(3) + "_SY317_CR0,0,214,317_.jpg";
            else
                a.thumb = rx.cap(1);
            a.name = rx.cap(2);
            a.role = rx.cap(3);
            movie->addActor(a);
            pos += rx.matchedLength();
        }
    }
}

QString IMDB::parsePosters(QString html)
{
    QRegExp rx("<div class=\"image\">(.*)</div>");
    rx.setMinimal(true);
    if (rx.indexIn(html) == -1)
        return QString();

    QString content = rx.cap(1);
    rx.setPattern("<a href=\"([^\"]*)\" >");
    if (rx.indexIn(content) == -1)
        return QString();

    return QString("http://www.imdb.com%1").arg(rx.cap(1));
}

void IMDB::parseAndAssignPoster(QString html, Movie *movie, QList<int> infos)
{
    QRegExp rx("<img onmousedown=\"return false;\" onmousemove=\"return false;\" oncontextmenu=\"return false;\" id=\"primary-img\" title=\"[^\"]*\" alt=\"[^\"]*\" src=\"([^\"]*)\" />");
    rx.setMinimal(true);
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.originalUrl = rx.cap(1);
        p.thumbUrl = rx.cap(1);
        movie->addPoster(p);
    }
}
