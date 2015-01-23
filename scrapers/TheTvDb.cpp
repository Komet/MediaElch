#include "TheTvDb.h"

#include <QComboBox>
#include <QDomDocument>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>
#include <QSpacerItem>

#include "data/Storage.h"
#include "globals/Globals.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "main/MainWindow.h"
#include "mediaCenterPlugins/XbmcXml.h"
#include "settings/Settings.h"

/**
 * @brief TheTvDb::TheTvDb
 * @param parent
 */
TheTvDb::TheTvDb(QObject *parent)
{
    setParent(parent);

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
    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);

    m_apiKey = "A0BB9A0F6762942B";
    m_language = "en";
    m_xmlMirrors.append("http://thetvdb.com");
    m_bannerMirrors.append("http://thetvdb.com");
    m_zipMirrors.append("http://thetvdb.com");

    setMirrors();
}

QWidget *TheTvDb::settingsWidget()
{
    return m_widget;
}

/**
 * @brief Just returns a pointer to the scrapers network access manager
 * @return Network Access Manager
 */
QNetworkAccessManager *TheTvDb::qnam()
{
    return &m_qnam;
}

/**
 * @brief Returns the name of the scraper
 * @return Name of the Scraper
 */
QString TheTvDb::name()
{
    return QString("The TV DB");
}

QString TheTvDb::identifier()
{
    return QString("tvdb");
}

QString TheTvDb::apiKey()
{
    return m_apiKey;
}

QString TheTvDb::language()
{
    return m_language;
}

/**
 * @brief Returns if the scraper has settings
 * @return Scraper has settings
 */
bool TheTvDb::hasSettings()
{
    return true;
}

/**
 * @brief Loads scrapers settings
 */
void TheTvDb::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/TheTvDb/Language", "en").toString();
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language)
            m_box->setCurrentIndex(i);
    }
}

/**
 * @brief Saves scrapers settings
 */
void TheTvDb::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setValue("Scrapers/TheTvDb/Language", m_language);
}

/**
 * @brief Starts loading a list of mirrors
 */
void TheTvDb::setMirrors()
{
    QUrl url(QString("http://www.thetvdb.com/api/%1/mirrors.xml").arg(m_apiKey));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onMirrorsReady()));
}

/**
 * @brief Called when mirrors are loaded
 * Parses and assigns mirrors
 */
void TheTvDb::onMirrorsReady()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    m_xmlMirrors.clear();
    m_bannerMirrors.clear();
    m_zipMirrors.clear();

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("Mirror").size() ; i<n ; ++i) {
            QDomNode node = domDoc.elementsByTagName("Mirror").at(i);
            QString mirror;
            int typemask = 0;
            if (!node.toElement().elementsByTagName("mirrorpath").isEmpty())
                mirror = node.toElement().elementsByTagName("mirrorpath").at(0).toElement().text();
            if (!node.toElement().elementsByTagName("typemask").isEmpty())
                typemask = node.toElement().elementsByTagName("typemask").at(0).toElement().text().toInt();

            if ((typemask & 1) == 1)
                m_xmlMirrors.append(mirror);
            if ((typemask & 2) == 2)
                m_bannerMirrors.append(mirror);
            if ((typemask & 4) == 4)
                m_zipMirrors.append(mirror);
        }
    }

    reply->deleteLater();
}

/**
 * @brief Searches for a tv show
 * @param searchStr The tv show name/search string
 * @see TheTvDb::onSearchFinished
 */
void TheTvDb::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QUrl url;
    QRegExp rxId("^id(\\d+)$");
    if (rxId.exactMatch(searchStr))
        url.setUrl(QString("http://www.thetvdb.com/api/%1/series/%2/%3.xml").arg(m_apiKey).arg(rxId.cap(1)).arg(m_language));
    else
        url.setUrl(QString("http://www.thetvdb.com/api/GetSeries.php?language=%1&seriesname=%2").arg(m_language).arg(searchStr));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there was an error
 * @see TheTvDb::parseSearch
 */
void TheTvDb::onSearchFinished()
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
    emit sigSearchDone(results);
}

/**
 * @brief Parses the search results
 * @param xml XML data
 * @return List of search results
 */
QList<ScraperSearchResult> TheTvDb::parseSearch(QString xml)
{
    QList<ScraperSearchResult> results;

    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Series").size() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Series").at(i).toElement();
        ScraperSearchResult result;
        if (!elem.elementsByTagName("SeriesName").isEmpty())
            result.name = elem.elementsByTagName("SeriesName").at(0).toElement().text();
        if (!elem.elementsByTagName("id").isEmpty())
            result.id = elem.elementsByTagName("id").at(0).toElement().text();
        if (!elem.elementsByTagName("FirstAired").isEmpty())
            result.released = QDate::fromString(elem.elementsByTagName("FirstAired").at(0).toElement().text(), "yyyy-MM-dd");

        bool alreadyAdded = false;
        for (int n=0, s=results.count() ; n<s ; ++n) {
            if (results[n].id == result.id)
                alreadyAdded = true;
        }
        if (!alreadyAdded)
            results.append(result);
    }

    return results;
}

/**
 * @brief Starts network requests to download infos from TheTvDb
 * @param id TheTvDb show ID
 * @param show Tv show object
 * @see TheTvDb::onLoadFinished
 */
void TheTvDb::loadTvShowData(QString id, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad)
{
    Q_UNUSED(infosToLoad)
    show->setTvdbId(id);
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/all/%4.xml").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    show->setEpisodeGuideUrl(QString("%1/api/%2/series/%3/all/%4.zip").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, show));
    reply->setProperty("updateType", updateType);
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infosToLoad));
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

/**
 * @brief Called when the tv show infos are downloaded
 * Starts download of actors
 * @see TheTvDb::parseAndAssignInfos
 * @see TheTvDb::onActorsFinished
 */
void TheTvDb::onLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    TvShow *show = reply->property("storage").value<Storage*>()->show();
    TvShowUpdateType updateType = static_cast<TvShowUpdateType>(reply->property("updateType").toInt());
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!show)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, show, updateType, infos);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/actors.xml").arg(mirror).arg(m_apiKey).arg(show->tvdbId()));
    reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, show));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    reply->setProperty("updateType", updateType);
    connect(reply, SIGNAL(finished()), this, SLOT(onActorsFinished()));
}

/**
 * @brief Called when the tv show actors are downloaded
 * Starts download of banners
 * @see TheTvDb::parseAndAssignActors
 * @see TheTvDb::onBannersFinished
 */
void TheTvDb::onActorsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    TvShow *show = reply->property("storage").value<Storage*>()->show();
    TvShowUpdateType updateType = static_cast<TvShowUpdateType>(reply->property("updateType").toInt());
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!show)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        if (show->infosToLoad().contains(TvShowScraperInfos::Actors) &&
                (updateType == UpdateShow || updateType == UpdateShowAndAllEpisodes || updateType == UpdateShowAndNewEpisodes))
            parseAndAssignActors(msg, show);
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/banners.xml").arg(mirror).arg(m_apiKey).arg(show->tvdbId()));
    reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, show));
    reply->setProperty("updateType", updateType);
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onBannersFinished()));
}

/**
 * @brief Called when the tv show banners are downloaded
 * @see TheTvDb::parseAndAssignBanners
 * Tells the current show that scraping has ended
 */
void TheTvDb::onBannersFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    TvShow *show = reply->property("storage").value<Storage*>()->show();
    TvShowUpdateType updateType = static_cast<TvShowUpdateType>(reply->property("updateType").toInt());
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    if (!show)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignBanners(msg, show, updateType, infos);
    } else {
        qDebug() << "Network Error" << reply->errorString();
    }
    show->scraperLoadDone();
}

/**
 * @brief Parses info XML data and assigns it to the given tv show object
 * @param xml XML data
 * @param show Tv Show object
 * @param updateAllEpisodes Update all child episodes (regardless if they already have infos or not)
 */
void TheTvDb::parseAndAssignInfos(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);

    if (updateType == UpdateShow || updateType == UpdateShowAndAllEpisodes || updateType == UpdateShowAndNewEpisodes) {
        show->clear(infosToLoad);
        if (!domDoc.elementsByTagName("Series").isEmpty()) {
            QDomElement elem = domDoc.elementsByTagName("Series").at(0).toElement();
            if (infosToLoad.contains(TvShowScraperInfos::Certification) && !elem.elementsByTagName("ContentRating").isEmpty())
                show->setCertification(Helper::instance()->mapCertification(elem.elementsByTagName("ContentRating").at(0).toElement().text()));
            if (infosToLoad.contains(TvShowScraperInfos::FirstAired) && !elem.elementsByTagName("FirstAired").isEmpty())
                show->setFirstAired(QDate::fromString(elem.elementsByTagName("FirstAired").at(0).toElement().text(), "yyyy-MM-dd"));
            if (infosToLoad.contains(TvShowScraperInfos::Genres) && !elem.elementsByTagName("Genre").isEmpty())
                show->setGenres(Helper::instance()->mapGenre(elem.elementsByTagName("Genre").at(0).toElement().text().split("|", QString::SkipEmptyParts)));
            if (infosToLoad.contains(TvShowScraperInfos::Network) && !elem.elementsByTagName("Network").isEmpty())
                show->setNetwork(elem.elementsByTagName("Network").at(0).toElement().text());
            if (infosToLoad.contains(TvShowScraperInfos::Overview) && !elem.elementsByTagName("Overview").isEmpty())
                show->setOverview(elem.elementsByTagName("Overview").at(0).toElement().text());
            if (infosToLoad.contains(TvShowScraperInfos::Rating) && !elem.elementsByTagName("Rating").isEmpty())
                show->setRating(elem.elementsByTagName("Rating").at(0).toElement().text().toFloat());
            if (infosToLoad.contains(TvShowScraperInfos::Title) && !elem.elementsByTagName("SeriesName").isEmpty())
                show->setName(elem.elementsByTagName("SeriesName").at(0).toElement().text().trimmed());
            if (infosToLoad.contains(TvShowScraperInfos::Runtime) && !elem.elementsByTagName("Runtime").isEmpty())
                show->setRuntime(elem.elementsByTagName("Runtime").at(0).toElement().text().toInt());
        }
    }

    if (updateType == UpdateAllEpisodes || updateType == UpdateNewEpisodes || updateType == UpdateShowAndAllEpisodes || updateType == UpdateShowAndNewEpisodes) {
        for (int i=0, n=domDoc.elementsByTagName("Episode").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("Episode").at(i).toElement();

            TvShowEpisode *episode = 0;
            if (Settings::instance()->tvShowDvdOrder() &&
                    !elem.elementsByTagName("DVD_season").isEmpty() &&
                    !elem.elementsByTagName("DVD_season").at(0).toElement().text().isEmpty() &&
                    !elem.elementsByTagName("DVD_episodenumber").isEmpty() &&
                    !elem.elementsByTagName("DVD_episodenumber").at(0).toElement().text().isEmpty()) {
                QRegExp rx("^(\\d*)\\D*");
                int seasonNumber = -1;
                int episodeNumber = -1;
                QString seasonText = elem.elementsByTagName("DVD_season").at(0).toElement().text();
                QString episodeText = elem.elementsByTagName("DVD_episodenumber").at(0).toElement().text();
                if (rx.indexIn(QString("%1").arg(seasonText), 0) != -1)
                    seasonNumber = rx.cap(1).toInt();
                if (rx.indexIn(QString("%1").arg(episodeText), 0) != -1)
                    episodeNumber = rx.cap(1).toInt();
                episode = show->episode(seasonNumber, episodeNumber);
            } else if (!elem.elementsByTagName("SeasonNumber").isEmpty() && !elem.elementsByTagName("EpisodeNumber").isEmpty()) {
                int seasonNumber = elem.elementsByTagName("SeasonNumber").at(0).toElement().text().toInt();
                int episodeNumber = elem.elementsByTagName("EpisodeNumber").at(0).toElement().text().toInt();
                episode = show->episode(seasonNumber, episodeNumber);
            }

            if (!episode || !episode->isValid())
                continue;

            if (updateType == UpdateAllEpisodes || updateType == UpdateShowAndAllEpisodes ||
                    ((updateType == UpdateNewEpisodes || updateType == UpdateShowAndNewEpisodes) && !episode->infoLoaded())) {
                episode->clear(infosToLoad);
                parseAndAssignSingleEpisodeInfos(elem, episode, infosToLoad);
            }
        }
    }

    fillDatabaseWithAllEpisodes(xml, show);
}

void TheTvDb::fillDatabaseWithAllEpisodes(QString xml, TvShow *show)
{
    QList<int> infosToLoad;
    infosToLoad << TvShowScraperInfos::Director << TvShowScraperInfos::Title << TvShowScraperInfos::FirstAired
                << TvShowScraperInfos::Overview << TvShowScraperInfos::Rating << TvShowScraperInfos::Writer
                << TvShowScraperInfos::Thumbnail;

    int showsSettingsId = Manager::instance()->database()->showsSettingsId(show);
    Manager::instance()->database()->clearEpisodeList(showsSettingsId);

    TvShowEpisode *episode = new TvShowEpisode();
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Episode").count() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Episode").at(i).toElement();
        if (!elem.elementsByTagName("SeasonNumber").isEmpty() && !elem.elementsByTagName("EpisodeNumber").isEmpty()) {
            episode->clear();
            int seasonNumber = elem.elementsByTagName("SeasonNumber").at(0).toElement().text().toInt();
            int episodeNumber = elem.elementsByTagName("EpisodeNumber").at(0).toElement().text().toInt();
            QString id = elem.elementsByTagName("id").at(0).toElement().text();
            episode->setSeason(seasonNumber);
            episode->setEpisode(episodeNumber);
            parseAndAssignSingleEpisodeInfos(elem, episode, infosToLoad);
            Manager::instance()->database()->addEpisodeToShowList(episode, showsSettingsId, id);
        }
    }
    Manager::instance()->database()->cleanUpEpisodeList(showsSettingsId);
}

/**
 * @brief Parses actor XML data and assigns it to the given tv show object
 * @param xml XML data
 * @param show Tv Show object
 */
void TheTvDb::parseAndAssignActors(QString xml, TvShow *show)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Actor").count() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Actor").at(i).toElement();
        Actor actor;
        if (!elem.elementsByTagName("Name").isEmpty())
            actor.name = elem.elementsByTagName("Name").at(0).toElement().text();
        if (!elem.elementsByTagName("Role").isEmpty())
            actor.role = elem.elementsByTagName("Role").at(0).toElement().text();
        if (!elem.elementsByTagName("Image").isEmpty()) {
            QString mirror = m_bannerMirrors.at(qrand()%m_bannerMirrors.count());
            actor.thumb = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("Image").at(0).toElement().text());
        }
        show->addActor(actor);
    }
}

/**
 * @brief Parses banner XML data and assigns it to the given tv show object
 * @param xml XML data
 * @param show Tv Show object
 */
void TheTvDb::parseAndAssignBanners(QString xml, TvShow *show, TvShowUpdateType updateType, QList<int> infosToLoad)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Banner").count() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Banner").at(i).toElement();
        if (elem.elementsByTagName("BannerType").isEmpty())
            continue;

        if (updateType == UpdateAllEpisodes || updateType == UpdateNewEpisodes)
            continue;

        QString mirror = m_bannerMirrors.at(qrand()%m_bannerMirrors.count());
        QString bannerType = elem.elementsByTagName("BannerType").at(0).toElement().text();
        QString bannerType2 = elem.elementsByTagName("BannerType2").at(0).toElement().text();
        if (bannerType == "fanart" && infosToLoad.contains(TvShowScraperInfos::Fanart)) {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty())
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            if (!elem.elementsByTagName("ThumbnailPath").isEmpty())
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("ThumbnailPath").at(0).toElement().text());
            if (!elem.elementsByTagName("BannerType2").isEmpty()) {
                QRegExp rx("(\\d+)x(\\d+)");
                if (rx.indexIn(elem.elementsByTagName("BannerType2").at(0).toElement().text(), 0) != -1) {
                    p.originalSize.setWidth(rx.cap(1).toInt());
                    p.originalSize.setHeight(rx.cap(2).toInt());
                }
            }
            show->addBackdrop(p);
        } else if (bannerType == "poster" && infosToLoad.contains(TvShowScraperInfos::Poster)) {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty()) {
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            }
            if (!elem.elementsByTagName("BannerType2").isEmpty()) {
                QRegExp rx("(\\d+)x(\\d+)");
                if (rx.indexIn(elem.elementsByTagName("BannerType2").at(0).toElement().text(), 0) != -1) {
                    p.originalSize.setWidth(rx.cap(1).toInt());
                    p.originalSize.setHeight(rx.cap(2).toInt());
                }
            }
            show->addPoster(p);
        } else if (bannerType == "season" && bannerType2 == "season" && infosToLoad.contains(TvShowScraperInfos::SeasonPoster)) {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty()) {
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            }
            if (!elem.elementsByTagName("Season").isEmpty()) {
                int season = elem.elementsByTagName("Season").at(0).toElement().text().toInt();
                show->addSeasonPoster(season, p);
            }
        } else if (bannerType == "season" && bannerType2 == "seasonwide" && infosToLoad.contains(TvShowScraperInfos::SeasonBanner)) {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty()) {
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            }
            if (!elem.elementsByTagName("Season").isEmpty()) {
                int season = elem.elementsByTagName("Season").at(0).toElement().text().toInt();
                show->addSeasonBanner(season, p);
            }
        } else if (bannerType == "series" && (infosToLoad.contains(TvShowScraperInfos::Banner) || infosToLoad.contains(TvShowScraperInfos::SeasonBanner))) {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty()) {
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            }
            show->addBanner(p);
        }
    }
}

/**
 * @brief Parses XML data (dom element) and assigns it to the given episode object
 * @param elem Dom element
 * @param episode Episode object
 */
void TheTvDb::parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode, QList<int> infosToLoad)
{
    if (infosToLoad.contains(TvShowScraperInfos::Director) && !elem.elementsByTagName("Director").isEmpty())
        episode->setDirectors(elem.elementsByTagName("Director").at(0).toElement().text().split("|", QString::SkipEmptyParts));
    if (infosToLoad.contains(TvShowScraperInfos::Title) && !elem.elementsByTagName("EpisodeName").isEmpty())
        episode->setName(elem.elementsByTagName("EpisodeName").at(0).toElement().text().trimmed());
    if (infosToLoad.contains(TvShowScraperInfos::FirstAired) && !elem.elementsByTagName("FirstAired").isEmpty())
        episode->setFirstAired(QDate::fromString(elem.elementsByTagName("FirstAired").at(0).toElement().text(), "yyyy-MM-dd"));
    if (infosToLoad.contains(TvShowScraperInfos::Overview) && !elem.elementsByTagName("Overview").isEmpty())
        episode->setOverview(elem.elementsByTagName("Overview").at(0).toElement().text());
    if (infosToLoad.contains(TvShowScraperInfos::Rating) && !elem.elementsByTagName("Rating").isEmpty())
        episode->setRating(elem.elementsByTagName("Rating").at(0).toElement().text().toFloat());
    if (infosToLoad.contains(TvShowScraperInfos::Writer) && !elem.elementsByTagName("Writer").isEmpty())
        episode->setWriters(elem.elementsByTagName("Writer").at(0).toElement().text().split("|", QString::SkipEmptyParts));
    if (infosToLoad.contains(TvShowScraperInfos::Thumbnail) && !elem.elementsByTagName("filename").isEmpty() &&
            !elem.elementsByTagName("filename").at(0).toElement().text().isEmpty()) {
        QString mirror = m_bannerMirrors.at(qrand()%m_bannerMirrors.count());
        episode->setThumbnail(QUrl(QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("filename").at(0).toElement().text())));
    }
    if (!elem.elementsByTagName("airsafter_season").isEmpty() && !elem.elementsByTagName("airsafter_season").at(0).toElement().text().isEmpty() &&
            !elem.elementsByTagName("airsbefore_season").isEmpty() && !elem.elementsByTagName("airsbefore_season").at(0).toElement().text().isEmpty()) {
        episode->setDisplaySeason(elem.elementsByTagName("airsafter_season").at(0).toElement().text().toInt());
        episode->setDisplayEpisode(4096);
    } else if (!elem.elementsByTagName("airsbefore_season").isEmpty() && !elem.elementsByTagName("airsbefore_season").at(0).toElement().text().isEmpty()) {
        episode->setDisplaySeason(elem.elementsByTagName("airsbefore_season").at(0).toElement().text().toInt());
        if (!elem.elementsByTagName("airsbefore_episode").isEmpty() && !elem.elementsByTagName("airsbefore_episode").at(0).toElement().text().isEmpty())
            episode->setDisplayEpisode(elem.elementsByTagName("airsbefore_episode").at(0).toElement().text().toInt());
    }

    episode->setInfosLoaded(true);
}

/**
 * @brief Starts network requests to download infos from TheTvDb
 * @param id TheTvDb show ID
 * @param episode Episode object
 * @see TheTvDb::onEpisodeLoadFinished
 */
void TheTvDb::loadTvShowEpisodeData(QString id, TvShowEpisode *episode, QList<int> infosToLoad)
{
    qDebug() << "Entered, id=" << id << "episode=" << episode->name();
    episode->clear(infosToLoad);
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/all/%4.xml").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, episode));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infosToLoad));
    connect(reply, SIGNAL(finished()), this, SLOT(onEpisodeLoadFinished()));
}

/**
 * @brief Called when the episode infos are downloaded
 * @see TheTvDb::parseAndAssignSingleEpisodeInfos
 */
void TheTvDb::onEpisodeLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    TvShowEpisode *episode = reply->property("storage").value<Storage*>()->episode();
    if (!episode)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        QDomElement dvdElem;
        QDomElement airedElem;
        for (int i=0, n=domDoc.elementsByTagName("Episode").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("Episode").at(i).toElement();
            if (!elem.elementsByTagName("SeasonNumber").isEmpty() && !elem.elementsByTagName("EpisodeNumber").isEmpty()) {
                int seasonNumber = elem.elementsByTagName("SeasonNumber").at(0).toElement().text().toInt();
                int episodeNumber = elem.elementsByTagName("EpisodeNumber").at(0).toElement().text().toInt();
                if (episode->season() == seasonNumber && episode->episode() == episodeNumber)
                    airedElem = elem;
            }
            if (!elem.elementsByTagName("DVD_season").isEmpty() &&
                    !elem.elementsByTagName("DVD_season").at(0).toElement().text().isEmpty() &&
                    !elem.elementsByTagName("DVD_episodenumber").isEmpty() &&
                    !elem.elementsByTagName("DVD_episodenumber").at(0).toElement().text().isEmpty()) {
                QRegExp rx("^(\\d*)\\D*");
                int seasonNumber = -1;
                int episodeNumber = -1;
                QString seasonText = elem.elementsByTagName("DVD_season").at(0).toElement().text();
                QString episodeText = elem.elementsByTagName("DVD_episodenumber").at(0).toElement().text();
                if (rx.indexIn(QString("%1").arg(seasonText), 0) != -1)
                    seasonNumber = rx.cap(1).toInt();
                if (rx.indexIn(QString("%1").arg(episodeText), 0) != -1)
                    episodeNumber = rx.cap(1).toInt();
                if (episode->season() == seasonNumber && episode->episode() == episodeNumber)
                    dvdElem = elem;
            }
            if (!dvdElem.isNull() && !airedElem.isNull())
                break;
        }

        qDebug() << "DVD ORDER" << Settings::instance()->tvShowDvdOrder();

        if (Settings::instance()->tvShowDvdOrder() && !dvdElem.isNull()) {
            episode->clear(infos);
            parseAndAssignSingleEpisodeInfos(dvdElem, episode, infos);
        } else if (!airedElem.isNull()) {
            episode->clear(infos);
            parseAndAssignSingleEpisodeInfos(airedElem, episode, infos);
        }
    } else {
        qWarning() << "Network Error" << reply->errorString();
    }
    episode->scraperLoadDone();
}
