#include "TheTvDb.h"

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "globals/NetworkReplyWatcher.h"
#include "media_centers/KodiXml.h"
#include "movies/Movie.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/tv_show/TheTvDb/Cache.h"
#include "scrapers/tv_show/TheTvDb/EpisodeLoader.h"
#include "scrapers/tv_show/TheTvDb/Search.h"
#include "scrapers/tv_show/TheTvDb/ShowLoader.h"
#include "settings/Settings.h"
#include "ui/main/MainWindow.h"

#include <QComboBox>
#include <QDomDocument>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSpacerItem>
#include <chrono>

/**
 * @brief TheTvDb scraper. Uses the "TheTVDB API v2". See https://api.thetvdb.com/swagger
 * @param parent Parent QObject that owns the instance.
 */
TheTvDb::TheTvDb(QObject* parent) :
    m_widget{new QWidget(MainWindow::instance())}, m_imdb{new IMDB(this)}, m_dummyMovie{new Movie(QStringList(), this)}
{
    setParent(parent);

    setupLanguages();
    setupLayout();

    m_movieInfos = {MovieScraperInfos::Title,
        MovieScraperInfos::Rating,
        MovieScraperInfos::Released,
        MovieScraperInfos::Runtime,
        MovieScraperInfos::Director,
        MovieScraperInfos::Writer,
        MovieScraperInfos::Certification,
        MovieScraperInfos::Overview,
        MovieScraperInfos::Tags,
        MovieScraperInfos::Genres,
        MovieScraperInfos::Actors};

    connect(m_dummyMovie->controller(), &MovieController::sigLoadDone, [=]() {
        QMessageBox::information(nullptr, "Test", "Test");
    });
}

void TheTvDb::loadSettings(const ScraperSettings& settings)
{
    m_language = settings.language("en");
    for (int i = 0, n = m_languageComboBox->count(); i < n; ++i) {
        if (m_languageComboBox->itemData(i).toString() == m_language) {
            m_languageComboBox->setCurrentIndex(i);
        }
    }
}

void TheTvDb::saveSettings(ScraperSettings& settings)
{
    m_language = m_languageComboBox->itemData(m_languageComboBox->currentIndex()).toString();
    settings.setLanguage(m_language);
}

void TheTvDb::search(QString searchStr)
{
    qInfo() << "[TheTvDb] Search for:" << searchStr;

    auto* search = new thetvdb::Search(m_language, this);
    connect(search, &thetvdb::Search::sigSearchDone, this, [this, search](QVector<ScraperSearchResult> results) {
        search->deleteLater();
        emit sigSearchDone(results);
    });
    search->search(searchStr);
}

void TheTvDb::loadTvShowData(TvDbId tvDbId,
    TvShow* show,
    TvShowUpdateType updateType,
    QVector<TvShowScraperInfos> infosToLoad)
{
    qInfo() << "[TheTvDb] Load TV show with id:" << tvDbId.toString();

    // todo: use reference for show
    if (show == nullptr) {
        qWarning() << "[TheTvDb] Tried to load nullptr TvShow!";
        return;
    }

    // Default: Load only basic show information that is neccessary for episode scraping (IMDb ID,...)
    // If the user wants to update show information, set showInfosToLoad.
    QVector<TvShowScraperInfos> showInfosToLoad = {};
    if (isShowUpdateType(updateType)) {
        show->clear(infosToLoad);
        showInfosToLoad = infosToLoad;
    }
    QVector<TvShowScraperInfos> episodeInfosToLoad = {};
    if (isEpisodeUpdateType(updateType)) {
        episodeInfosToLoad = infosToLoad;
    }

    // Load the show and update database with episodes
    show->setTvdbId(tvDbId);
    auto* loader = new thetvdb::ShowLoader(*show, m_language, showInfosToLoad, episodeInfosToLoad, updateType, this);
    connect(loader, &thetvdb::ShowLoader::sigLoadDone, this, [=]() {
        qDebug() << "[TheTvDb] TV show with ID" << tvDbId.toString() << "loaded";
        loader->storeEpisodesInDatabase();
        // episodes which exist in the database and were updated with TheTvDb data
        const auto updatedEpisodes = loader->mergeEpisodesToShow();
        loadShowFromImdb(*show, showInfosToLoad, updateType, updatedEpisodes);
        loader->deleteLater();
    });
    loader->loadShowAndEpisodes();
}

void TheTvDb::loadShowFromImdb(TvShow& show,
    const QVector<TvShowScraperInfos>& infosToLoad,
    TvShowUpdateType updateType,
    QVector<TvShowEpisode*> episodesToLoad)
{
    if (!shouldLoadImdb(infosToLoad) || !show.imdbId().isValid()) {
        show.scraperLoadDone();
        return;
    }

    qDebug() << "[TheTvDb] Load IMDb TVShow with id:" << show.imdbId();

    const QUrl url(QStringLiteral("https://www.imdb.com/title/%1/").arg(show.imdbId().toString()));
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Accept-Language", "en;q=0.8");

    QNetworkReply* reply = m_qnam.get(request);
    new NetworkReplyWatcher(this, reply);

    connect(reply, &QNetworkReply::finished, this, [=, &show]() {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError) {
            showNetworkError(*reply);
            qWarning() << "[TheTvDb] Network Error (load imdb):" << reply->errorString();
            show.scraperLoadDone(); // avoid endless "loading..." message in case of an error
            return;
        }

        const QString html = QString::fromUtf8(reply->readAll());
        parseAndAssignImdbInfos(html, show, updateType, infosToLoad);

        // Can't load episodes from IMDb without an IMDb id...
        if (!show.imdbId().isValid()) {
            show.scraperLoadDone();
            return;
        }

        show.setProperty("episodesToLoadCount", episodesToLoad.count());
        loadEpisodesFromImdb(show, episodesToLoad, infosToLoad);
    });
}

void TheTvDb::loadEpisodesFromImdb(TvShow& show,
    QVector<TvShowEpisode*> episodes,
    QVector<TvShowScraperInfos> infosToLoad)
{
    if (episodes.isEmpty()) {
        show.scraperLoadDone();
        return;
    }

    const int episodesToLoadCount = show.property("episodesToLoadCount").toInt();
    emit sigLoadProgress(&show, episodesToLoadCount - episodes.count(), episodesToLoadCount);

    TvShowEpisode* episode = episodes.takeFirst();

    const auto loadImdbEpisode = [&](ImdbId episodeImdbId) {
        QUrl url(QStringLiteral("https://www.imdb.com/title/%1/").arg(episodeImdbId.toString()));

        QNetworkRequest request = QNetworkRequest(url);
        request.setRawHeader("Accept-Language", "en;q=0.8");

        QNetworkReply* reply = m_qnam.get(request);
        reply->setProperty("storage", Storage::toVariant(reply, episode));
        reply->setProperty("show", Storage::toVariant(reply, &show));
        reply->setProperty("episodes", Storage::toVariant(reply, episodes));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infosToLoad));

        new NetworkReplyWatcher(this, reply);
        connect(reply, &QNetworkReply::finished, this, &TheTvDb::onEpisodesImdbEpisodeLoaded);
    };

    if (episode->imdbId().isValid()) {
        loadImdbEpisode(episode->imdbId());
        return;
    }

    // In case that we don't have the episode's IMDb id, load the season page.
    QUrl url(QStringLiteral("https://www.imdb.com/title/%1/episodes?season=%2")
                 .arg(episode->tvShow()->imdbId().toString())
                 .arg(episode->property("airedSeason").toInt()));

    if (thetvdb::hasValidCacheElement(url)) {
        const EpisodeNumber airedEpisode = EpisodeNumber(episode->property("airedEpisode").toInt());
        const ImdbId imdbId = getImdbIdForEpisode(thetvdb::getCacheElement(url), airedEpisode);
        if (imdbId.isValid()) {
            loadImdbEpisode(imdbId);
            return;
        }
    }

    QNetworkRequest request{url};
    request.setRawHeader("Accept-Language", "en;q=0.8");

    QNetworkReply* reply = m_qnam.get(request);
    reply->setProperty("storage", Storage::toVariant(reply, episode));
    reply->setProperty("show", Storage::toVariant(reply, &show));
    reply->setProperty("episodes", Storage::toVariant(reply, episodes));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infosToLoad));

    new NetworkReplyWatcher(this, reply);
    connect(reply, &QNetworkReply::finished, this, &TheTvDb::onEpisodesImdbSeasonLoaded);
}


void TheTvDb::loadTvShowEpisodeData(TvDbId tvDbId, TvShowEpisode* episode, QVector<TvShowScraperInfos> infosToLoad)
{
    qDebug() << "[TheTvDb] Load single episode of TV show with ID:" << tvDbId.toString();
    episode->clear(infosToLoad);

    auto* api = new thetvdb::EpisodeLoader(tvDbId, *episode, m_language, infosToLoad, this);
    connect(api, &thetvdb::EpisodeLoader::sigLoadDone, this, [=]() {
        qDebug() << "[TheTvDb] Single episode scraper done";
        api->deleteLater();
        episode->scraperLoadDone();
    });
    api->loadData();
}

void TheTvDb::setupLanguages()
{
    m_languageComboBox = new QComboBox(m_widget);
    m_languageComboBox->addItem(tr("Bulgarian"), "bg");
    m_languageComboBox->addItem(tr("Chinese"), "zh");
    m_languageComboBox->addItem(tr("Croatian"), "hr");
    m_languageComboBox->addItem(tr("Czech"), "cs");
    m_languageComboBox->addItem(tr("Danish"), "da");
    m_languageComboBox->addItem(tr("Dutch"), "nl");
    m_languageComboBox->addItem(tr("English"), "en");
    m_languageComboBox->addItem(tr("Finnish"), "fi");
    m_languageComboBox->addItem(tr("French"), "fr");
    m_languageComboBox->addItem(tr("German"), "de");
    m_languageComboBox->addItem(tr("Greek"), "el");
    m_languageComboBox->addItem(tr("Hebrew"), "he");
    m_languageComboBox->addItem(tr("Hungarian"), "hu");
    m_languageComboBox->addItem(tr("Italian"), "it");
    m_languageComboBox->addItem(tr("Japanese"), "ja");
    m_languageComboBox->addItem(tr("Korean"), "ko");
    m_languageComboBox->addItem(tr("Norwegian"), "no");
    m_languageComboBox->addItem(tr("Polish"), "pl");
    m_languageComboBox->addItem(tr("Portuguese"), "pt");
    m_languageComboBox->addItem(tr("Russian"), "ru");
    m_languageComboBox->addItem(tr("Slovene"), "sl");
    m_languageComboBox->addItem(tr("Spanish"), "es");
    m_languageComboBox->addItem(tr("Swedish"), "sv");
    m_languageComboBox->addItem(tr("Turkish"), "tr");
}

void TheTvDb::setupLayout()
{
    auto layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_languageComboBox, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

void TheTvDb::parseAndAssignImdbInfos(const QString& html,
    TvShow& show,
    TvShowUpdateType updateType,
    QVector<TvShowScraperInfos> infosToLoad)
{
    m_dummyMovie->clear();
    m_imdb->parseAndAssignInfos(html, m_dummyMovie, m_movieInfos);

    if (!isShowUpdateType(updateType)) {
        return;
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Title, infosToLoad) && !m_dummyMovie->name().isEmpty()) {
        show.setName(m_dummyMovie->name());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Rating, infosToLoad)) {
        Rating movieRating = m_dummyMovie->ratings().first();
        movieRating.source = "imdb";

        if (movieRating.rating >= 0 || movieRating.voteCount != 0) {
            // @todo currently only one rating is supported
            show.ratings().clear();
            show.ratings().push_back(movieRating);
        }

        if (m_dummyMovie->top250() != 0) {
            show.setTop250(m_dummyMovie->top250());
        }
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::FirstAired, infosToLoad) && m_dummyMovie->released().isValid()) {
        show.setFirstAired(m_dummyMovie->released());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Runtime, infosToLoad) && m_dummyMovie->runtime().count() != 0) {
        show.setRuntime(m_dummyMovie->runtime());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Certification, infosToLoad) && m_dummyMovie->certification().isValid()) {
        show.setCertification(m_dummyMovie->certification());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Overview, infosToLoad) && !m_dummyMovie->overview().isEmpty()) {
        show.setOverview(m_dummyMovie->overview());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Genres, infosToLoad) && !m_dummyMovie->genres().isEmpty()) {
        show.clear({TvShowScraperInfos::Genres});
        for (const QString& genre : m_dummyMovie->genres()) {
            show.addGenre(helper::mapGenre(genre));
        }
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Tags, infosToLoad) && !m_dummyMovie->tags().isEmpty()) {
        show.clear({TvShowScraperInfos::Tags});
        for (const QString& tag : m_dummyMovie->tags()) {
            show.addTag(tag);
        }
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Actors, infosToLoad) && !m_dummyMovie->actors().isEmpty()) {
        show.clear({TvShowScraperInfos::Actors});
        for (const Actor* actor : m_dummyMovie->actors()) {
            show.addActor(*actor);
        }
    }
}

/// Loads *all* episodes for a given show from TheTvDb. This is used for filling
/// missing episodes.
void TheTvDb::fillDatabaseWithAllEpisodes(TvShow& show, std::function<void()> callback)
{
    QVector<TvShowScraperInfos> episodeInfos{TvShowScraperInfos::Director,
        TvShowScraperInfos::Title,
        TvShowScraperInfos::FirstAired,
        TvShowScraperInfos::Overview,
        TvShowScraperInfos::Rating,
        TvShowScraperInfos::Writer,
        TvShowScraperInfos::Thumbnail};

    const TvDbId id = show.tvdbId();
    auto* loader = new thetvdb::ShowLoader(show, m_language, {}, episodeInfos, TvShowUpdateType::AllEpisodes, this);

    connect(loader, &thetvdb::ShowLoader::sigLoadDone, this, [=]() {
        qDebug() << "[TheTvDb] All episodes with show ID" << id.toString() << "loaded";
        loader->storeEpisodesInDatabase();
        loader->deleteLater();
        callback();
    });

    loader->loadShowAndEpisodes();
}

void TheTvDb::parseAndAssignImdbInfos(const QString& html,
    TvShowEpisode& episode,
    QVector<TvShowScraperInfos> infosToLoad)
{
    m_dummyMovie->clear();
    m_imdb->parseAndAssignInfos(html, m_dummyMovie, m_movieInfos);

    if (shouldLoadFromImdb(TvShowScraperInfos::Title, infosToLoad) && !m_dummyMovie->name().isEmpty()) {
        episode.setName(m_dummyMovie->name());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Rating, infosToLoad)) {
        if (!m_dummyMovie->ratings().empty()) {
            Rating movieRating = m_dummyMovie->ratings().first();
            if (movieRating.rating >= 0 || movieRating.voteCount != 0) {
                Rating rating;
                rating.rating = movieRating.rating;
                rating.voteCount = movieRating.voteCount;
                rating.maxRating = 10;
                rating.minRating = 0;
                rating.source = "imdb";
                // @todo currently only one rating is supported
                episode.ratings().clear();
                episode.ratings().push_back(rating);
            }
        }
        if (m_dummyMovie->top250() != 0) {
            episode.setTop250(m_dummyMovie->top250());
        }
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::FirstAired, infosToLoad) && m_dummyMovie->released().isValid()) {
        episode.setFirstAired(m_dummyMovie->released());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Certification, infosToLoad) && m_dummyMovie->certification().isValid()) {
        episode.setCertification(m_dummyMovie->certification());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Overview, infosToLoad) && !m_dummyMovie->overview().isEmpty()) {
        episode.setOverview(m_dummyMovie->overview());
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Director, infosToLoad) && !m_dummyMovie->director().isEmpty()) {
        episode.setDirectors(m_dummyMovie->director().split(", "));
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Writer, infosToLoad) && !m_dummyMovie->writer().isEmpty()) {
        episode.setWriters(m_dummyMovie->writer().split(", "));
    }

    if (shouldLoadFromImdb(TvShowScraperInfos::Actors, infosToLoad) && !m_dummyMovie->actors().isEmpty()) {
        episode.clear(QVector<TvShowScraperInfos>() << TvShowScraperInfos::Actors);
        for (const auto* actor : m_dummyMovie->actors()) {
            Actor a;
            a.id = actor->id;
            a.image = actor->image;
            a.imageHasChanged = actor->imageHasChanged;
            a.name = actor->name;
            a.role = actor->role;
            a.thumb = actor->thumb;
            episode.addActor(std::move(a));
        }
    }
}

void TheTvDb::onImdbSeasonLoaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    TvShowEpisode* episode = reply->property("storage").value<Storage*>()->episode();
    QVector<TvShowScraperInfos> infos = reply->property("infosToLoad").value<Storage*>()->showInfosToLoad();
    EpisodeNumber episodeNumber(reply->property("episodeNumber").toInt());

    if (episode == nullptr) {
        qWarning() << "[TheTvDb] Couldn't get episode* from storage";
        episode->scraperLoadDone();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        showNetworkError(*reply);
        qWarning() << "[TheTvDb] Network Error (load)" << reply->errorString();
        episode->scraperLoadDone();
        return;
    }

    const QString data = QString::fromUtf8(reply->readAll());
    thetvdb::addCacheElement(reply->url(), data);

    const ImdbId imdbId = getImdbIdForEpisode(data, episodeNumber);
    if (!imdbId.isValid()) {
        episode->scraperLoadDone();
        return;
    }

    if (episode->imdbId() == ImdbId::NoId) {
        episode->setImdbId(imdbId);
    }

    qDebug() << "[TheTvDb] Now loading IMDb entry for" << imdbId;

    QUrl url(QStringLiteral("https://www.imdb.com/title/%1/").arg(imdbId.toString()));
    QNetworkRequest request = QNetworkRequest(url);
    request.setRawHeader("Accept-Language", "en;q=0.8");

    QNetworkReply* imdbReply = m_qnam.get(request);
    new NetworkReplyWatcher(this, imdbReply);
    imdbReply->setProperty("storage", Storage::toVariant(imdbReply, episode));
    imdbReply->setProperty("infosToLoad", Storage::toVariant(imdbReply, infos));

    connect(imdbReply, &QNetworkReply::finished, this, &TheTvDb::onImdbEpisodeLoaded);
}


void TheTvDb::onEpisodesImdbSeasonLoaded()
{
    using namespace std::chrono_literals;

    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    TvShowEpisode* episode = reply->property("storage").value<Storage*>()->episode();
    const QVector<TvShowScraperInfos> infosToLoad = reply->property("infosToLoad").value<Storage*>()->showInfosToLoad();
    const QVector<TvShowEpisode*> episodes = reply->property("episodes").value<Storage*>()->episodes();
    TvShow* show = reply->property("show").value<Storage*>()->show();

    if (episode == nullptr) {
        qWarning() << "[TheTvDb] Couldn't get episode* from storage";
        show->scraperLoadDone();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        showNetworkError(*reply);
        qWarning() << "[TheTvDb] Network Error (load):" << reply->errorString();
        show->scraperLoadDone();
        return;
    }

    const QString html = QString::fromUtf8(reply->readAll());
    thetvdb::addCacheElement(reply->url(), html);

    const EpisodeNumber num(episode->property("airedEpisode").toInt());
    const ImdbId imdbId = getImdbIdForEpisode(html, num);

    if (imdbId.isValid()) {
        qDebug() << "[TheTvDb] Now loading IMDb entry for:" << imdbId;

        QUrl url = QUrl(QString("https://www.imdb.com/title/%1/").arg(imdbId.toString()));
        QNetworkRequest request = QNetworkRequest(url);
        request.setRawHeader("Accept-Language", "en;q=0.8");

        QNetworkReply* episodeImdbReply = m_qnam.get(request);
        new NetworkReplyWatcher(this, episodeImdbReply);
        episodeImdbReply->setProperty("storage", Storage::toVariant(episodeImdbReply, episode));
        episodeImdbReply->setProperty("show", Storage::toVariant(episodeImdbReply, show));
        episodeImdbReply->setProperty("episodes", Storage::toVariant(episodeImdbReply, episodes));
        episodeImdbReply->setProperty("infosToLoad", Storage::toVariant(episodeImdbReply, infosToLoad));
        connect(episodeImdbReply, &QNetworkReply::finished, this, &TheTvDb::onEpisodesImdbEpisodeLoaded);
        return;
    }

    loadEpisodesFromImdb(*show, episodes, infosToLoad);
}

void TheTvDb::onImdbEpisodeLoaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();

    TvShowEpisode* episode = reply->property("storage").value<Storage*>()->episode();
    const QVector<TvShowScraperInfos> infosToLoad = reply->property("infosToLoad").value<Storage*>()->showInfosToLoad();

    if (episode == nullptr) {
        qWarning() << "[TheTvDb] Couldn't get episode* from storage";
        episode->scraperLoadDone();
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        showNetworkError(*reply);
        qWarning() << "[TheTvDb] Network Error (load imdb episode):" << reply->errorString();
        episode->scraperLoadDone();
        return;
    }

    const QString html = QString::fromUtf8(reply->readAll());
    parseAndAssignImdbInfos(html, *episode, infosToLoad);
    episode->scraperLoadDone();
}


void TheTvDb::onEpisodesImdbEpisodeLoaded()
{
    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    TvShowEpisode* episode = reply->property("storage").value<Storage*>()->episode();
    QVector<TvShowScraperInfos> infos = reply->property("infosToLoad").value<Storage*>()->showInfosToLoad();
    QVector<TvShowEpisode*> episodes = reply->property("episodes").value<Storage*>()->episodes();
    TvShow* show = reply->property("show").value<Storage*>()->show();

    if (episode == nullptr) {
        qCritical() << "[TheTvDb] Couldn't get episode* from storage";
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        const QString data = QString::fromUtf8(reply->readAll());
        parseAndAssignImdbInfos(data, *episode, infos);

    } else {
        showNetworkError(*reply);
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    loadEpisodesFromImdb(*show, episodes, infos);
}

ImdbId TheTvDb::getImdbIdForEpisode(const QString& html, EpisodeNumber episodeNumber)
{
    QRegExp rx("<a href=\"/title/(tt[0-9]*)/\\?ref_=ttep_ep" + episodeNumber.toString() + "\"");
    rx.setMinimal(true);
    if (rx.indexIn(html) != -1 && ImdbId::isValidFormat(rx.cap(1))) {
        return ImdbId(rx.cap(1));
    }
    return ImdbId::NoId;
}

bool TheTvDb::shouldLoadImdb(QVector<TvShowScraperInfos> infosToLoad) const
{
    QMap<TvShowScraperInfos, QString> scraperSettings = Settings::instance()->customTvScraper();
    for (const TvShowScraperInfos info : infosToLoad) {
        if (scraperSettings.value(info) == IMDB::scraperIdentifier) {
            return true;
        }
    }
    return false;
}

bool TheTvDb::shouldLoadFromImdb(TvShowScraperInfos info, QVector<TvShowScraperInfos> infosToLoad)
{
    QMap<TvShowScraperInfos, QString> scraperSettings = Settings::instance()->customTvScraper();
    return infosToLoad.contains(info) && scraperSettings.value(info) == IMDB::scraperIdentifier;
}
