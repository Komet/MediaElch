#include "IMDB.h"

#include <QWidget>

#include "data/Storage.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"

IMDB::IMDB(QObject *parent)
{
    setParent(parent);
    m_scraperSupports << MovieScraperInfos::Title         //
                      << MovieScraperInfos::Director      //
                      << MovieScraperInfos::Writer        //
                      << MovieScraperInfos::Genres        //
                      << MovieScraperInfos::Tags          //
                      << MovieScraperInfos::Released      //
                      << MovieScraperInfos::Certification //
                      << MovieScraperInfos::Runtime       //
                      << MovieScraperInfos::Overview      //
                      << MovieScraperInfos::Rating        //
                      << MovieScraperInfos::Tagline       //
                      << MovieScraperInfos::Studios       //
                      << MovieScraperInfos::Countries     //
                      << MovieScraperInfos::Actors        //
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
    return nullptr;
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
        QUrl url = QUrl(QString("https://www.imdb.com/title/%1/").arg(searchStr).toUtf8());
        QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
        new NetworkReplyWatcher(this, reply);
        connect(reply, SIGNAL(finished()), this, SLOT(onSearchIdFinished()));
    } else {
        QUrl url = QUrl::fromEncoded(
            QString("https://www.imdb.com/find?s=tt&ttype=ft&ref_=fn_ft&q=%1").arg(encodedSearch).toUtf8());
        QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
        new NetworkReplyWatcher(this, reply);
        connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
    }
}

void IMDB::onSearchFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError) {
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
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    QList<ScraperSearchResult> results;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        ScraperSearchResult result;

        QRegExp rx;
        rx.setMinimal(true);
        rx.setPattern(R"(<h1 class="header"> <span class="itemprop" itemprop="name">(.*)</span>)");
        if (rx.indexIn(msg) != -1) {
            result.name = rx.cap(1);

            rx.setPattern("<h1 class=\"header\"> <span class=\"itemprop\" itemprop=\"name\">.*<span "
                          "class=\"nobr\">\\(<a href=\"[^\"]*\" >([0-9]*)</a>\\)</span>");
            if (rx.indexIn(msg) != -1) {
                result.released = QDate::fromString(rx.cap(1), "yyyy");
            } else {
                rx.setPattern("<h1 class=\"header\"> <span class=\"itemprop\" itemprop=\"name\">.*</span>.*<span "
                              "class=\"nobr\">\\(([0-9]*)\\)</span>");
                if (rx.indexIn(msg) != -1)
                    result.released = QDate::fromString(rx.cap(1), "yyyy");
            }
        } else {
            rx.setPattern("<h1 itemprop=\"name\" class=\"\">(.*)&nbsp;<span id=\"titleYear\">\\(<a "
                          "href=\"/year/([0-9]+)/\\?ref_=tt_ov_inf\"");
            if (rx.indexIn(msg) != -1) {
                result.name = rx.cap(1);
                result.released = QDate::fromString(rx.cap(2), "yyyy");
            }
        }

        rx.setPattern(R"(<link rel="canonical" href="https://www.imdb.com/title/(.*)/" />)");
        if (rx.indexIn(msg) != -1)
            result.id = rx.cap(1);

        if ((!result.id.isEmpty()) && (!result.name.isEmpty()))
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

    QRegExp rx("<td class=\"result_text\"> <a href=\"/title/([t]*[\\d]+)/[^\"]*\" >([^<]*)</a>(?: \\(I+\\) | "
               ")\\(([0-9]*)\\) (?:</td>|<br/>)");
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

void IMDB::loadData(QMap<ScraperInterface *, QString> ids, Movie *movie, QList<int> infos)
{
    movie->clear(infos);
    movie->setId(ids.values().first());

    QUrl url = QUrl(QString("https://www.imdb.com/title/%1/").arg(ids.values().first()).toUtf8());
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Accept-Language", "en");
    QNetworkReply *reply = qnam()->get(request);
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void IMDB::onLoadFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    reply->deleteLater();
    Movie *movie = reply->property("storage").value<Storage *>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
        QUrl posterViewerUrl = parsePosters(msg);
        if (infos.contains(MovieScraperInfos::Poster) && !posterViewerUrl.isEmpty()) {
            qDebug() << "Loading movie poster detail view";
            QNetworkReply *reply = qnam()->get(QNetworkRequest(posterViewerUrl));
            new NetworkReplyWatcher(this, reply);
            reply->setProperty("storage", Storage::toVariant(reply, movie));
            reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
            connect(reply, &QNetworkReply::finished, this, &IMDB::onPosterLoadFinished);
        }
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

void IMDB::onPosterLoadFinished()
{
    auto reply = static_cast<QNetworkReply *>(QObject::sender());
    auto posterId = reply->url().fileName();
    reply->deleteLater();
    Movie *movie = reply->property("storage").value<Storage *>()->movie();
    QList<int> infos = reply->property("infosToLoad").value<Storage *>()->infosToLoad();
    if (!movie)
        return;

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignPoster(msg, posterId, movie, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

void IMDB::parseAndAssignInfos(QString html, Movie *movie, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    rx.setPattern(R"(<h1 class="header">[^<]*<span class="itemprop" itemprop="name">([^<]*)</span>)");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1));

    rx.setPattern(R"(<h1 itemprop="name" class="">(.*)&nbsp;<span id="titleYear">)");
    if (infos.contains(MovieScraperInfos::Title) && rx.indexIn(html) != -1)
        movie->setName(rx.cap(1));

    if (infos.contains(MovieScraperInfos::Director)) {
        rx.setPattern("<div class=\"txt-block\" itemprop=\"director\" itemscope "
                      "itemtype=\"http://schema.org/Person\">(.*)</div>");
        QString directorsBlock;
        if (rx.indexIn(html) != -1) {
            directorsBlock = rx.cap(1);
        } else {
            rx.setPattern(R"(<div class="credit_summary_item">[^<]*<h4 class="inline">Director:</h4>(.*)</div>)");
            if (rx.indexIn(html) != -1)
                directorsBlock = rx.cap(1);
        }

        if (!directorsBlock.isEmpty()) {
            QStringList directors;
            rx.setPattern(
                R"(<a href="[^"]*"(.*)itemprop='url'><span class="itemprop" itemprop="name">([^<]*)</span></a>)");
            int pos = 0;
            while ((pos = rx.indexIn(directorsBlock, pos)) != -1) {
                directors << rx.cap(2);
                pos += rx.matchedLength();
            }
            movie->setDirector(directors.join(", "));
        }
    }

    if (infos.contains(MovieScraperInfos::Writer)) {
        rx.setPattern(
            R"(<div class="txt-block" itemprop="creator" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
        QString writersBlock;
        if (rx.indexIn(html) != -1) {
            writersBlock = rx.cap(1);
        } else {
            rx.setPattern(R"(<div class="credit_summary_item">[^<]*<h4 class="inline">Writers:</h4>(.*)</div>)");
            if (rx.indexIn(html) != -1)
                writersBlock = rx.cap(1);
        }

        if (!writersBlock.isEmpty()) {
            QStringList writers;
            rx.setPattern(
                R"(<a href="[^"]*"(.*)itemprop='url'><span class="itemprop" itemprop="name">([^<]*)</span></a>)");
            int pos = 0;
            while ((pos = rx.indexIn(writersBlock, pos)) != -1) {
                writers << rx.cap(2);
                pos += rx.matchedLength();
            }
            movie->setWriter(writers.join(", "));
        }
    }

    rx.setPattern(
        R"(<div class="see-more inline canwrap" itemprop="genre">[^<]*<h4 class="inline">Genres:</h4>(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Genres) && rx.indexIn(html) != -1) {
        QString genres = rx.cap(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(genres, pos)) != -1) {
            movie->addGenre(Helper::instance()->mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern(R"(<div class="txt-block">.*<h4 class="inline">Taglines:</h4>(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1) {
        QString tagline = rx.cap(1);
        QRegExp rxMore("<span class=\"see-more inline\">.*</span>");
        rxMore.setMinimal(true);
        tagline.remove(rxMore);
        movie->setTagline(tagline.trimmed());
    }

    rx.setPattern(R"(<div class="see-more inline canwrap" itemprop="keywords">(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Tags) && rx.indexIn(html) != -1) {
        QString keywords = rx.cap(1);
        rx.setPattern(R"(<span class="itemprop" itemprop="keywords">([^<]*)</span>)");
        int pos = 0;
        while ((pos = rx.indexIn(keywords, pos)) != -1) {
            movie->addTag(rx.cap(1).trimmed());
            pos += rx.matchedLength();
        }
    }

    if (infos.contains(MovieScraperInfos::Released)) {
        rx.setPattern("<a href=\"[^\"]*\"(.*)title=\"See all release dates\" >[^<]*<meta itemprop=\"datePublished\" "
                      "content=\"([^\"]*)\" />");
        if (rx.indexIn(html) != -1) {
            movie->setReleased(QDate::fromString(rx.cap(2), "yyyy-MM-dd"));
        } else {
            rx.setPattern("<h4 class=\"inline\">Release Date:</h4> ([0-9]+) ([A-z]*) ([0-9]{4})");
            if (rx.indexIn(html) != -1) {
                int day = rx.cap(1).trimmed().toInt();
                int month = -1;
                QString monthName = rx.cap(2).trimmed();
                int year = rx.cap(3).trimmed().toInt();
                if (monthName.contains("January", Qt::CaseInsensitive))
                    month = 1;
                else if (monthName.contains("February", Qt::CaseInsensitive))
                    month = 2;
                else if (monthName.contains("March", Qt::CaseInsensitive))
                    month = 3;
                else if (monthName.contains("April", Qt::CaseInsensitive))
                    month = 4;
                else if (monthName.contains("May", Qt::CaseInsensitive))
                    month = 5;
                else if (monthName.contains("June", Qt::CaseInsensitive))
                    month = 6;
                else if (monthName.contains("July", Qt::CaseInsensitive))
                    month = 7;
                else if (monthName.contains("August", Qt::CaseInsensitive))
                    month = 8;
                else if (monthName.contains("September", Qt::CaseInsensitive))
                    month = 9;
                else if (monthName.contains("October", Qt::CaseInsensitive))
                    month = 10;
                else if (monthName.contains("November", Qt::CaseInsensitive))
                    month = 11;
                else if (monthName.contains("December", Qt::CaseInsensitive))
                    month = 12;

                if (day != 0 && month != -1 && year != 0)
                    movie->setReleased(QDate(year, month, day));
            }
        }
    }


    rx.setPattern("<meta itemprop=\"contentRating\" content=\"([^\"]*)\">");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1)
        movie->setCertification(Helper::instance()->mapCertification(rx.cap(1)));

    rx.setPattern(R"(<time itemprop="duration" datetime="PT([0-9]+)M" >)");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).toInt());

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time itemprop="duration" datetime="PT([0-9]+)M">)");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1)
        movie->setRuntime(rx.cap(1).toInt());

    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        QString outline = rx.cap(1).remove(QRegExp("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        movie->setOutline(outline);
    }

    rx.setPattern(R"(<div class="summary_text" itemprop="description">(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        QString outline = rx.cap(1).remove(QRegExp("<[^>]*>")).trimmed();
        movie->setOutline(outline);
    }

    rx.setPattern(R"(<div class="inline canwrap" itemprop="description">(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        QString overview = rx.cap(1).trimmed();
        QRegExp rxWrittenBy("<em class=\"nobr\">.*</em>");
        rxWrittenBy.setMinimal(true);
        overview.remove(rxWrittenBy).remove(QRegExp("<[^>]*>"));
        movie->setOverview(overview.trimmed());
    }

    if (infos.contains(MovieScraperInfos::Rating)) {
        rx.setPattern("<div class=\"star-box-details\" itemtype=\"http://schema.org/AggregateRating\" itemscope "
                      "itemprop=\"aggregateRating\">(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString content = rx.cap(1);
            rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
            if (rx.indexIn(content) != -1)
                movie->setRating(rx.cap(1).trimmed().replace(",", ".").toFloat());

            rx.setPattern("<span itemprop=\"ratingCount\">(.*)</span>");
            if (rx.indexIn(content) != -1)
                movie->setVotes(rx.cap(1).replace(",", "").replace(".", "").toInt());
        } else {
            rx.setPattern("<div class=\"imdbRating\" itemtype=\"http://schema.org/AggregateRating\" itemscope=\"\" "
                          "itemprop=\"aggregateRating\">(.*)</div>");
            if (rx.indexIn(html) != -1) {
                QString content = rx.cap(1);
                rx.setPattern("([0-9]\\.[0-9]) based on ([0-9\\,]*) ");
                if (rx.indexIn(content) != -1) {
                    movie->setRating(rx.cap(1).trimmed().replace(",", ".").toFloat());
                    movie->setVotes(rx.cap(2).replace(",", "").replace(".", "").toInt());
                }
                rx.setPattern("([0-9]\\,[0-9]) based on ([0-9\\.]*) ");
                if (rx.indexIn(content) != -1) {
                    movie->setRating(rx.cap(1).trimmed().replace(",", ".").toFloat());
                    movie->setVotes(rx.cap(2).replace(",", "").replace(".", "").toInt());
                }
            }
        }
    }

    rx.setPattern("<strong>Top 250 #([0-9]+)</strong>");
    if (infos.contains(MovieScraperInfos::Rating) && rx.indexIn(html) != -1)
        movie->setTop250(rx.cap(1).toInt());

    if (infos.contains(MovieScraperInfos::Studios)) {
        rx.setPattern("<span itemprop=\"creator\" itemscope itemtype=\"http://schema.org/Organization\">.*<span "
                      "class=\"itemprop\" itemprop=\"name\">([^<]*)</span>.*</span>");
        int pos = 0;
        while ((pos = rx.indexIn(html, pos)) != -1) {
            movie->addStudio(Helper::instance()->mapStudio(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern(R"(<div class="txt-block">[^<]*<h4 class="inline">Country:</h4>(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Countries) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern(R"(<a href="[^"]*"[\n\s]*itemprop='url'>([^<]*)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            movie->addCountry(Helper::instance()->mapCountry(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern("<table class=\"cast_list\">(.*)</table>");
    if (infos.contains(MovieScraperInfos::Actors) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern(R"(<tr class="[^"]*">(.*)</tr>)");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            QString actor = rx.cap(1);
            pos += rx.matchedLength();

            Actor a;

            QRegExp rxName(R"(<span class="itemprop" itemprop="name">(.*)</span>)");
            rxName.setMinimal(true);
            if (rxName.indexIn(actor) != -1)
                a.name = rxName.cap(1).trimmed();

            QRegExp rxRole(R"(<td class="character">[\s\n]*<div>[\s\n](.*)[\s\n]*</div>)");
            rxRole.setMinimal(true);
            if (rxRole.indexIn(actor) != -1) {
                QString role = rxRole.cap(1);
                rxRole.setPattern(R"(<a href="[^"]*" >(.*)</a>)");
                if (rxRole.indexIn(role) != -1)
                    role = rxRole.cap(1);
                a.role = role.trimmed().replace(QRegExp("[\\s\\n]+"), " ");
            }

            QRegExp rxImg("<img [^<]*loadlate=\"([^\"]*)\"[^<]* />");
            rxImg.setMinimal(true);
            if (rxImg.indexIn(actor) != -1) {
                QString img = rxImg.cap(1);
                QRegExp aRx1("http://ia.media-imdb.com/images/(.*)/(.*)._V(.*).jpg");
                aRx1.setMinimal(true);
                if (aRx1.indexIn(img) != -1)
                    a.thumb = "http://ia.media-imdb.com/images/" + aRx1.cap(1) + "/" + aRx1.cap(2) + ".jpg";
                else
                    a.thumb = rxImg.cap(1);
            }

            movie->addActor(a);
        }
    }
}

QUrl IMDB::parsePosters(QString html)
{
    QRegExp rx("<div class=\"poster\">(.*)</div>");
    rx.setMinimal(true);
    if (rx.indexIn(html) == -1)
        return QUrl();

    QString content = rx.cap(1);
    rx.setPattern("<a href=\"([^\"]*)\"[^>]*>");
    if (rx.indexIn(content) == -1)
        return QUrl();

    return QString("https://www.imdb.com%1").arg(rx.cap(1));
}

void IMDB::parseAndAssignPoster(QString html, QString posterId, Movie *movie, QList<int> infos)
{
    // IMDB's media viewer contains all links to the gallery's image files.
    // We only want the poster, which has the id "viewerID".
    QString regex = QStringLiteral(R"url(window[.]IMDbReactInitialState[.]push\(.*"id":"%1",.*"src":"([^"]*)")url");
    QRegExp rx(regex.arg(posterId));
    rx.setMinimal(true);
    if (infos.contains(MovieScraperInfos::Poster) && rx.indexIn(html) != -1) {
        Poster p;
        p.originalUrl = rx.cap(1);
        p.thumbUrl = rx.cap(1);
        movie->addPoster(p);
    }
}
