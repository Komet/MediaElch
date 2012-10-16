#include "TheTvDb.h"

#include <QComboBox>
#include <QDomDocument>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QSpacerItem>

#include "globals/Globals.h"

/**
 * @brief TheTvDb::TheTvDb
 * @param parent
 */
TheTvDb::TheTvDb(QObject *parent)
{
    setParent(parent);
    m_apiKey = "A0BB9A0F6762942B";
    m_language = "en";

    m_xmlMirrors.append("http://thetvdb.com");
    m_bannerMirrors.append("http://thetvdb.com");
    m_zipMirrors.append("http://thetvdb.com");
    setMirrors();
}

/**
 * @brief languages
 * @return
 */
QMap<QString, QString> TheTvDb::languages()
{
    QMap<QString, QString> m;

    m.insert(tr("Bulgarian"), "bg");
    m.insert(tr("Chinese"), "zh");
    m.insert(tr("Croatian"), "hr");
    m.insert(tr("Czech"), "cs");
    m.insert(tr("Danish"), "da");
    m.insert(tr("Dutch"), "nl");
    m.insert(tr("English"), "en");
    m.insert(tr("Finnish"), "fi");
    m.insert(tr("French"), "fr");
    m.insert(tr("German"), "de");
    m.insert(tr("Greek"), "el");
    m.insert(tr("Hebrew"), "he");
    m.insert(tr("Hungarian"), "hu");
    m.insert(tr("Italian"), "it");
    m.insert(tr("Japanese"), "ja");
    m.insert(tr("Korean"), "ko");
    m.insert(tr("Norwegian"), "no");
    m.insert(tr("Polish"), "pl");
    m.insert(tr("Portuguese"), "pt");
    m.insert(tr("Russian"), "ru");
    m.insert(tr("Slovene"), "sl");
    m.insert(tr("Spanish"), "es");
    m.insert(tr("Swedish"), "sv");
    m.insert(tr("Turkish"), "tr");

    return m;
}

/**
 * @brief language
 * @return
 */
QString TheTvDb::language()
{
    return m_language;
}

/**
 * @brief TheTvDb::setLanguage
 * @param language
 */
void TheTvDb::setLanguage(QString language)
{
    m_language = language;
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
void TheTvDb::loadSettings()
{
    QSettings settings;
    m_language = settings.value("Scrapers/TheTvDb/Language", "en").toString();
}

/**
 * @brief Saves scrapers settings
 */
void TheTvDb::saveSettings()
{
    QSettings settings;
    settings.setValue("Scrapers/TheTvDb/Language", m_language);
}

/**
 * @brief Starts loading a list of mirrors
 */
void TheTvDb::setMirrors()
{
    QUrl url(QString("http://www.thetvdb.com/api/%1/mirrors.xml").arg(m_apiKey));
    m_mirrorsReply = qnam()->get(QNetworkRequest(url));
    connect(m_mirrorsReply, SIGNAL(finished()), this, SLOT(onMirrorsReady()));
}

/**
 * @brief Called when mirrors are loaded
 * Parses and assigns mirrors
 */
void TheTvDb::onMirrorsReady()
{
    m_xmlMirrors.clear();
    m_bannerMirrors.clear();
    m_zipMirrors.clear();

    if (m_mirrorsReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_mirrorsReply->readAll());
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

    m_mirrorsReply->deleteLater();
}

/**
 * @brief Searches for a tv show
 * @param searchStr The tv show name/search string
 * @see TheTvDb::onSearchFinished
 */
void TheTvDb::search(QString searchStr)
{
    qDebug() << "Entered, searchStr=" << searchStr;
    QUrl url(QString("http://www.thetvdb.com/api/GetSeries.php?language=%1&seriesname=%2").arg(m_language).arg(searchStr));
    m_searchReply = qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

/**
 * @brief Called when the search result was downloaded
 *        Emits "sigSearchDone" if there was an error
 * @see TheTvDb::parseSearch
 */
void TheTvDb::onSearchFinished()
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_searchReply->readAll());
        results = parseSearch(msg);
    } else {
        qWarning() << "Network Error" << m_searchReply->errorString();
    }
    m_searchReply->deleteLater();
    emit sigSearchDone(results);
}

/**
 * @brief Parses the search results
 * @param xml XML data
 * @return List of search results
 */
QList<ScraperSearchResult> TheTvDb::parseSearch(QString xml)
{
    qDebug() << "Entered";
    QList<ScraperSearchResult> results;

    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Series").size() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Series").at(i).toElement();
        ScraperSearchResult result;
        if (!elem.elementsByTagName("SeriesName").isEmpty())
            result.name = elem.elementsByTagName("SeriesName").at(0).toElement().text();
        if (!elem.elementsByTagName("seriesid").isEmpty())
            result.id = elem.elementsByTagName("seriesid").at(0).toElement().text();
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
 * @param updateAllEpisodes Also update all child episodes (regardless if they already have infos or not)
 * @see TheTvDb::onLoadFinished
 */
void TheTvDb::loadTvShowData(QString id, TvShow *show, bool updateAllEpisodes)
{
    qDebug() << "Entered, id=" << id << "show=" << show->name() << "updateAllEpisodes=" << updateAllEpisodes;
    show->setTvdbId(id);
    m_currentShow = show;
    m_currentId = id;
    m_updateAllEpisodes = updateAllEpisodes;
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/all/%4.xml").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    m_loadReply = qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

/**
 * @brief Called when the tv show infos are downloaded
 * Starts download of actors
 * @see TheTvDb::parseAndAssignInfos
 * @see TheTvDb::onActorsFinished
 */
void TheTvDb::onLoadFinished()
{
    qDebug() << "Entered";
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentShow, m_updateAllEpisodes);
    } else {
        qWarning() << "Network Error" << m_loadReply->errorString();
    }
    m_loadReply->deleteLater();
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/actors.xml").arg(mirror).arg(m_apiKey).arg(m_currentId));
    m_actorsReply = qnam()->get(QNetworkRequest(url));
    connect(m_actorsReply, SIGNAL(finished()), this, SLOT(onActorsFinished()));
}

/**
 * @brief Called when the tv show actors are downloaded
 * Starts download of banners
 * @see TheTvDb::parseAndAssignActors
 * @see TheTvDb::onBannersFinished
 */
void TheTvDb::onActorsFinished()
{
    qDebug() << "Entered";
    if (m_actorsReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_actorsReply->readAll());
        parseAndAssignActors(msg, m_currentShow);
    } else {
        qWarning() << "Network Error" << m_actorsReply->errorString();
    }
    m_actorsReply->deleteLater();
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/banners.xml").arg(mirror).arg(m_apiKey).arg(m_currentId));
    m_bannersReply = qnam()->get(QNetworkRequest(url));
    connect(m_bannersReply, SIGNAL(finished()), this, SLOT(onBannersFinished()));
}

/**
 * @brief Called when the tv show banners are downloaded
 * @see TheTvDb::parseAndAssignBanners
 * Tells the current show that scraping has ended
 */
void TheTvDb::onBannersFinished()
{
    qDebug() << "Entered";
    if (m_bannersReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_bannersReply->readAll());
        parseAndAssignBanners(msg, m_currentShow);
    } else {
        qDebug() << "Network Error" << m_bannersReply->errorString();
    }
    m_bannersReply->deleteLater();
    m_currentShow->scraperLoadDone();
}

/**
 * @brief Parses info XML data and assigns it to the given tv show object
 * @param xml XML data
 * @param show Tv Show object
 * @param updateAllEpisodes Update all child episodes (regardless if they already have infos or not)
 */
void TheTvDb::parseAndAssignInfos(QString xml, TvShow *show, bool updateAllEpisodes)
{
    qDebug() << "Entered";
    show->clear();
    QDomDocument domDoc;
    domDoc.setContent(xml);
    if (!domDoc.elementsByTagName("Series").isEmpty()) {
        QDomElement elem = domDoc.elementsByTagName("Series").at(0).toElement();
        if (!elem.elementsByTagName("ContentRating").isEmpty())
            show->setCertification(elem.elementsByTagName("ContentRating").at(0).toElement().text());
        if (!elem.elementsByTagName("FirstAired").isEmpty())
            show->setFirstAired(QDate::fromString(elem.elementsByTagName("FirstAired").at(0).toElement().text(), "yyyy-MM-dd"));
        if (!elem.elementsByTagName("Genre").isEmpty())
            show->setGenres(elem.elementsByTagName("Genre").at(0).toElement().text().split("|", QString::SkipEmptyParts));
        if (!elem.elementsByTagName("Network").isEmpty())
            show->setNetwork(elem.elementsByTagName("Network").at(0).toElement().text());
        if (!elem.elementsByTagName("Overview").isEmpty())
            show->setOverview(elem.elementsByTagName("Overview").at(0).toElement().text());
        if (!elem.elementsByTagName("Rating").isEmpty())
            show->setRating(elem.elementsByTagName("Rating").at(0).toElement().text().toFloat());
        if (!elem.elementsByTagName("SeriesName").isEmpty())
            show->setName(elem.elementsByTagName("SeriesName").at(0).toElement().text());
    }

    for (int i=0, n=domDoc.elementsByTagName("Episode").count() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Episode").at(i).toElement();
        if (!elem.elementsByTagName("SeasonNumber").isEmpty() && !elem.elementsByTagName("EpisodeNumber").isEmpty()) {
            int seasonNumber = elem.elementsByTagName("SeasonNumber").at(0).toElement().text().toInt();
            int episodeNumber = elem.elementsByTagName("EpisodeNumber").at(0).toElement().text().toInt();
            TvShowEpisode *episode = show->episode(seasonNumber, episodeNumber);
            if (episode->isValid() && (updateAllEpisodes || !episode->infoLoaded() ))
                parseAndAssignSingleEpisodeInfos(elem, episode);
        }
    }
}

/**
 * @brief Parses actor XML data and assigns it to the given tv show object
 * @param xml XML data
 * @param show Tv Show object
 */
void TheTvDb::parseAndAssignActors(QString xml, TvShow *show)
{
    qDebug() << "Entered";
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
void TheTvDb::parseAndAssignBanners(QString xml, TvShow *show)
{
    qDebug() << "Entered";
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Banner").count() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Banner").at(i).toElement();
        if (elem.elementsByTagName("BannerType").isEmpty())
            continue;

        QString mirror = m_bannerMirrors.at(qrand()%m_bannerMirrors.count());
        QString bannerType = elem.elementsByTagName("BannerType").at(0).toElement().text();
        QString bannerType2 = elem.elementsByTagName("BannerType2").at(0).toElement().text();
        if (bannerType == "fanart") {
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
        } else if (bannerType == "poster") {
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
        } else if (bannerType == "season" && bannerType2 == "season") {
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
        } else if (bannerType == "series") {
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
void TheTvDb::parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode)
{
    qDebug() << "Entered";
    episode->clear();
    if (!elem.elementsByTagName("Director").isEmpty())
        episode->setDirectors(elem.elementsByTagName("Director").at(0).toElement().text().split("|", QString::SkipEmptyParts));
    if (!elem.elementsByTagName("EpisodeName").isEmpty())
        episode->setName(elem.elementsByTagName("EpisodeName").at(0).toElement().text());
    if (!elem.elementsByTagName("EpisodeNumber").isEmpty())
        episode->setEpisode(elem.elementsByTagName("EpisodeNumber").at(0).toElement().text().toInt());
    if (!elem.elementsByTagName("FirstAired").isEmpty())
        episode->setFirstAired(QDate::fromString(elem.elementsByTagName("FirstAired").at(0).toElement().text(), "yyyy-MM-dd"));
    if (!elem.elementsByTagName("Overview").isEmpty())
        episode->setOverview(elem.elementsByTagName("Overview").at(0).toElement().text());
    if (!elem.elementsByTagName("Rating").isEmpty())
        episode->setRating(elem.elementsByTagName("Rating").at(0).toElement().text().toFloat());
    if (!elem.elementsByTagName("SeasonNumber").isEmpty())
        episode->setSeason(elem.elementsByTagName("SeasonNumber").at(0).toElement().text().toInt());
    if (!elem.elementsByTagName("Writer").isEmpty())
        episode->setWriters(elem.elementsByTagName("Writer").at(0).toElement().text().split("|", QString::SkipEmptyParts));
    if (!elem.elementsByTagName("filename").isEmpty()) {
        QString mirror = m_bannerMirrors.at(qrand()%m_bannerMirrors.count());
        episode->setThumbnail(QUrl(QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("filename").at(0).toElement().text())));
    }
    episode->setInfosLoaded(true);
}

/**
 * @brief Starts network requests to download infos from TheTvDb
 * @param id TheTvDb show ID
 * @param episode Episode object
 * @see TheTvDb::onEpisodeLoadFinished
 */
void TheTvDb::loadTvShowEpisodeData(QString id, TvShowEpisode *episode)
{
    qDebug() << "Entered, id=" << id << "episode=" << episode->name();
    m_currentEpisode = episode;
    m_currentId = id;
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/all/%4.xml").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    m_episodeLoadReply = qnam()->get(QNetworkRequest(url));
    connect(m_episodeLoadReply, SIGNAL(finished()), this, SLOT(onEpisodeLoadFinished()));
}

/**
 * @brief Called when the episode infos are downloaded
 * @see TheTvDb::parseAndAssignSingleEpisodeInfos
 */
void TheTvDb::onEpisodeLoadFinished()
{
    qDebug() << "Entered";
    if (m_episodeLoadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_episodeLoadReply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("Episode").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("Episode").at(i).toElement();
            if (!elem.elementsByTagName("SeasonNumber").isEmpty() && !elem.elementsByTagName("EpisodeNumber").isEmpty()) {
                int seasonNumber = elem.elementsByTagName("SeasonNumber").at(0).toElement().text().toInt();
                int episodeNumber = elem.elementsByTagName("EpisodeNumber").at(0).toElement().text().toInt();
                if (m_currentEpisode->season() == seasonNumber && m_currentEpisode->episode() == episodeNumber)
                    parseAndAssignSingleEpisodeInfos(elem, m_currentEpisode);
            }
        }
    } else {
        qWarning() << "Network Error" << m_episodeLoadReply->errorString();
    }
    m_episodeLoadReply->deleteLater();
    m_currentEpisode->scraperLoadDone();
}
