#include "TheTvDb.h"

#include <QComboBox>
#include <QDomDocument>
#include <QHBoxLayout>
#include <QLabel>
#include <QSettings>
#include <QSpacerItem>

TheTvDb::TheTvDb(QObject *parent)
{
    setParent(parent);
    m_apiKey = "A0BB9A0F6762942B";
    m_language = "en";

    m_settingsLanguageCombo = new QComboBox;
    m_settingsLanguageCombo->addItem(tr("Chinese"), "zh");
    m_settingsLanguageCombo->addItem(tr("Croatian"), "hr");
    m_settingsLanguageCombo->addItem(tr("Czech"), "cs");
    m_settingsLanguageCombo->addItem(tr("Danish"), "da");
    m_settingsLanguageCombo->addItem(tr("Dutch"), "nl");
    m_settingsLanguageCombo->addItem(tr("English"), "en");
    m_settingsLanguageCombo->addItem(tr("Finnish"), "fi");
    m_settingsLanguageCombo->addItem(tr("French"), "fr");
    m_settingsLanguageCombo->addItem(tr("German"), "de");
    m_settingsLanguageCombo->addItem(tr("Greek"), "el");
    m_settingsLanguageCombo->addItem(tr("Hebrew"), "he");
    m_settingsLanguageCombo->addItem(tr("Hungarian"), "hu");
    m_settingsLanguageCombo->addItem(tr("Italian"), "it");
    m_settingsLanguageCombo->addItem(tr("Japanese"), "ja");
    m_settingsLanguageCombo->addItem(tr("Korean"), "ko");
    m_settingsLanguageCombo->addItem(tr("Norwegian"), "no");
    m_settingsLanguageCombo->addItem(tr("Polish"), "pl");
    m_settingsLanguageCombo->addItem(tr("Portuguese"), "pt");
    m_settingsLanguageCombo->addItem(tr("Russian"), "ru");
    m_settingsLanguageCombo->addItem(tr("Slovene"), "sl");
    m_settingsLanguageCombo->addItem(tr("Spanish"), "es");
    m_settingsLanguageCombo->addItem(tr("Swedish"), "sv");
    m_settingsLanguageCombo->addItem(tr("Turkish"), "tr");

    m_xmlMirrors.append("http://thetvdb.com");
    m_bannerMirrors.append("http://thetvdb.com");
    m_zipMirrors.append("http://thetvdb.com");
    setMirrors();
}

QNetworkAccessManager *TheTvDb::qnam()
{
    return &m_qnam;
}

QString TheTvDb::name()
{
    return QString("The TV DB");
}

bool TheTvDb::hasSettings()
{
    return true;
}

void TheTvDb::loadSettings()
{
    QSettings settings;
    m_language = settings.value("Scrapers/TheTvDb/Language", "en").toString();
    for (int i=0, n=m_settingsLanguageCombo->count() ; i<n ; i++) {
        if (m_settingsLanguageCombo->itemData(i).toString() == m_language)
            m_settingsLanguageCombo->setCurrentIndex(i);
    }
}

void TheTvDb::saveSettings()
{
    QSettings settings;
    m_language = m_settingsLanguageCombo->itemData(m_settingsLanguageCombo->currentIndex()).toString();
    settings.setValue("Scrapers/TheTvDb/Language", m_language);
}

QWidget *TheTvDb::settingsWidget()
{
    QWidget *widget = new QWidget;
    m_settingsLanguageCombo->setParent(widget);
    QLabel *label = new QLabel(tr("Language"), widget);
    QHBoxLayout *hboxLayout = new QHBoxLayout(widget);
    QSpacerItem *spacer = new QSpacerItem(1, 1, QSizePolicy::Expanding);
    hboxLayout->addWidget(label);
    hboxLayout->addWidget(m_settingsLanguageCombo);
    hboxLayout->addSpacerItem(spacer);
    widget->setLayout(hboxLayout);

    return widget;
}

void TheTvDb::setMirrors()
{
    QUrl url(QString("http://www.thetvdb.com/api/%1/mirrors.xml").arg(m_apiKey));
    m_mirrorsReply = qnam()->get(QNetworkRequest(url));
    connect(m_mirrorsReply, SIGNAL(finished()), this, SLOT(onMirrorsReady()));
}

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

            if (typemask & 1 == 1)
                m_xmlMirrors.append(mirror);
            if (typemask & 2 == 2)
                m_bannerMirrors.append(mirror);
            if (typemask & 4 == 4)
                m_zipMirrors.append(mirror);
        }
    }

    m_mirrorsReply->deleteLater();
}

void TheTvDb::search(QString searchStr)
{
    QUrl url(QString("http://www.thetvdb.com/api/GetSeries.php?language=%1&seriesname=%2").arg(m_language).arg(searchStr));
    m_searchReply = qnam()->get(QNetworkRequest(url));
    connect(m_searchReply, SIGNAL(finished()), this, SLOT(onSearchFinished()));
}

void TheTvDb::onSearchFinished()
{
    QList<ScraperSearchResult> results;
    if (m_searchReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_searchReply->readAll());
        results = parseSearch(msg);
    }
    m_searchReply->deleteLater();
    emit sigSearchDone(results);
}

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

void TheTvDb::loadTvShowData(QString id, TvShow *show)
{
    m_currentShow = show;
    m_currentId = id;
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/all/%4.xml").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    m_loadReply = qnam()->get(QNetworkRequest(url));
    connect(m_loadReply, SIGNAL(finished()), this, SLOT(onLoadFinished()));
}

void TheTvDb::onLoadFinished()
{
    if (m_loadReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_loadReply->readAll());
        parseAndAssignInfos(msg, m_currentShow);
    }
    m_loadReply->deleteLater();
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/actors.xml").arg(mirror).arg(m_apiKey).arg(m_currentId));
    m_actorsReply = qnam()->get(QNetworkRequest(url));
    connect(m_actorsReply, SIGNAL(finished()), this, SLOT(onActorsFinished()));
}

void TheTvDb::onActorsFinished()
{
    if (m_actorsReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_actorsReply->readAll());
        parseAndAssignActors(msg, m_currentShow);
    }
    m_actorsReply->deleteLater();
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/banners.xml").arg(mirror).arg(m_apiKey).arg(m_currentId));
    m_bannersReply = qnam()->get(QNetworkRequest(url));
    connect(m_bannersReply, SIGNAL(finished()), this, SLOT(onBannersFinished()));
}

void TheTvDb::onBannersFinished()
{
    if (m_bannersReply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(m_bannersReply->readAll());
        parseAndAssignBanners(msg, m_currentShow);
    }
    m_bannersReply->deleteLater();
    m_currentShow->scraperLoadDone();
}

void TheTvDb::parseAndAssignInfos(QString xml, TvShow *show)
{
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
            if (episode->isValid())
                parseAndAssignSingleEpisodeInfos(elem, episode);
        }
    }
}

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

void TheTvDb::parseAndAssignBanners(QString xml, TvShow *show)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);
    for (int i=0, n=domDoc.elementsByTagName("Banner").count() ; i<n ; ++i) {
        QDomElement elem = domDoc.elementsByTagName("Banner").at(i).toElement();
        if (elem.elementsByTagName("BannerType").isEmpty())
            continue;

        QString mirror = m_bannerMirrors.at(qrand()%m_bannerMirrors.count());
        QString bannerType = elem.elementsByTagName("BannerType").at(0).toElement().text();
        if (bannerType == "fanart") {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty())
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            if (!elem.elementsByTagName("ThumbnailPath").isEmpty())
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("ThumbnailPath").at(0).toElement().text());
            show->addBackdrop(p);
        } else if (bannerType == "poster") {
            Poster p;
            if (!elem.elementsByTagName("id").isEmpty())
                p.id = elem.elementsByTagName("id").at(0).toElement().text();
            if (!elem.elementsByTagName("BannerPath").isEmpty()) {
                p.originalUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
                p.thumbUrl = QString("%1/banners/%2").arg(mirror).arg(elem.elementsByTagName("BannerPath").at(0).toElement().text());
            }
            show->addPoster(p);
        } else if (bannerType == "season") {
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
        }
    }
}

void TheTvDb::parseAndAssignSingleEpisodeInfos(QDomElement elem, TvShowEpisode *episode)
{
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
}

void TheTvDb::loadTvShowEpisodeData(QString id, TvShowEpisode *episode)
{
    m_currentEpisode = episode;
    m_currentId = id;
    QString mirror = m_xmlMirrors.at(qrand()%m_xmlMirrors.count());
    QUrl url(QString("%1/api/%2/series/%3/all/%4.xml").arg(mirror).arg(m_apiKey).arg(id).arg(m_language));
    m_episodeLoadReply = qnam()->get(QNetworkRequest(url));
    connect(m_episodeLoadReply, SIGNAL(finished()), this, SLOT(onEpisodeLoadFinished()));
}

void TheTvDb::onEpisodeLoadFinished()
{
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
    }
    m_episodeLoadReply->deleteLater();
    m_currentEpisode->scraperLoadDone();
}
