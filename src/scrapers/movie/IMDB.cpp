#include "IMDB.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QWidget>

#include "data/Storage.h"
#include "globals/Helper.h"
#include "globals/NetworkReplyWatcher.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

IMDB::IMDB(QObject* parent) : m_loadAllTags{false}
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

    m_settingsWidget = new QWidget(MainWindow::instance());
    m_loadAllTagsWidget = new QCheckBox(tr("Load all tags"), m_settingsWidget);
    auto layout = new QGridLayout(m_settingsWidget);
    layout->addWidget(m_loadAllTagsWidget, 0, 0);
    layout->setContentsMargins(12, 0, 12, 12);
    m_settingsWidget->setLayout(layout);
}

QString IMDB::name() const
{
    return QStringLiteral("IMDb");
}

QString IMDB::identifier() const
{
    return scraperIdentifier;
}

bool IMDB::isAdult() const
{
    return false;
}

bool IMDB::hasSettings() const
{
    return true;
}

QWidget* IMDB::settingsWidget()
{
    return m_settingsWidget;
}

void IMDB::loadSettings(const ScraperSettings& settings)
{
    m_loadAllTags = settings.valueBool("LoadAllTags", false);
    m_loadAllTagsWidget->setChecked(m_loadAllTags);
}

void IMDB::saveSettings(ScraperSettings& settings)
{
    m_loadAllTags = m_loadAllTagsWidget->isChecked();
    settings.setBool("LoadAllTags", m_loadAllTags);
}

QVector<MovieScraperInfos> IMDB::scraperSupports()
{
    return m_scraperSupports;
}

QVector<MovieScraperInfos> IMDB::scraperNativelySupports()
{
    return m_scraperSupports;
}

std::vector<ScraperLanguage> IMDB::supportedLanguages()
{
    return {{tr("English"), "en"}};
}

void IMDB::changeLanguage(QString /*languageKey*/)
{
    // no-op: Only one language is supported and it is hard-coded.
}

QString IMDB::defaultLanguageKey()
{
    return QStringLiteral("en");
}

void IMDB::search(QString searchStr)
{
    QString encodedSearch = QUrl::toPercentEncoding(searchStr);

    QRegExp rx("^tt\\d+$");
    if (rx.exactMatch(searchStr)) {
        QUrl url = QUrl(QStringLiteral("https://www.imdb.com/title/%1/").arg(searchStr).toUtf8());
        QNetworkRequest request(url);
        request.setRawHeader("Accept-Language", "en"); // todo: add language dropdown in settings
        QNetworkReply* reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        connect(reply, &QNetworkReply::finished, this, &IMDB::onSearchIdFinished);

    } else {
        QUrl url = QUrl::fromEncoded(
            QStringLiteral("https://www.imdb.com/find?s=tt&ttype=ft&ref_=fn_ft&q=%1").arg(encodedSearch).toUtf8());
        QNetworkRequest request(url);
        request.setRawHeader("Accept-Language", "en"); // todo: add language dropdown in settings
        QNetworkReply* reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        connect(reply, &QNetworkReply::finished, this, &IMDB::onSearchFinished);
    }
}

void IMDB::onSearchFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[IMDb] onSearchFinished: nullptr reply | Please report this issue!";
        emit searchDone({}, {ScraperSearchError::ErrorType::InternalError, tr("Internal Error: Please report!")});
        return;
    }
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[IMDb] Search: Network Error" << reply->errorString();
        emit searchDone({}, {ScraperSearchError::ErrorType::NetworkError, reply->errorString()});
        return;
    }

    QString msg = QString::fromUtf8(reply->readAll());
    auto results = parseSearch(msg);
    emit searchDone(results, {});
}

void IMDB::onSearchIdFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    QVector<ScraperSearchResult> results;
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
                if (rx.indexIn(msg) != -1) {
                    result.released = QDate::fromString(rx.cap(1), "yyyy");
                }
            }
        } else {
            rx.setPattern(R"(<h1 class="">(.*)&nbsp;<span id="titleYear">\(<a href="/year/([0-9]+)/\?ref_=tt_ov_inf")");
            if (rx.indexIn(msg) != -1) {
                result.name = rx.cap(1);
                result.released = QDate::fromString(rx.cap(2), "yyyy");
            }
        }

        rx.setPattern(R"(<link rel="canonical" href="https://www.imdb.com/title/(.*)/" />)");
        if (rx.indexIn(msg) != -1) {
            result.id = rx.cap(1);
        }

        if ((!result.id.isEmpty()) && (!result.name.isEmpty())) {
            results.append(result);
        }
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }

    reply->deleteLater();
    emit searchDone(results, {});
}

QVector<ScraperSearchResult> IMDB::parseSearch(QString html)
{
    QVector<ScraperSearchResult> results;

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

void IMDB::loadData(QMap<MovieScraperInterface*, QString> ids, Movie* movie, QVector<MovieScraperInfos> infos)
{
    QString imdbId = ids.values().first();
    movie->clear(infos);
    movie->setId(ImdbId(imdbId));

    QUrl url = QUrl(QString("https://www.imdb.com/title/%1/").arg(imdbId).toUtf8());
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Accept-Language", "en");
    QNetworkReply* reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, &QNetworkReply::finished, this, &IMDB::onLoadFinished);
}

void IMDB::onLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    QVector<MovieScraperInfos> infos = reply->property("infosToLoad").value<Storage*>()->movieInfosToLoad();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, movie, infos);
        QUrl posterViewerUrl = parsePosters(msg);
        if (infos.contains(MovieScraperInfos::Poster) && !posterViewerUrl.isEmpty()) {
            qDebug() << "Loading movie poster detail view";
            QNetworkReply* posterReply = m_qnam.get(QNetworkRequest(posterViewerUrl));
            new NetworkReplyWatcher(this, posterReply);
            posterReply->setProperty("storage", Storage::toVariant(posterReply, movie));
            posterReply->setProperty("infosToLoad", Storage::toVariant(posterReply, infos));
            connect(posterReply, &QNetworkReply::finished, this, &IMDB::onPosterLoadFinished);
        }
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    // IMDb has an extra page listing all tags (popular movies can have more than 100 tags).
    if (m_loadAllTags && infos.contains(MovieScraperInfos::Tags)) {
        QUrl tagsUrl =
            QUrl(QStringLiteral("https://www.imdb.com/title/%1/keywords").arg(movie->imdbId().toString()).toUtf8());
        QNetworkReply* tagsReply = m_qnam.get(QNetworkRequest(tagsUrl));
        tagsReply->setProperty("storage", Storage::toVariant(tagsReply, movie));
        new NetworkReplyWatcher(this, tagsReply);
        connect(tagsReply, &QNetworkReply::finished, this, &IMDB::onTagsFinished);

    } else {
        movie->controller()->scraperLoadDone(this);
    }
}

void IMDB::onTagsFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignTags(msg, *movie);

    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (load tags)" << reply->errorString();
    }

    movie->controller()->scraperLoadDone(this);
}

void IMDB::onPosterLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    auto posterId = reply->url().fileName();
    reply->deleteLater();
    Movie* movie = reply->property("storage").value<Storage*>()->movie();
    if (movie == nullptr) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignPoster(msg, posterId, movie);
    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    movie->controller()->scraperLoadDone(this);
}

void IMDB::parseAndAssignInfos(const QString& html, Movie* movie, QVector<MovieScraperInfos> infos)
{
    using namespace std::chrono;

    QRegExp rx;
    rx.setMinimal(true);

    if (infos.contains(MovieScraperInfos::Title)) {
        rx.setPattern(R"(<h1 class="[^"]*">([^<]*)&nbsp;)");
        if (rx.indexIn(html) != -1) {
            movie->setName(rx.cap(1));
        }
        rx.setPattern(R"(<h1 itemprop="name" class="">(.*)&nbsp;<span id="titleYear">)");
        if (rx.indexIn(html) != -1) {
            movie->setName(rx.cap(1));
        }
        rx.setPattern(R"(<div class="originalTitle">([^<]*)<span)");
        if (rx.indexIn(html) != -1) {
            movie->setOriginalName(rx.cap(1));
        }
    }

    if (infos.contains(MovieScraperInfos::Director)) {
        rx.setPattern(
            R"(<div class="txt-block" itemprop="director" itemscope itemtype="http://schema.org/Person">(.*)</div>)");
        QString directorsBlock;
        if (rx.indexIn(html) != -1) {
            directorsBlock = rx.cap(1);
        } else {
            // the ghost span may only exist if there are more than 2 directors
            rx.setPattern(
                R"(<div class="credit_summary_item">\n +<h4 class="inline">Directors?:</h4>(.*)(?:<span class="ghost">|</div>))");
            if (rx.indexIn(html) != -1) {
                directorsBlock = rx.cap(1);
            }
        }

        if (!directorsBlock.isEmpty()) {
            QStringList directors;
            rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
            int pos = 0;
            while ((pos = rx.indexIn(directorsBlock, pos)) != -1) {
                directors << rx.cap(1);
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
            // the ghost span may only exist if there are more than 2 writers
            rx.setPattern(
                R"(<div class="credit_summary_item">\n +<h4 class="inline">Writers?:</h4>(.*)(?:<span class="ghost">|</div>))");
            if (rx.indexIn(html) != -1) {
                writersBlock = rx.cap(1);
            }
        }

        if (!writersBlock.isEmpty()) {
            QStringList writers;
            rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
            int pos = 0;
            while ((pos = rx.indexIn(writersBlock, pos)) != -1) {
                writers << rx.cap(1);
                pos += rx.matchedLength();
            }
            movie->setWriter(writers.join(", "));
        }
    }

    rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Genres:</h4>(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Genres) && rx.indexIn(html) != -1) {
        QString genres = rx.cap(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(genres, pos)) != -1) {
            movie->addGenre(helper::mapGenre(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern(R"(<div class="txt-block">[^<]*<h4 class="inline">Taglines:</h4>(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Tagline) && rx.indexIn(html) != -1) {
        QString tagline = rx.cap(1);
        QRegExp rxMore("<span class=\"see-more inline\">.*</span>");
        rxMore.setMinimal(true);
        tagline.remove(rxMore);
        movie->setTagline(tagline.trimmed());
    }

    rx.setPattern(R"(<div class="see-more inline canwrap">\n *<h4 class="inline">Plot Keywords:</h4>(.*)<nobr>)");
    if (!m_loadAllTags && infos.contains(MovieScraperInfos::Tags) && rx.indexIn(html) != -1) {
        QString tags = rx.cap(1);
        rx.setPattern(R"(<span class="itemprop">([^<]*)</span>)");
        int pos = 0;
        while ((pos = rx.indexIn(tags, pos)) != -1) {
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
            rx.setPattern(R"(<h4 class="inline">Release Date:</h4> ([0-9]+) ([A-z]*) ([0-9]{4}))");
            if (rx.indexIn(html) != -1) {
                int day = rx.cap(1).trimmed().toInt();
                int month = -1;
                QString monthName = rx.cap(2).trimmed();
                int year = rx.cap(3).trimmed().toInt();
                if (monthName.contains("January", Qt::CaseInsensitive)) {
                    month = 1;
                } else if (monthName.contains("February", Qt::CaseInsensitive)) {
                    month = 2;
                } else if (monthName.contains("March", Qt::CaseInsensitive)) {
                    month = 3;
                } else if (monthName.contains("April", Qt::CaseInsensitive)) {
                    month = 4;
                } else if (monthName.contains("May", Qt::CaseInsensitive)) {
                    month = 5;
                } else if (monthName.contains("June", Qt::CaseInsensitive)) {
                    month = 6;
                } else if (monthName.contains("July", Qt::CaseInsensitive)) {
                    month = 7;
                } else if (monthName.contains("August", Qt::CaseInsensitive)) {
                    month = 8;
                } else if (monthName.contains("September", Qt::CaseInsensitive)) {
                    month = 9;
                } else if (monthName.contains("October", Qt::CaseInsensitive)) {
                    month = 10;
                } else if (monthName.contains("November", Qt::CaseInsensitive)) {
                    month = 11;
                } else if (monthName.contains("December", Qt::CaseInsensitive)) {
                    month = 12;
                }

                if (day != 0 && month != -1 && year != 0) {
                    movie->setReleased(QDate(year, month, day));
                }
            }
        }
    }


    rx.setPattern(R"rx("contentRating": "([^"]*)",)rx");
    if (infos.contains(MovieScraperInfos::Certification) && rx.indexIn(html) != -1) {
        movie->setCertification(helper::mapCertification(Certification(rx.cap(1))));
    }

    rx.setPattern(R"("duration": "PT([0-9]+)H?([0-9]+)M")");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        if (rx.captureCount() > 1) {
            minutes runtime = hours(rx.cap(1).toInt()) + minutes(rx.cap(2).toInt());
            movie->setRuntime(runtime);
        } else {
            minutes runtime = minutes(rx.cap(1).toInt());
            movie->setRuntime(runtime);
        }
    }

    rx.setPattern(R"(<h4 class="inline">Runtime:</h4>[^<]*<time datetime="PT([0-9]+)M">)");
    if (infos.contains(MovieScraperInfos::Runtime) && rx.indexIn(html) != -1) {
        movie->setRuntime(minutes(rx.cap(1).toInt()));
    }

    rx.setPattern("<p itemprop=\"description\">(.*)</p>");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        QString outline = rx.cap(1).remove(QRegExp("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        movie->setOutline(outline);
    }

    rx.setPattern(R"(<div class="summary_text">(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        QString outline = rx.cap(1).remove(QRegExp("<[^>]*>"));
        outline = outline.remove("See full summary&nbsp;&raquo;").trimmed();
        movie->setOutline(outline);
    }

    rx.setPattern(R"(<h2>Storyline</h2>\n +\n +<div class="inline canwrap">\n +<p>\n +<span>(.*)</span>)");
    if (infos.contains(MovieScraperInfos::Overview) && rx.indexIn(html) != -1) {
        QString overview = rx.cap(1).trimmed();
        overview.remove(QRegExp("<[^>]*>"));
        movie->setOverview(overview.trimmed());
    }

    if (infos.contains(MovieScraperInfos::Rating)) {
        Rating rating;
        rating.source = "imdb";
        rating.maxRating = 10;
        rx.setPattern("<div class=\"star-box-details\" itemtype=\"http://schema.org/AggregateRating\" itemscope "
                      "itemprop=\"aggregateRating\">(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString content = rx.cap(1);
            rx.setPattern("<span itemprop=\"ratingValue\">(.*)</span>");
            if (rx.indexIn(content) != -1) {
                rating.rating = rx.cap(1).trimmed().replace(",", ".").toDouble();
            }

            rx.setPattern("<span itemprop=\"ratingCount\">(.*)</span>");
            if (rx.indexIn(content) != -1) {
                rating.voteCount = rx.cap(1).replace(",", "").replace(".", "").toInt();
            }
        } else {
            rx.setPattern(R"(<div class="imdbRating"[^>]*>\n +<div class="ratingValue">(.*)</div>)");
            if (rx.indexIn(html) != -1) {
                QString content = rx.cap(1);
                rx.setPattern("([0-9]\\.[0-9]) based on ([0-9\\,]*) ");
                if (rx.indexIn(content) != -1) {
                    rating.rating = rx.cap(1).trimmed().replace(",", ".").toDouble();
                    rating.voteCount = rx.cap(2).replace(",", "").replace(".", "").toInt();
                }
                rx.setPattern("([0-9]\\,[0-9]) based on ([0-9\\.]*) ");
                if (rx.indexIn(content) != -1) {
                    rating.rating = rx.cap(1).trimmed().replace(",", ".").toDouble();
                    rating.voteCount = rx.cap(2).replace(",", "").replace(".", "").toInt();
                }
            }
        }

        movie->ratings().push_back(rating);

        // Top250 for movies
        rx.setPattern("Top Rated Movies #([0-9]+)\\n</a>");
        if (rx.indexIn(html) != -1) {
            movie->setTop250(rx.cap(1).toInt());
        }
        // Top250 for TV shows (used by TheTvDb)
        rx.setPattern("Top Rated TV #([0-9]+)\\n</a>");
        if (rx.indexIn(html) != -1) {
            movie->setTop250(rx.cap(1).toInt());
        }
    }

    rx.setPattern(R"(<h4 class="inline">Production Co:</h4>(.*)<span class="see-more inline">)");
    if (infos.contains(MovieScraperInfos::Studios) && rx.indexIn(html) != -1) {
        QString studios = rx.cap(1);
        rx.setPattern(R"(<a href="/company/[^"]*"[^>]*>([^<]+)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(studios, pos)) != -1) {
            movie->addStudio(helper::mapStudio(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern(R"(<h4 class="inline">Country:</h4>(.*)</div>)");
    if (infos.contains(MovieScraperInfos::Countries) && rx.indexIn(html) != -1) {
        QString content = rx.cap(1);
        rx.setPattern(R"(<a href="[^"]*"[^>]*>([^<]*)</a>)");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            movie->addCountry(helper::mapCountry(rx.cap(1).trimmed()));
            pos += rx.matchedLength();
        }
    }

    rx.setPattern("<table class=\"cast_list\">(.*)</table>");
    if (infos.contains(MovieScraperInfos::Actors) && rx.indexIn(html) != -1) {
        // clear actors
        movie->setActors({});

        QString content = rx.cap(1);
        rx.setPattern(R"(<tr class="[^"]*">(.*)</tr>)");
        int pos = 0;
        while ((pos = rx.indexIn(content, pos)) != -1) {
            QString actor = rx.cap(1);
            pos += rx.matchedLength();

            Actor a;

            QRegExp rxName(R"(<a href="/name/[^"]+"\n *>([^<]*)</a>)");
            rxName.setMinimal(true);
            if (rxName.indexIn(actor) != -1) {
                a.name = rxName.cap(1).trimmed();
            }

            QRegExp rxRole(R"(<td class="character">\n *(.*)</td>)");
            rxRole.setMinimal(true);
            if (rxRole.indexIn(actor) != -1) {
                QString role = rxRole.cap(1);
                rxRole.setPattern(R"(<a href="[^"]*" >([^<]*)</a>)");
                if (rxRole.indexIn(role) != -1) {
                    role = rxRole.cap(1);
                }
                a.role = role.trimmed().replace(QRegExp("[\\s\\n]+"), " ");
            }

            QRegExp rxImg("<img [^<]*loadlate=\"([^\"]*)\"[^<]* />");
            rxImg.setMinimal(true);
            if (rxImg.indexIn(actor) != -1) {
                QString img = rxImg.cap(1);
                QRegExp aRx1("https://ia.media-imdb.com/images/(.*)/(.*)._V(.*).jpg");
                aRx1.setMinimal(true);
                if (aRx1.indexIn(img) != -1) {
                    a.thumb = "https://ia.media-imdb.com/images/" + aRx1.cap(1) + "/" + aRx1.cap(2) + ".jpg";
                } else {
                    a.thumb = rxImg.cap(1);
                }
            }

            movie->addActor(a);
        }
    }
}

QUrl IMDB::parsePosters(QString html)
{
    QRegExp rx("<div class=\"poster\">(.*)</div>");
    rx.setMinimal(true);
    if (rx.indexIn(html) == -1) {
        return QUrl();
    }

    QString content = rx.cap(1);
    rx.setPattern("<a href=\"/title/tt([^\"]*)\"[^>]*>");
    if (rx.indexIn(content) == -1) {
        return QUrl();
    }

    return QString("https://www.imdb.com/title/tt%1").arg(rx.cap(1));
}

void IMDB::parseAndAssignTags(const QString& html, Movie& movie)
{
    QRegExp rx;
    rx.setMinimal(true);
    if (m_loadAllTags) {
        rx.setPattern(R"(<a href="/search/keyword[^"]+"\n?>([^<]+)</a>)");
    } else {
        rx.setPattern(R"(<a href="/keyword/[^"]+"[^>]*>([^<]+)</a>)");
    }

    int pos = 0;
    while ((pos = rx.indexIn(html, pos)) != -1) {
        movie.addTag(rx.cap(1).trimmed());
        pos += rx.matchedLength();
    }
}

void IMDB::parseAndAssignPoster(const QString& html, QString posterId, Movie* movie)
{
    // IMDB's media viewer contains all links to the gallery's image files.
    // We only want the poster, which has the given id.
    //
    // Relevant JavaScript example:
    //   "id":"rm2278496512","h":1000,"msrc":"https://m.media-amazon.com/images/M/<image>.jpg",
    //   "src":"https://m.media-amazon.com/images/M/<image>.jpg",
    //
    QString regex = QStringLiteral(R"url("id":"%1","h":[0-9]+,"msrc":"([^"]+)","src":"([^"]+)")url");
    QRegExp rx(regex.arg(posterId));
    rx.setMinimal(true);

    if (rx.indexIn(html) != -1) {
        Poster p;
        p.thumbUrl = rx.cap(1);
        p.originalUrl = rx.cap(2);
        movie->images().addPoster(p);
    }
}
