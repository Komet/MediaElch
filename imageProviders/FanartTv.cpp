#include "FanartTv.h"
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>
#include "data/Storage.h"
#include "main/MainWindow.h"
#include "scrapers/TMDb.h"

/**
 * @brief FanartTv::FanartTv
 * @param parent
 */
FanartTv::FanartTv(QObject *parent)
{
    setParent(parent);

    m_language = "en";
    m_preferredDiscType = "BluRay";

    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);
    m_box->addItem(tr("Bulgarian"), "bg");
    m_box->addItem(tr("Chinese"), "zh");
    m_box->addItem(tr("Croatian"), "hr");
    m_box->addItem(tr("Czech"), "cs");
    m_box->addItem(tr("Danish"), "da");
    m_box->addItem(tr("Dutch"), "nl");
    m_box->addItem(tr("English"), "en");
    m_box->addItem(tr("Finnish"), "fi");
    m_box->addItem(tr("French"), "fr");
    m_box->addItem(tr("German"), "de");
    m_box->addItem(tr("Greek"), "el");
    m_box->addItem(tr("Hebrew"), "he");
    m_box->addItem(tr("Hungarian"), "hu");
    m_box->addItem(tr("Italian"), "it");
    m_box->addItem(tr("Japanese"), "ja");
    m_box->addItem(tr("Korean"), "ko");
    m_box->addItem(tr("Norwegian"), "no");
    m_box->addItem(tr("Polish"), "pl");
    m_box->addItem(tr("Portuguese"), "pt");
    m_box->addItem(tr("Russian"), "ru");
    m_box->addItem(tr("Slovene"), "sl");
    m_box->addItem(tr("Spanish"), "es");
    m_box->addItem(tr("Swedish"), "sv");
    m_box->addItem(tr("Turkish"), "tr");
    m_discBox = new QComboBox(m_widget);
    m_discBox->addItem(tr("3D"), "3D");
    m_discBox->addItem(tr("BluRay"), "BluRay");
    m_discBox->addItem(tr("DVD"), "DVD");
    m_personalApiKeyEdit = new QLineEdit(m_widget);
    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Preferred Disc Type")), 1, 0);
    layout->addWidget(m_discBox, 1, 1);
    layout->addWidget(new QLabel(tr("Personal API key")), 2, 0);
    layout->addWidget(m_personalApiKeyEdit, 2, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    m_provides << ImageType::MovieBackdrop << ImageType::MovieLogo << ImageType::MovieClearArt << ImageType::MovieCdArt
               << ImageType::MovieBanner << ImageType::MovieThumb << ImageType::MoviePoster
               << ImageType::TvShowClearArt << ImageType::TvShowBackdrop << ImageType::TvShowBanner
               << ImageType::TvShowThumb << ImageType::TvShowSeasonThumb << ImageType::TvShowSeasonPoster
               << ImageType::TvShowLogos << ImageType::TvShowCharacterArt << ImageType::TvShowPoster
               << ImageType::ConcertBackdrop << ImageType::ConcertLogo << ImageType::ConcertClearArt << ImageType::ConcertCdArt;
    m_apiKey = "842f7a5d1cc7396f142b8dd47c4ba42b";
    m_searchResultLimit = 0;
    m_tvdb = new TheTvDb(this);
    m_tmdb = new TMDb(this);
    connect(m_tvdb, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchTvShowFinished(QList<ScraperSearchResult>)));
    connect(m_tmdb, SIGNAL(searchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchMovieFinished(QList<ScraperSearchResult>)));
}

/**
 * @brief Returns the name of this image provider
 * @return Name of this image provider
 */
QString FanartTv::name()
{
    return QString("Fanart.tv");
}

QUrl FanartTv::siteUrl()
{
    return QUrl("https://fanart.tv");
}

QString FanartTv::identifier()
{
    return QString("images.fanarttv");
}

/**
 * @brief Returns a list of supported image types
 * @return List of supported image types
 */
QList<int> FanartTv::provides()
{
    return m_provides;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *FanartTv::qnam()
{
    return &m_qnam;
}

/**
 * @brief Searches for a movie
 * @param searchStr The Movie name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTv::onSearchMovieFinished
 */
void FanartTv::searchMovie(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tmdb->search(searchStr);
}

/**
 * @brief Searches for a concert
 * @param searchStr The Concert name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTv::searchMovie
 */
void FanartTv::searchConcert(QString searchStr, int limit)
{
    searchMovie(searchStr, limit);
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there are no more pages in the result set
 * @param results List of results from scraper
 * @see TMDb::parseSearch
 */
void FanartTv::onSearchMovieFinished(QList<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

/**
 * @brief Loads given image types
 * @param movie
 * @param tmdbId
 * @param types
 */
void FanartTv::movieImages(Movie *movie, QString tmdbId, QList<int> types)
{
    loadMovieData(tmdbId, types, movie);
}

/**
 * @brief Load movie posters
 * @param tmdbId
 */
void FanartTv::moviePosters(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MoviePoster);
}

/**
 * @brief Load movie backdrops
 * @param tmdbId
 */
void FanartTv::movieBackdrops(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieBackdrop);
}

/**
 * @brief Load movie logos
 * @param tmdbId The Movie DB id
 */
void FanartTv::movieLogos(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieLogo);
}

void FanartTv::movieBanners(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieBanner);
}

void FanartTv::movieThumbs(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieThumb);
}

/**
 * @brief Load movie clear arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::movieClearArts(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieClearArt);
}

/**
 * @brief Load movie cd arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::movieCdArts(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::MovieCdArt);
}

/**
 * @brief Loads given image types
 * @param concert
 * @param tmdbId
 * @param types
 */
void FanartTv::concertImages(Concert *concert, QString tmdbId, QList<int> types)
{
    loadConcertData(tmdbId, types, concert);
}

/**
 * @brief Would load concert posters (not supported by fanart.tv)
 * @param tmdbId
 */
void FanartTv::concertPosters(QString tmdbId)
{
    Q_UNUSED(tmdbId);
}

/**
 * @brief Load concert backdrops
 * @param tmdbId
 */
void FanartTv::concertBackdrops(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertBackdrop);
}

/**
 * @brief Load concert logos
 * @param tmdbId The Movie DB id
 */
void FanartTv::concertLogos(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertLogo);
}

/**
 * @brief Load concert clear arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::concertClearArts(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertClearArt);
}

/**
 * @brief Load concert cd arts
 * @param tmdbId The Movie DB id
 */
void FanartTv::concertCdArts(QString tmdbId)
{
    loadMovieData(tmdbId, ImageType::ConcertCdArt);
}

/**
 * @brief FanartTv::loadMovieData
 * @param tmdbId
 * @param type
 */
void FanartTv::loadMovieData(QString tmdbId, int type)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/movies/%1?%2").arg(tmdbId).arg(keyParameter()));
    qDebug() << url;
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", type);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadMovieDataFinished()));
}

/**
 * @brief FanartTv::loadMovieData
 * @param tmdbId
 * @param types
 */
void FanartTv::loadMovieData(QString tmdbId, QList<int> types, Movie *movie)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/movies/%1?%2").arg(tmdbId).arg(keyParameter()));
    qDebug() << url;
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("storage", Storage::toVariant(reply, movie));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAllMovieDataFinished()));
}

/**
 * @brief FanartTv::loadConcertData
 * @param tmdbId
 * @param types
 */
void FanartTv::loadConcertData(QString tmdbId, QList<int> types, Concert *concert)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/movies/%1?%2").arg(tmdbId).arg(keyParameter()));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    reply->setProperty("storage", Storage::toVariant(reply, concert));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAllConcertDataFinished()));
}

/**
 * @brief Called when the movie images are downloaded
 * @see TMDbImages::parseMovieData
 */
void FanartTv::onLoadMovieDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    QList<Poster> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        posters = parseMovieData(msg, reply->property("infoToLoad").toInt());
    }
    emit sigImagesLoaded(posters);
}

/**
 * @brief Called when all movie images are downloaded
 * @see TMDbImages::parseMovieData
 */
void FanartTv::onLoadAllMovieDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Movie *movie = reply->property("storage").value<Storage*>()->movie();
    reply->deleteLater();
    QMap<int, QList<Poster> > posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        foreach (int type, reply->property("infosToLoad").value<Storage*>()->infosToLoad())
            posters.insert(type, parseMovieData(msg, type));
    }
    emit sigImagesLoaded(movie, posters);
}

/**
 * @brief Called when all concert images are downloaded
 * @see TMDbImages::parseMovieData
 */
void FanartTv::onLoadAllConcertDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Concert *concert = reply->property("storage").value<Storage*>()->concert();
    reply->deleteLater();
    QMap<int, QList<Poster> > posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        foreach (int type, reply->property("infosToLoad").value<Storage*>()->infosToLoad())
            posters.insert(type, parseMovieData(msg, type));
    }
    emit sigImagesLoaded(concert, posters);
}

/**
 * @brief Parses JSON data for movies
 * @param json JSON data
 * @param type Type of image (ImageType)
 * @return List of posters
 */
QList<Poster> FanartTv::parseMovieData(QString json, int type)
{
    QMap<int, QStringList> map;
    map.insert(ImageType::MoviePoster, QStringList() << "movieposter");
    map.insert(ImageType::MovieBackdrop, QStringList() << "moviebackground");
    map.insert(ImageType::MovieLogo, QStringList() << "hdmovielogo" << "movielogo");
    map.insert(ImageType::MovieClearArt, QStringList() << "hdmovieclearart" << "movieart");
    map.insert(ImageType::MovieCdArt, QStringList() << "moviedisc");
    map.insert(ImageType::MovieBanner, QStringList() << "moviebanner");
    map.insert(ImageType::MovieThumb, QStringList() << "moviethumb");
    map.insert(ImageType::ConcertBackdrop, QStringList() << "moviebackground");
    map.insert(ImageType::ConcertLogo, QStringList() << "hdmovielogo" << "movielogo");
    map.insert(ImageType::ConcertClearArt, QStringList() << "hdmovieclearart" << "movieart");
    map.insert(ImageType::ConcertCdArt, QStringList() << "moviedisc");
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    foreach (const QString &section, map.value(type)) {
        if (sc.property(section).isArray()) {
            QScriptValueIterator itB(sc.property(section));
            while (itB.hasNext()) {
                itB.next();
                QScriptValue vB = itB.value();
                if (vB.property("url").toString().isEmpty())
                    continue;
                Poster b;
                b.thumbUrl = vB.property("url").toString().replace("/fanart/", "/preview/");
                b.originalUrl = vB.property("url").toString();
                if (section == "hdmovielogo" || section == "hdmovieclearart")
                    b.hint = "HD";
                else if (section == "movielogo" || section == "movieart")
                    b.hint = "SD";
                else if (vB.property("disc_type").toString() == "bluray")
                    b.hint = "BluRay";
                else if (vB.property("disc_type").toString() == "dvd")
                    b.hint = "DVD";
                else if (vB.property("disc_type").toString() == "3d")
                    b.hint = "3D";
                b.language = vB.property("lang").toString();
                insertPoster(posters, b, m_language, m_preferredDiscType);
            }
        }
    }

    return posters;
}

/**
 * @brief Searches for a tv show
 * @param searchStr The tv show name/search string
 * @param limit Number of results, if zero, all results are returned
 * @see FanartTv::onSearchTvShowFinished
 */
void FanartTv::searchTvShow(QString searchStr, int limit)
{
    m_searchResultLimit = limit;
    m_tvdb->search(searchStr);
}

/**
 * @brief FanartTv::onSearchTvShowFinished
 * @param results Result list
 */
void FanartTv::onSearchTvShowFinished(QList<ScraperSearchResult> results)
{
    if (m_searchResultLimit == 0)
        emit sigSearchDone(results);
    else
        emit sigSearchDone(results.mid(0, m_searchResultLimit));
}

/**
 * @brief Loads given image types
 * @param show
 * @param tvdbId
 * @param types
 */
void FanartTv::tvShowImages(TvShow *show, QString tvdbId, QList<int> types)
{
    loadTvShowData(tvdbId, types, show);
}

/**
 * @brief FanartTv::loadTvShowData
 * @param tvdbId The Tv DB Id
 * @param type
 */
void FanartTv::loadTvShowData(QString tvdbId, int type, int season)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/tv/%1?%2").arg(tvdbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infoToLoad", type);
    reply->setProperty("season", season);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadTvShowDataFinished()));
}

/**
 * @brief FanartTv::loadTvShowData
 * @param tvdbId The Tv DB Id
 * @param types
 */
void FanartTv::loadTvShowData(QString tvdbId, QList<int> types, TvShow *show)
{
    QUrl url;
    QNetworkRequest request;
    request.setRawHeader("Accept", "application/json");
    url.setUrl(QString("http://webservice.fanart.tv/v3/tv/%1?%2").arg(tvdbId).arg(keyParameter()));
    request.setUrl(url);
    QNetworkReply *reply = qnam()->get(QNetworkRequest(request));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, types));
    reply->setProperty("storage", Storage::toVariant(reply, show));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadAllTvShowDataFinished()));
}

/**
 * @brief Called when the tv show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void FanartTv::onLoadTvShowDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    QList<Poster> posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        posters = parseTvShowData(msg, reply->property("infoToLoad").toInt(), reply->property("season").toInt());
    }
    emit sigImagesLoaded(posters);
}

/**
 * @brief Called when all tv show images are downloaded
 * @see TMDbImages::parseTvShowData
 */
void FanartTv::onLoadAllTvShowDataFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    TvShow *show = reply->property("storage").value<Storage*>()->show();
    reply->deleteLater();
    QMap<int, QList<Poster> > posters;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        foreach (int type, reply->property("infosToLoad").value<Storage*>()->infosToLoad())
            posters.insert(type, parseTvShowData(msg, type));
    }
    reply->deleteLater();
    emit sigImagesLoaded(show, posters);
}

/**
 * @brief Load tv show posters
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowPosters(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowPoster);
}

/**
 * @brief Load tv show backdrops
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowBackdrops(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowBackdrop);
}

/**
 * @brief Load tv show logos
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowLogos(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowLogos);
}

void FanartTv::tvShowThumbs(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowThumb);
}

/**
 * @brief Load tv show clear arts
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowClearArts(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowClearArt);
}

/**
 * @brief Load tv show character arts
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowCharacterArts(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowCharacterArt);
}

/**
 * @brief Load tv show banners
 * @param tvdbId The TV DB id
 */
void FanartTv::tvShowBanners(QString tvdbId)
{
    loadTvShowData(tvdbId, ImageType::TvShowBanner);
}

/**
 * @brief Load tv show thumbs
 * @param tvdbId The TV DB id
 * @param season Season number
 * @param episode Episode number
 */
void FanartTv::tvShowEpisodeThumb(QString tvdbId, int season, int episode)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
    Q_UNUSED(episode);
}

/**
 * @brief Load tv show season
 * @param tvdbId The TV DB id
 * @param season Season number
 */
void FanartTv::tvShowSeason(QString tvdbId, int season)
{
    loadTvShowData(tvdbId, ImageType::TvShowSeasonPoster, season);
}

void FanartTv::tvShowSeasonBanners(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTv::tvShowSeasonBackdrops(QString tvdbId, int season)
{
    Q_UNUSED(tvdbId);
    Q_UNUSED(season);
}

void FanartTv::tvShowSeasonThumbs(QString tvdbId, int season)
{
    loadTvShowData(tvdbId, ImageType::TvShowSeasonThumb, season);
}

/**
 * @brief Parses JSON data for tv shows
 * @param json JSON data
 * @param type Type of image (ImageType)
 * @return List of posters
 */
QList<Poster> FanartTv::parseTvShowData(QString json, int type, int season)
{
    QMap<int, QStringList> map;
    map.insert(ImageType::TvShowBackdrop, QStringList() << "showbackground");
    map.insert(ImageType::TvShowLogos, QStringList() << "hdtvlogo" << "clearlogo");
    map.insert(ImageType::TvShowClearArt, QStringList() << "hdclearart" << "clearart");
    map.insert(ImageType::TvShowBanner, QStringList() << "tvbanner");
    map.insert(ImageType::TvShowCharacterArt, QStringList() << "characterart");
    map.insert(ImageType::TvShowThumb, QStringList() << "tvthumb");
    map.insert(ImageType::TvShowSeasonThumb, QStringList() << "seasonthumb");
    map.insert(ImageType::TvShowSeasonPoster, QStringList() << "seasonposter");
    map.insert(ImageType::TvShowPoster, QStringList() << "tvposter");
    QList<Poster> posters;
    QScriptValue sc;
    QScriptEngine engine;
    sc = engine.evaluate("(" + QString(json) + ")");

    foreach (const QString &section, map.value(type)) {
        if (sc.property(section).isArray()) {
            QScriptValueIterator itB(sc.property(section));
            while (itB.hasNext()) {
                itB.next();
                QScriptValue vB = itB.value();
                if (vB.property("url").toString().isEmpty())
                    continue;

                if ((type == ImageType::TvShowSeasonThumb || type == ImageType::TvShowSeasonPoster) && season != -2 && !vB.property("season").toString().isEmpty() && vB.property("season").toString().toInt() != season)
                    continue;

                Poster b;
                b.thumbUrl = vB.property("url").toString().replace("/fanart/", "/preview/");
                b.originalUrl = vB.property("url").toString();
                b.season = vB.property("season").toString().toInt();
                if (section == "hdtvlogo" || section == "hdclearart")
                    b.hint = "HD";
                else if (section == "clearlogo" || section == "clearart")
                    b.hint = "SD";
                else if (vB.property("disc_type").toString() == "bluray")
                    b.hint = "BluRay";
                else if (vB.property("disc_type").toString() == "dvd")
                    b.hint = "DVD";
                else if (vB.property("disc_type").toString() == "3d")
                    b.hint = "3D";
                b.language = vB.property("lang").toString();
                insertPoster(posters, b, m_language, m_preferredDiscType);
            }
        }
    }

    return posters;
}

bool FanartTv::hasSettings()
{
    return true;
}

void FanartTv::loadSettings(QSettings &settings)
{
    m_tvdb->loadSettings(settings);
    m_tmdb->loadSettings(settings);
    m_language = settings.value("Scrapers/FanartTv/Language", "en").toString();
    m_preferredDiscType = settings.value("Scrapers/FanartTv/DiscType", "BluRay").toString();
    m_personalApiKey = settings.value("Scrapers/FanartTv/PersonalApiKey").toString();
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language)
            m_box->setCurrentIndex(i);
    }
    for (int i=0, n=m_discBox->count() ; i<n ; ++i) {
        if (m_discBox->itemData(i).toString() == m_preferredDiscType)
            m_discBox->setCurrentIndex(i);
    }
    m_personalApiKeyEdit->setText(m_personalApiKey);
}

void FanartTv::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    m_preferredDiscType = m_discBox->itemData(m_discBox->currentIndex()).toString();
    m_personalApiKey = m_personalApiKeyEdit->text();
    settings.setValue("Scrapers/FanartTv/Language", m_language);
    settings.setValue("Scrapers/FanartTv/DiscType", m_preferredDiscType);
    settings.setValue("Scrapers/FanartTv/PersonalApiKey", m_personalApiKey);
}

QWidget* FanartTv::settingsWidget()
{
    return m_widget;
}

void FanartTv::insertPoster(QList<Poster> &posters, Poster b, QString language, QString preferredDiscType)
{
    int lastInPreferredLangAndHd = -1;
    int lastInPreferredLang = -1;
    int lastHd = -1;

    for (int i=0, n=posters.count() ; i<n ; ++i) {
        if (posters[i].language == language && (posters[i].hint == "HD" || posters[i].hint == preferredDiscType))
            lastInPreferredLangAndHd = i;
        if (posters[i].language == language)
            lastInPreferredLang = i;
        if (posters[i].hint == "HD" || posters[i].hint == preferredDiscType)
            lastHd = i;
    }

    if (b.language == language && (b.hint == "HD" || b.hint == preferredDiscType))
        posters.insert(lastInPreferredLangAndHd+1, b);
    else if (b.language == language)
        posters.insert(lastInPreferredLang+1, b);
    else if (b.hint == "HD" || b.hint == preferredDiscType)
        posters.insert(lastHd+1, b);
    else
        posters.append(b);
}

QString FanartTv::keyParameter()
{
    return (!m_personalApiKey.isEmpty()) ? QString("api_key=%1&client_key=%2").arg(m_apiKey).arg(m_personalApiKey) : QString("api_key=%1").arg(m_apiKey);
}

void FanartTv::searchAlbum(QString artistName, QString searchStr, int limit)
{
    Q_UNUSED(artistName);
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTv::searchArtist(QString searchStr, int limit)
{
    Q_UNUSED(searchStr);
    Q_UNUSED(limit);
}

void FanartTv::artistFanarts(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::artistLogos(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::artistThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::albumCdArts(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::albumThumbs(QString mbId)
{
    Q_UNUSED(mbId);
}

void FanartTv::artistImages(Artist *artist, QString mbId, QList<int> types)
{
    Q_UNUSED(artist);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTv::albumImages(Album *album, QString mbId, QList<int> types)
{
    Q_UNUSED(album);
    Q_UNUSED(mbId);
    Q_UNUSED(types);
}

void FanartTv::albumBooklets(QString mbId)
{
    Q_UNUSED(mbId);
}
