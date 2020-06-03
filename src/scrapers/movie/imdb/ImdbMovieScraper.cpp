#include "scrapers/movie/imdb/ImdbMovieScraper.h"

#include "globals/Helper.h"
#include "network/NetworkRequest.h"
#include "scrapers/movie/IMDB.h"

void ImdbMovieLoader::load()
{
    m_movie.clear(m_infos);
    m_movie.setId(ImdbId(m_imdbId));

    QUrl url = QUrl(QString("https://www.imdb.com/title/%1/").arg(m_imdbId).toUtf8());
    QNetworkRequest request = mediaelch::network::requestWithDefaults(url);
    request.setRawHeader("Accept-Language", "en");
    QNetworkReply* reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &ImdbMovieLoader::onLoadFinished);
}

void ImdbMovieLoader::onLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[ImdbMovieLoader] onLoadFinished: reply was nullptr; Please report!";
        emit sigLoadDone(m_movie, this);
        return;
    }

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        m_scraper.showNetworkError(*reply);
        qWarning() << "Network Error (load)" << reply->errorString();
        emit sigLoadDone(m_movie, this);
        return;
    }

    QUrl posterViewerUrl;
    {
        const QString html = QString::fromUtf8(reply->readAll());
        posterViewerUrl = parsePoster(html);
        parseAndAssignInfos(html);
        parseAndStoreActors(html);
    }

    const bool shouldLoadPoster = m_infos.contains(MovieScraperInfos::Poster) && !posterViewerUrl.isEmpty();
    const bool shouldLoadTags = m_infos.contains(MovieScraperInfos::Tags) && m_loadAllTags;
    const bool shouldLoadActors = m_infos.contains(MovieScraperInfos::Actors) && !m_actorUrls.isEmpty();

    { // How many pages do we have to download? Count them.
        QMutexLocker locker(&m_mutex);
        m_itemsLeftToDownloads = 1;

        if (shouldLoadPoster) {
            ++m_itemsLeftToDownloads;
        }
        // IMDb has an extra page listing all tags (popular movies can have more than 100 tags).
        if (shouldLoadTags) {
            ++m_itemsLeftToDownloads;
        }
        if (shouldLoadActors) {
            m_itemsLeftToDownloads += m_actorUrls.size();
        }
    }

    if (shouldLoadPoster) {
        loadPoster(posterViewerUrl);
    }
    if (shouldLoadTags) {
        loadTags();
    }
    if (shouldLoadActors) {
        loadActorImageUrls();
    }
    // It's possible that none of the above items should be loaded.
    decreaseDownloadCount();
}


void ImdbMovieLoader::loadPoster(const QUrl& posterViewerUrl)
{
    qDebug() << "[ImdbMovieLoader] Loading movie poster detail view";
    auto request = mediaelch::network::requestWithDefaults(posterViewerUrl);
    QNetworkReply* posterReply = m_qnam.get(request);
    new NetworkReplyWatcher(this, posterReply);
    connect(posterReply, &QNetworkReply::finished, this, &ImdbMovieLoader::onPosterLoadFinished);
}

void ImdbMovieLoader::loadTags()
{
    QUrl tagsUrl(QStringLiteral("https://www.imdb.com/title/%1/keywords").arg(m_movie.imdbId().toString()));
    auto request = mediaelch::network::requestWithDefaults(tagsUrl);
    QNetworkReply* tagsReply = m_qnam.get(request);
    new NetworkReplyWatcher(this, tagsReply);
    connect(tagsReply, &QNetworkReply::finished, this, &ImdbMovieLoader::onTagsFinished);
}

void ImdbMovieLoader::loadActorImageUrls()
{
    for (int index = 0; index < m_actorUrls.size(); ++index) {
        auto request = mediaelch::network::requestWithDefaults(m_actorUrls[index].second);
        // The actor's image should be the same for all languages. So we can
        // just load the English version of the page.
        request.setRawHeader("Accept-Language", "en");
        QNetworkReply* reply = m_qnam.get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("actorIndex", QVariant(index));
        connect(reply, &QNetworkReply::finished, this, &ImdbMovieLoader::onActorImageUrlLoadDone);
    }
}

void ImdbMovieLoader::onPosterLoadFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[ImdbMovieLoader] onPosterLoadFinished: reply was nullptr; Please report!";
        decreaseDownloadCount();
        return;
    }
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        const QString posterId = reply->url().fileName();
        const QString html = QString::fromUtf8(reply->readAll());
        parseAndAssignPoster(html, posterId);

    } else {
        m_scraper.showNetworkError(*reply);
        qWarning() << "[ImdbMovieLoader] Network Error (load poster)" << reply->errorString();
    }
    decreaseDownloadCount();
}

void ImdbMovieLoader::onTagsFinished()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[ImdbMovieLoader] onTagsFinished: reply was nullptr; Please report!";
        decreaseDownloadCount();
        return;
    }
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        const QString html = QString::fromUtf8(reply->readAll());
        parseAndAssignTags(html);

    } else {
        m_scraper.showNetworkError(*reply);
        qWarning() << "[ImdbMovieLoader] Network Error (load tags)" << reply->errorString();
    }
    decreaseDownloadCount();
}

void ImdbMovieLoader::onActorImageUrlLoadDone()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    if (reply == nullptr) {
        qCritical() << "[ImdbMovieLoader] onActorImageUrlLoadDone: reply was nullptr; Please report!";
        decreaseDownloadCount();
        return;
    }
    reply->deleteLater();

    bool ok = false;
    const int actorIndex = reply->property("actorIndex").toInt(&ok);

    if (!ok) {
        qCritical() << "[ImdbMovieLoader] onActorImageUrlLoadDone: Cannot get actor index; Please report!";
        decreaseDownloadCount();
        return;
    }

    if (actorIndex < 0 || actorIndex >= m_actorUrls.size()) {
        qCritical() << "[ImdbMovieLoader] onActorImageUrlLoadDone: Actor index out of bounds; Please report!";
        decreaseDownloadCount();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        m_scraper.showNetworkError(*reply);
        qWarning() << "[ImdbMovieLoader] Network Error (load poster)" << reply->errorString();
        decreaseDownloadCount();
        return;
    }

    const QString html = QString::fromUtf8(reply->readAll());
    QString url = parseActorImageUrl(html);
    if (!url.isEmpty()) {
        m_actorUrls[actorIndex].first.thumb = url;
    }
    decreaseDownloadCount();
}

void ImdbMovieLoader::parseAndAssignInfos(const QString& html)
{
    m_scraper.parseAndAssignInfos(html, &m_movie, m_infos);
}

void ImdbMovieLoader::parseAndStoreActors(const QString& html)
{
    QRegExp rx;
    rx.setMinimal(true);
    rx.setPattern("<table class=\"cast_list\">(.*)</table>");
    if (rx.indexIn(html) == -1) {
        return;
    }

    QString content = rx.cap(1);
    rx.setPattern(R"(<tr class="[^"]*">(.*)</tr>)");
    int pos = 0;
    while ((pos = rx.indexIn(content, pos)) != -1) {
        QString actorHtml = rx.cap(1);
        pos += rx.matchedLength();

        QPair<Actor, QUrl> actorUrl;

        QRegExp rxName(R"re(<a href="(/name/[^"]+)"\n *>([^<]*)</a>)re");
        rxName.setMinimal(true);
        if (rxName.indexIn(actorHtml) != -1) {
            actorUrl.second = QUrl("https://www.imdb.com" + rxName.cap(1));
            actorUrl.first.name = rxName.cap(2).trimmed();
        }

        QRegExp rxRole(R"(<td class="character">\n *(.*)</td>)");
        rxRole.setMinimal(true);
        if (rxRole.indexIn(actorHtml) != -1) {
            QString role = rxRole.cap(1);
            rxRole.setPattern(R"(<a href="[^"]*" >([^<]*)</a>)");
            if (rxRole.indexIn(role) != -1) {
                role = rxRole.cap(1);
            }
            actorUrl.first.role = role.trimmed().replace(QRegExp("[\\s\\n]+"), " ");
        }

        QRegExp rxImg("<img [^<]*loadlate=\"([^\"]*)\"[^<]* />");
        rxImg.setMinimal(true);
        if (rxImg.indexIn(actorHtml) != -1) {
            QString img = rxImg.cap(1);
            QRegExp aRx1("https://ia.media-imdb.com/images/(.*)/(.*)._V(.*).jpg");
            aRx1.setMinimal(true);
            if (aRx1.indexIn(img) != -1) {
                actorUrl.first.thumb = "https://ia.media-imdb.com/images/" + aRx1.cap(1) + "/" + aRx1.cap(2) + ".jpg";
            } else {
                actorUrl.first.thumb = rxImg.cap(1);
            }
        }

        m_movie.addActor(actorUrl.first);
        m_actorUrls.push_back(actorUrl);
    }
}

QUrl ImdbMovieLoader::parsePoster(const QString& html)
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

void ImdbMovieLoader::parseAndAssignTags(const QString& html)
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
        m_movie.addTag(rx.cap(1).trimmed());
        pos += rx.matchedLength();
    }
}

QString ImdbMovieLoader::parseActorImageUrl(const QString& html)
{
    QRegExp rx(R"re(<link rel=['"]image_src['"] href="([^"]+)">)re");
    rx.setMinimal(true);
    if (rx.indexIn(html) == -1) {
        return "";
    }

    return rx.cap(1);
}

void ImdbMovieLoader::mergeActors()
{
    // Simple brute-force merge.
    // @todo This code can most likely be simplified
    for (const auto& actorUrlPair : m_actorUrls) {
        for (Actor* actor : m_movie.actors()) {
            if (actor->name == actorUrlPair.first.name) {
                actor->thumb = actorUrlPair.first.thumb;
                break;
            }
        }
    }
}

void ImdbMovieLoader::parseAndAssignPoster(const QString& html, QString posterId)
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
        m_movie.images().addPoster(p);
    }
}

void ImdbMovieLoader::decreaseDownloadCount()
{
    QMutexLocker locker(&m_mutex);
    --m_itemsLeftToDownloads;
    if (m_itemsLeftToDownloads == 0) {
        locker.unlock();
        mergeActors();
        emit sigLoadDone(m_movie, this);
    }
}
