#include "UniversalMusicScraper.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGridLayout>
#include <QLabel>
#include <QMutexLocker>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>

#include "../data/Storage.h"
#include "../globals/NetworkReplyWatcher.h"
#include "../main/MainWindow.h"

UniversalMusicScraper::UniversalMusicScraper(QObject *parent)
{
    setParent(parent);
    m_tadbApiKey = "7490823590829082posuda";

    m_language = "en";
    m_prefer = "theaudiodb";
    m_widget = new QWidget(MainWindow::instance());
    m_box = new QComboBox(m_widget);
    m_box->addItem(tr("Chinese"), "cn");
    m_box->addItem(tr("Dutch"), "nl");
    m_box->addItem(tr("English"), "en");
    m_box->addItem(tr("French"), "fr");
    m_box->addItem(tr("German"), "de");
    m_box->addItem(tr("Hebrew"), "il");
    m_box->addItem(tr("Hungarian"), "hu");
    m_box->addItem(tr("Italian"), "it");
    m_box->addItem(tr("Japanese"), "jp");
    m_box->addItem(tr("Norwegian"), "no");
    m_box->addItem(tr("Polish"), "pl");
    m_box->addItem(tr("Portuguese"), "pt");
    m_box->addItem(tr("Russian"), "ru");
    m_box->addItem(tr("Spanish"), "es");
    m_box->addItem(tr("Swedish"), "se");
    m_preferBox = new QComboBox(m_widget);
    m_preferBox->addItem(tr("The Audio DB"), "theaudiodb");
    m_preferBox->addItem(tr("AllMusic"), "allmusic");
    m_preferBox->addItem(tr("Discogs"), "discogs");
    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Prefer")), 1, 0);
    layout->addWidget(m_preferBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

QNetworkAccessManager *UniversalMusicScraper::qnam()
{
    return &m_qnam;
}

QString UniversalMusicScraper::name()
{
    return QString("Universal Music Scraper");
}

QString UniversalMusicScraper::identifier()
{
    return QString("universalmusicscraper");
}

void UniversalMusicScraper::searchArtist(QString searchStr)
{
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/?query=artist:\"%1\"").arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchArtistFinished()));
}

void UniversalMusicScraper::onSearchArtistFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("artist").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("artist").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("name").isEmpty())
                name = elem.elementsByTagName("name").at(0).toElement().text();
            if (!elem.elementsByTagName("disambiguation").isEmpty())
                name.append(QString(" (%1)").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text()));

            if (!name.isEmpty() && !elem.attribute("id").isEmpty()) {
                ScraperSearchResult result;
                result.id = elem.attribute("id");
                result.name = name;
                results.append(result);
            }
        }
    }
    emit sigSearchDone(results);
}

void UniversalMusicScraper::loadData(QString mbId, Artist *artist, QList<int> infos)
{
    // Otherwise deleted images are showing up again
    infos.removeOne(MusicScraperInfos::ExtraFanarts);
    artist->clear(infos);
    artist->setMbId(mbId);
    artist->setAllMusicId("");
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/%1?inc=url-rels").arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onArtistRelsFinished()));
}

void UniversalMusicScraper::onArtistRelsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    QString discogsUrl;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("relation").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
            if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                QString url = elem.elementsByTagName("target").at(0).toElement().text();
                QRegExp rx("allmusic\\.com/artist/(.*)$");
                if (rx.indexIn(url) != -1)
                    artist->setAllMusicId(rx.cap(1));
            }
            if (elem.attribute("type") == "discogs" && elem.elementsByTagName("target").count() > 0)
                discogsUrl = elem.elementsByTagName("target").at(0).toElement().text();
        }
    }

    if (!m_artistDownloads.contains(artist))
        m_artistDownloads.insert(artist, QList<DownloadElement>());
    m_artistDownloads[artist].clear();

    appendDownloadElement(artist, "theaudiodb", "tadb_data", QUrl(QString("http://www.theaudiodb.com/api/v1/json/%1/artist-mb.php?i=%2").arg(m_tadbApiKey).arg(artist->mbId())));
    appendDownloadElement(artist, "theaudiodb", "tadb_discography", QUrl(QString("http://www.theaudiodb.com/api/v1/json/%1/discography-mb.php?s=%2").arg(m_tadbApiKey).arg(artist->mbId())));
    if (!artist->allMusicId().isEmpty()) {
        appendDownloadElement(artist, "allmusic", "am_data", QUrl(QString("http://www.allmusic.com/artist/%1").arg(artist->allMusicId())));
        appendDownloadElement(artist, "allmusic", "am_biography", QUrl(QString("http://www.allmusic.com/artist/%1/biography").arg(artist->allMusicId())));
    }
    if (!discogsUrl.isEmpty())
        appendDownloadElement(artist, "discogs", "discogs_data", QUrl(discogsUrl + "?type=Releases&subtype=Albums"));

    foreach (DownloadElement elem, m_artistDownloads[artist]) {
        QNetworkRequest request(elem.url);
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0");
        QNetworkReply *reply = qnam()->get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, artist));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(onArtistLoadFinished()));
    }
}

void UniversalMusicScraper::onArtistLoadFinished()
{
    QMutexLocker locker(&m_artistMutex);

    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    if (!m_artistDownloads.contains(artist))
        return;

    int index = -1;
    for (int i=0, n=m_artistDownloads[artist].count() ; i<n ; ++i) {
        if (m_artistDownloads[artist][i].url == reply->url()) {
            index = i;
            break;
        }
    }

    if (index == -1)
        return;

    if (reply->error() == QNetworkReply::NoError)
        m_artistDownloads[artist][index].contents = QString::fromUtf8(reply->readAll());
    else
        qWarning() << "Network Error (load)" << reply->errorString();
    m_artistDownloads[artist][index].downloaded = true;

    bool finished = true;
    for (int i=0, n=m_artistDownloads[artist].count() ; i<n ; ++i) {
        if (!m_artistDownloads[artist][i].downloaded) {
            finished = false;
            break;
        }
    }

    if (!finished)
        return;

    foreach (DownloadElement elem, m_artistDownloads[artist]) {
        if (elem.source != m_prefer)
            continue;
        processDownloadElement(elem, artist, infos);
    }
    foreach (DownloadElement elem, m_artistDownloads[artist]) {
        if (elem.source == m_prefer)
            continue;
        processDownloadElement(elem, artist, infos);
    }

    m_artistDownloads.remove(artist);
    artist->controller()->scraperLoadDone(this);
}

void UniversalMusicScraper::processDownloadElement(DownloadElement elem, Artist *artist, QList<int> infos)
{
    if (elem.type == "tadb_data")
        parseAndAssignTadbInfos(elem.contents, artist, infos);
    else if (elem.type == "tadb_discography")
        parseAndAssignTadbDiscography(elem.contents, artist, infos);
    else if (elem.type == "am_data")
        parseAndAssignAmInfos(elem.contents, artist, infos);
    else if (elem.type == "am_biography")
        parseAndAssignAmBiography(elem.contents, artist, infos);
    else if (elem.type == "discogs_data")
        parseAndAssignDiscogsInfos(elem.contents, artist, infos);
}

void UniversalMusicScraper::searchAlbum(QString artistName, QString searchStr)
{
    QString year;
    QString cleanSearchStr = searchStr;
    QRegExp rx("^(.*)([0-9]{4})\\)?$");
    rx.setMinimal(true);
    if (rx.exactMatch(searchStr)) {
        year = rx.cap(2);
        cleanSearchStr = rx.cap(1);
    }
    rx.setPattern("^\\(?([0-9]{4})\\)?(.*)$");
    if (rx.exactMatch(searchStr)) {
        year = rx.cap(1);
        cleanSearchStr = rx.cap(2);
    }
    cleanSearchStr.replace("(", "");
    cleanSearchStr.replace(")", "");
    cleanSearchStr.replace("[", "");
    cleanSearchStr.replace("]", "");
    cleanSearchStr.replace("-", "");
    cleanSearchStr = cleanSearchStr.trimmed();
    if (cleanSearchStr.isEmpty())
        cleanSearchStr = searchStr;

    QString searchQuery = "release:\"" + QString(QUrl::toPercentEncoding(cleanSearchStr)) + "\"";
    if (!artistName.isEmpty())
        searchQuery += "%20AND%20artist:\"" + QString(QUrl::toPercentEncoding(artistName)) + "\"";
    QUrl url(QString("http://www.musicbrainz.org/ws/2/release/?query=%1").arg(searchQuery));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    new NetworkReplyWatcher(this, reply);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchAlbumFinished()));
}

void UniversalMusicScraper::onSearchAlbumFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("release").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("release").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("title").isEmpty())
                name = elem.elementsByTagName("title").at(0).toElement().text();
            else
                continue;

            QMap<QString, int> mediumList;
            if (!elem.elementsByTagName("medium-list").isEmpty()) {
                for (int i=0, n=elem.elementsByTagName("medium-list").count() ; i<n ; ++i) {
                    QDomElement mediumElem = elem.elementsByTagName("medium-list").at(i).toElement();
                    if (mediumElem.elementsByTagName("format").isEmpty())
                        continue;
                    QString medium = mediumElem.elementsByTagName("format").at(0).toElement().text();
                    mediumList.insert(medium, mediumList.value(medium, 0)+1);
                }
            }
            QStringList media;
            QMapIterator<QString, int> it(mediumList);
            while (it.hasNext()) {
                it.next();
                if (it.value() > 1)
                    media << QString("%1x%2").arg(it.value()).arg(it.key());
                else
                    media << it.key();
            }

            if (!media.isEmpty())
                name += QString(", %1").arg(media.join(" + "));

            if (!elem.elementsByTagName("disambiguation").isEmpty())
                name += QString(", %1").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text());

            if (!elem.elementsByTagName("date").isEmpty())
                name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());

            if (!elem.elementsByTagName("country").isEmpty())
                name += QString(" %1").arg(elem.elementsByTagName("country").at(0).toElement().text());

            ScraperSearchResult result;
            result.id = elem.attribute("id");
            result.name = name;
            if (!elem.elementsByTagName("release-group").isEmpty())
                result.id2 = elem.elementsByTagName("release-group").at(0).toElement().attribute("id");
            results.append(result);
        }
    }
    emit sigSearchDone(results);
}

void UniversalMusicScraper::loadData(QString mbAlbumId, QString mbReleaseGroupId, Album *album, QList<int> infos)
{
    album->clear(infos);
    album->setMbAlbumId(mbAlbumId);
    album->setMbReleaseGroupId(mbReleaseGroupId);
    album->setAllMusicId("");
    QUrl url(QString("http://www.musicbrainz.org/ws/2/release/%1?inc=url-rels+labels+artist-credits").arg(mbAlbumId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    new NetworkReplyWatcher(this, reply);
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onAlbumRelsFinished()));
}

void UniversalMusicScraper::onAlbumRelsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!album)
        return;

    QString discogsUrl;
    if (reply->error() == QNetworkReply::NoError) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignMusicbrainzInfos(msg, album, infos);
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("relation").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
            if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                QString url = elem.elementsByTagName("target").at(0).toElement().text();
                QRegExp rx("allmusic\\.com/album/(.*)$");
                if (rx.indexIn(url) != -1)
                    album->setAllMusicId(rx.cap(1));
            }
            if (elem.attribute("type") == "discogs" && elem.elementsByTagName("target").count() > 0)
                discogsUrl = elem.elementsByTagName("target").at(0).toElement().text();
        }
    } else {
        qDebug() << reply->errorString();
    }

    if (!m_albumDownloads.contains(album))
        m_albumDownloads.insert(album, QList<DownloadElement>());
    m_albumDownloads[album].clear();

    appendDownloadElement(album, "theaudiodb", "tadb_data", QUrl(QString("http://www.theaudiodb.com/api/v1/json/%1/album-mb.php?i=%2").arg(m_tadbApiKey).arg(album->mbReleaseGroupId())));
    if (!album->allMusicId().isEmpty())
        appendDownloadElement(album, "allmusic", "am_data", QString("http://www.allmusic.com/album/%1").arg(album->allMusicId()));
    if (!discogsUrl.isEmpty())
        appendDownloadElement(album, "discogs", "discogs_data", QUrl(discogsUrl));

    foreach (DownloadElement elem, m_albumDownloads[album]) {
        QNetworkRequest request(elem.url);
        request.setRawHeader("User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0");
        QNetworkReply *reply = qnam()->get(request);
        new NetworkReplyWatcher(this, reply);
        reply->setProperty("storage", Storage::toVariant(reply, album));
        reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
        connect(reply, SIGNAL(finished()), this, SLOT(onAlbumLoadFinished()));
    }
}

void UniversalMusicScraper::onAlbumLoadFinished()
{
    QMutexLocker locker(&m_albumMutex);

    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!album)
        return;

    if (!m_albumDownloads.contains(album))
        return;

    int index = -1;
    for (int i=0, n=m_albumDownloads[album].count() ; i<n ; ++i) {
        if (m_albumDownloads[album][i].url == reply->url()) {
            index = i;
            break;
        }
    }
    if (index == -1)
        return;

    if (reply->error() == QNetworkReply::NoError)
        m_albumDownloads[album][index].contents = QString::fromUtf8(reply->readAll());
    else
        qWarning() << "Network Error (load)" << reply->errorString();
    m_albumDownloads[album][index].downloaded = true;

    bool finished = true;
    for (int i=0, n=m_albumDownloads[album].count() ; i<n ; ++i) {
        if (!m_albumDownloads[album][i].downloaded) {
            finished = false;
            break;
        }
    }

    if (!finished)
        return;

    foreach (DownloadElement elem, m_albumDownloads[album]) {
        if (elem.source != m_prefer)
            continue;
        processDownloadElement(elem, album, infos);
    }
    foreach (DownloadElement elem, m_albumDownloads[album]) {
        if (elem.source == m_prefer)
            continue;
        processDownloadElement(elem, album, infos);
    }

    m_albumDownloads.remove(album);
    album->controller()->scraperLoadDone(this);
}

void UniversalMusicScraper::processDownloadElement(DownloadElement elem, Album *album, QList<int> infos)
{
    if (elem.type == "tadb_data")
        parseAndAssignTadbInfos(elem.contents, album, infos);
    else if (elem.type == "am_data")
        parseAndAssignAmInfos(elem.contents, album, infos);
    else if (elem.type == "discogs_data")
        parseAndAssignDiscogsInfos(elem.contents, album, infos);
}

void UniversalMusicScraper::parseAndAssignTadbInfos(QString json, Artist *artist, QList<int> infos)
{
    QScriptValue sc;
    QScriptValue scComplete;
    QScriptEngine engine;
    scComplete = engine.evaluate("(" + QString(json) + ")");

    QScriptValueIterator it(scComplete.property("artists"));
    while (it.hasNext()) {
        it.next();
        sc = it.value();
        break;
    }

    if (sc.property("strMusicBrainzID").isValid() && !sc.property("strMusicBrainzID").toString().isEmpty())
        artist->setMbId(sc.property("strMusicBrainzID").toString());

    if (shouldLoad(MusicScraperInfos::Name, infos, artist) && !sc.property("strArtist").isNull() && !sc.property("strArtist").toString().isEmpty())
        artist->setName(sc.property("strArtist").toString());

    if (shouldLoad(MusicScraperInfos::Died, infos, artist) && !sc.property("intDiedYear").isNull())
        artist->setDied(sc.property("intDiedYear").toString());

    if (shouldLoad(MusicScraperInfos::Formed, infos, artist) && !sc.property("intFormedYear").isNull())
        artist->setFormed(sc.property("intFormedYear").toString());

    if (shouldLoad(MusicScraperInfos::Born, infos, artist) && !sc.property("intBornYear").isNull())
        artist->setBorn(sc.property("intBornYear").toString());

    if (shouldLoad(MusicScraperInfos::Disbanded, infos, artist) && !sc.property("strDisbanded").isNull())
        artist->setDisbanded(sc.property("strDisbanded").toString());

    if (shouldLoad(MusicScraperInfos::Genres, infos, artist) && !sc.property("strGenre").isNull() && sc.property("strGenre").toString() != "...")
        artist->addGenre(sc.property("strGenre").toString());

    if (shouldLoad(MusicScraperInfos::Styles, infos, artist) && !sc.property("strStyle").isNull() && sc.property("strStyle").toString() != "...")
        artist->addStyle(sc.property("strStyle").toString());

    if (shouldLoad(MusicScraperInfos::Moods, infos, artist) && !sc.property("strMood").isNull() && sc.property("strMood").toString() != "...")
        artist->addMood(sc.property("strMood").toString());

    if (shouldLoad(MusicScraperInfos::Biography, infos, artist)) {
        if (!sc.property("strBiography" + m_language.toUpper()).isNull())
            artist->setBiography(sc.property("strBiography" + m_language.toUpper()).toString());
        else if (!sc.property("strBiographyEN").isNull())
            artist->setBiography(sc.property("strBiographyEN").toString());
    }
}

void UniversalMusicScraper::parseAndAssignTadbDiscography(QString json, Artist *artist, QList<int> infos)
{
    if (shouldLoad(MusicScraperInfos::Discography, infos, artist)) {
        QScriptValue sc;
        QScriptEngine engine;
        sc = engine.evaluate("(" + QString(json) + ")");
        if (!sc.property("album").isArray())
            return;

        QScriptValueIterator itC(sc.property("album"));
        while (itC.hasNext()) {
            itC.next();
            QScriptValue vC = itC.value();
            DiscographyAlbum a;
            a.title = vC.property("strAlbum").toString();
            a.year = vC.property("intYearReleased").toString();
            if (!a.title.isEmpty() || !a.year.isEmpty())
                artist->addDiscographyAlbum(a);
        }
    }
}

void UniversalMusicScraper::parseAndAssignAmInfos(QString html, Artist *artist, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (shouldLoad(MusicScraperInfos::Name, infos, artist)) {
        rx.setPattern("<h2 class=\"artist-name\" itemprop=\"name\">[\\n\\s]*(.*)[\\n\\s]*</h2>");
        if (rx.indexIn(html) != -1)
            artist->setName(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::YearsActive, infos, artist)) {
        rx.setPattern("<h4>Active</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1)
            artist->setYearsActive(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Formed, infos, artist)) {
        rx.setPattern("<h4>[\\n\\s]*Formed[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1)
            artist->setFormed(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Born, infos, artist)) {
        rx.setPattern("<h4>[\\n\\s]*Born[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1)
            artist->setBorn(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Died, infos, artist)) {
        rx.setPattern("<h4>[\\n\\s]*Died[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1)
            artist->setDied(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Disbanded, infos, artist)) {
        rx.setPattern("<h4>[\\n\\s]*Disbanded[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1)
            artist->setDisbanded(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Genres, infos, artist)) {
        rx.setPattern("<h4>[\\n\\s]*Genre[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString genres = rx.cap(1);
            rx.setPattern("<a[^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(genres, pos)) != -1) {
                artist->addGenre(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (shouldLoad(MusicScraperInfos::Styles, infos, artist)) {
        rx.setPattern("<h4>[\\n\\s]*Styles[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(styles, pos)) != -1) {
                artist->addStyle(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (shouldLoad(MusicScraperInfos::Moods, infos, artist)) {
        rx.setPattern("<h3 class=\"headline\">Artists Moods</h3>[\\n\\s]*<ul>(.*)</ul>");
        if (rx.indexIn(html) != -1) {
            QString moods = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(moods, pos)) != -1) {
                artist->addMood(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }
}

void UniversalMusicScraper::parseAndAssignAmBiography(QString html, Artist *artist, QList<int> infos)
{
    if (shouldLoad(MusicScraperInfos::Biography, infos, artist)) {
        QRegExp rx("<div class=\"text\" itemprop=\"reviewBody\">(.*)</div>");
        rx.setMinimal(true);
        if (rx.indexIn(html) != -1) {
            QString biography = rx.cap(1);
            biography.remove(QRegExp("<[^>]*>"));
            artist->setBiography(trim(biography));
        }
    }
}

void UniversalMusicScraper::parseAndAssignAmDiscography(QString html, Artist *artist, QList<int> infos)
{
    if (shouldLoad(MusicScraperInfos::Discography, infos, artist)) {
        QRegExp rx("<td class=\"year\" data\\-sort\\-value=\"[^\"]*\">[\\n\\s]*(.*)[\\n\\s]*</td>[\\n\\s]*<td class=\"title\" data\\-sort\\-value=\"(.*)\">");
        rx.setMinimal(true);
        int pos = 0;
        while ((pos = rx.indexIn(html, pos)) != -1) {
            DiscographyAlbum a;
            a.title = trim(rx.cap(2));
            a.year = trim(rx.cap(1));
            if (!a.title.isEmpty() || !a.year.isEmpty())
                artist->addDiscographyAlbum(a);
            pos += rx.matchedLength();
        }
    }
}

void UniversalMusicScraper::parseAndAssignDiscogsInfos(QString html, Artist *artist, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (shouldLoad(MusicScraperInfos::Name, infos, artist)) {
        rx.setPattern("<div class=\"body\">[\\n\\s]*<h1 class=\"hide_desktop\">(.*)</h1>");
        if (rx.indexIn(html) != -1)
            artist->setName(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Biography, infos, artist)) {
        rx.setPattern("<div [^>]* id=\"profile\">[\\n\\s]*(.*)[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1)
            artist->setBiography(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Discography, infos, artist)) {
        rx.setPattern("<table [^>]* id=\"artist\">(.*)</table>");
        if (rx.indexIn(html) != -1) {
            QString table = rx.cap(1);
            rx.setPattern("<tr[^>]*data\\-object\\-id=\"[^\"]*\"[^>]*>(.*)</tr>");
            int pos = 0;
            while ((pos = rx.indexIn(table, pos)) != -1) {
                QRegExp rx2("<td class=\"title\"[^>]*>.*<a href=\"[^\"]*\">(.*)</a>.*</td>");
                rx2.setMinimal(true);

                DiscographyAlbum a;
                if (rx2.indexIn(rx.cap(1)) != -1)
                    a.title = trim(rx2.cap(1));

                rx2.setPattern("<td class=\"year has_header\" data\\-header=\"Year: \">(.*)</td>");
                if (rx2.indexIn(rx.cap(1)) != -1)
                    a.year = trim(rx2.cap(1));

                if (a.title != "" || a.year != "")
                    artist->addDiscographyAlbum(a);

                pos += rx.matchedLength();
            }
        }
    }
}

void UniversalMusicScraper::parseAndAssignMusicbrainzInfos(QString xml, Album *album, QList<int> infos)
{
    QDomDocument domDoc;
    domDoc.setContent(xml);

    if (shouldLoad(MusicScraperInfos::Title, infos, album) && !domDoc.elementsByTagName("title").isEmpty())
        album->setTitle(domDoc.elementsByTagName("title").at(0).toElement().text());

    if (shouldLoad(MusicScraperInfos::Artist, infos, album) && !domDoc.elementsByTagName("artist-credit").isEmpty()) {
        QString artist;
        QString joinPhrase;
        QDomNodeList artistList = domDoc.elementsByTagName("artist-credit").at(0).toElement().elementsByTagName("name-credit");
        for (int i=0, n=artistList.count() ; i<n ; ++i) {
            QDomElement artistElem = artistList.at(i).toElement();
            joinPhrase = artistElem.attribute("joinphrase");
            if (artistElem.elementsByTagName("artist").isEmpty())
                continue;
            if (artistElem.elementsByTagName("artist").at(0).toElement().elementsByTagName("name").isEmpty())
                continue;

            if (!artist.isEmpty())
                artist.append(joinPhrase.isEmpty() ? ", " : joinPhrase);
            artist.append(artistElem.elementsByTagName("artist").at(0).toElement().elementsByTagName("name").at(0).toElement().text());
        }
        if (!artist.isEmpty())
            album->setArtist(artist);
    }

    if (shouldLoad(MusicScraperInfos::Label, infos, album) && !domDoc.elementsByTagName("label-info-list").isEmpty()) {
        QStringList labels;
        QDomNodeList labelList = domDoc.elementsByTagName("label-info-list").at(0).toElement().elementsByTagName("label-info");
        for (int i=0, n=labelList.count() ; i<n ; ++i) {
            QDomElement labelElem = labelList.at(i).toElement();
            if (labelElem.elementsByTagName("label").isEmpty())
                continue;
            if (labelElem.elementsByTagName("label").at(0).toElement().elementsByTagName("name").isEmpty())
                continue;
            labels << labelElem.elementsByTagName("label").at(0).toElement().elementsByTagName("name").at(0).toElement().text();
        }
        if (!labels.isEmpty())
            album->setLabel(labels.join(", "));
    }

    if (shouldLoad(MusicScraperInfos::ReleaseDate, infos, album) && !domDoc.elementsByTagName("release-event-list").isEmpty()) {
        QDomNodeList releaseList = domDoc.elementsByTagName("release-event-list").at(0).toElement().elementsByTagName("release-event");
        if (!releaseList.isEmpty() && !releaseList.at(0).toElement().elementsByTagName("date").isEmpty())
            album->setReleaseDate(releaseList.at(0).toElement().elementsByTagName("date").at(0).toElement().text());
    }
}

void UniversalMusicScraper::parseAndAssignTadbInfos(QString json, Album *album, QList<int> infos)
{
    QScriptValue sc;
    QScriptValue scComplete;
    QScriptEngine engine;
    scComplete = engine.evaluate("(" + QString(json) + ")");

    QScriptValueIterator it(scComplete.property("album"));
    while (it.hasNext()) {
        it.next();
        sc = it.value();
        break;
    }

    if (sc.property("strMusicBrainzID").isValid() && !sc.property("strMusicBrainzID").toString().isEmpty())
        album->setMbReleaseGroupId(sc.property("strMusicBrainzID").toString());

    if (shouldLoad(MusicScraperInfos::Title, infos, album) && !sc.property("strAlbum").isNull() && !sc.property("strAlbum").toString().isEmpty())
        album->setTitle(sc.property("strAlbum").toString());

    if (shouldLoad(MusicScraperInfos::Artist, infos, album) && !sc.property("strArtist").isNull() && !sc.property("strArtist").toString().isEmpty())
        album->setArtist(sc.property("strArtist").toString());

    if (shouldLoad(MusicScraperInfos::Rating, infos, album) && !sc.property("intScore").isNull())
        album->setRating(sc.property("intScore").toString().toInt());

    if (shouldLoad(MusicScraperInfos::Year, infos, album) && !sc.property("intYearReleased").isNull() && sc.property("intYearReleased").toInt32() > 0)
        album->setYear(sc.property("intYearReleased").toString().toInt());

    if (shouldLoad(MusicScraperInfos::Genres, infos, album) && !sc.property("strGenre").isNull() && sc.property("strGenre").toString() != "...")
        album->addGenre(sc.property("strGenre").toString());

    if (shouldLoad(MusicScraperInfos::Styles, infos, album) && !sc.property("strStyle").isNull() && sc.property("strStyle").toString() != "...")
        album->addStyle(sc.property("strStyle").toString());

    if (shouldLoad(MusicScraperInfos::Moods, infos, album) && !sc.property("strMood").isNull() && sc.property("strMood").toString() != "...")
        album->addMood(sc.property("strMood").toString());

    if (shouldLoad(MusicScraperInfos::Review, infos, album)) {
        if (!sc.property("strDescription" + m_language.toUpper()).isNull())
            album->setReview(sc.property("strDescription" + m_language.toUpper()).toString());
        else if (!sc.property("strDescriptionEN").isNull())
            album->setReview(sc.property("strDescriptionEN").toString());
    }
}

void UniversalMusicScraper::parseAndAssignAmInfos(QString html, Album *album, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (shouldLoad(MusicScraperInfos::Title, infos, album)) {
        rx.setPattern("<h2 class=\"album-name\" itemprop=\"name\">[\\n\\s]*(.*)[\\n\\s]*</h2>");
        if (rx.indexIn(html) != -1)
            album->setTitle(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Artist, infos, album)) {
        rx.setPattern("<h3 class=\"album-artist\"[^>]*>[\\n\\s]*<span itemprop=\"name\">[\\n\\s]*<a [^>]*>(.*)</a>");
        if (rx.indexIn(html) != -1)
            album->setArtist(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Review, infos, album)) {
        rx.setPattern("<div class=\"text\" itemprop=\"reviewBody\">(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString review = rx.cap(1);
            review.remove(QRegExp("<[^>]*>"));
            album->setReview(trim(review));
        }
    }

    if (shouldLoad(MusicScraperInfos::ReleaseDate, infos, album)) {
        rx.setPattern("<h4>[\\n\\s]*Release Date[\\n\\s]*</h4>[\\n\\s]*<span>(.*)</span>");
        if (rx.indexIn(html) != -1)
            album->setReleaseDate(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Rating, infos, album)) {
        rx.setPattern("<div class=\"allmusic-rating rating-allmusic-\\d\" itemprop=\"ratingValue\">[\\n\\s]*(\\d)[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1)
            album->setRating(rx.cap(1).toFloat());
    }

    if (shouldLoad(MusicScraperInfos::Year, infos, album)) {
        rx.setPattern("<h4>[\\n\\s]*Release Date[\\n\\s]*</h4>[\\n\\s]*<span>.*([0-9]{4}).*</span>");
        if (rx.indexIn(html) != -1)
            album->setYear(rx.cap(1).toInt());
    }

    if (shouldLoad(MusicScraperInfos::Genres, infos, album)) {
        rx.setPattern("<h4>[\\n\\s]*Genre[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString genres = rx.cap(1);
            rx.setPattern("<a[^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(genres, pos)) != -1) {
                album->addGenre(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (shouldLoad(MusicScraperInfos::Styles, infos, album)) {
        rx.setPattern("<h4>[\\n\\s]*Styles[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern("<a [^>]*>([^<]*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(styles, pos)) != -1) {
                album->addStyle(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (shouldLoad(MusicScraperInfos::Moods, infos, album)) {
        rx.setPattern("<h4>Album Moods</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString moods = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(moods, pos)) != -1) {
                album->addMood(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }
}

void UniversalMusicScraper::parseAndAssignDiscogsInfos(QString html, Album *album, QList<int> infos)
{
    QRegExp rx;
    rx.setMinimal(true);

    if (shouldLoad(MusicScraperInfos::Artist, infos, album)) {
        rx.setPattern("<span itemprop=\"byArtist\" itemscope itemtype=\"http://schema.org/MusicGroup\">[\\n\\s]*<span itemprop=\"name\" title=\"(.*)\" >");
        if (rx.indexIn(html) != -1)
            album->setArtist(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Title, infos, album)) {
        rx.setPattern("<span itemprop=\"name\">[\\n\\s]*(.*)[\\n\\s]*</span>");
        if (rx.indexIn(html) != -1)
            album->setTitle(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Genres, infos, album)) {
        rx.setPattern("<div class=\"content\" itemprop=\"genre\">[\\n\\s]*(.*)[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1) {
            QString genres = rx.cap(1);
            rx.setPattern("<a href=\"[^\"]*\">([^<]*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(genres, pos)) != -1) {
                album->addGenre(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (shouldLoad(MusicScraperInfos::Styles, infos, album)) {
        rx.setPattern("<div class=\"head\">Style:</div>[\\n\\s]*<div class=\"content\">[\\n\\s]*(.*)[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern("<a href=\"[^\"]*\">(.*)</a>");
            int pos = 0;
            while ((pos = rx.indexIn(styles, pos)) != -1) {
                album->addStyle(trim(rx.cap(1)));
                pos += rx.matchedLength();
            }
        }
    }

    if (shouldLoad(MusicScraperInfos::Year, infos, album)) {
        rx.setPattern("<div class=\"head\">Year:</div>[\\n\\s]*<div class=\"content\">[\\n\\s]*<a href=\"[^\"]*\">(.*)</a>[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1)
            album->setYear(trim(rx.cap(1)).toInt());
    }
}

bool UniversalMusicScraper::hasSettings()
{
    return true;
}

void UniversalMusicScraper::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/UniversalMusicScraper/Language", "en").toString();
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language)
            m_box->setCurrentIndex(i);
    }
    m_prefer = settings.value("Scrapers/UniversalMusicScraper/Prefer", "theaudiodb").toString();
    for (int i=0, n=m_preferBox->count() ; i<n ; ++i) {
        if (m_preferBox->itemData(i).toString() == m_prefer)
            m_preferBox->setCurrentIndex(i);
    }
}

void UniversalMusicScraper::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setValue("Scrapers/UniversalMusicScraper/Language", m_language);
    m_prefer = m_preferBox->itemData(m_preferBox->currentIndex()).toString();
    settings.setValue("Scrapers/UniversalMusicScraper/Prefer", m_prefer);
}

QList<int> UniversalMusicScraper::scraperSupports()
{
    return QList<int>() << MusicScraperInfos::Name
                        << MusicScraperInfos::Genres
                        << MusicScraperInfos::Styles
                        << MusicScraperInfos::Moods
                        << MusicScraperInfos::Formed
                        << MusicScraperInfos::Born
                        << MusicScraperInfos::Died
                        << MusicScraperInfos::Disbanded
                        << MusicScraperInfos::Biography
                        << MusicScraperInfos::Thumb
                        << MusicScraperInfos::Fanart
                        << MusicScraperInfos::Logo
                        << MusicScraperInfos::Title
                        << MusicScraperInfos::Artist
                        << MusicScraperInfos::Review
                        << MusicScraperInfos::Rating
                        << MusicScraperInfos::Year
                        << MusicScraperInfos::CdArt
                        << MusicScraperInfos::Cover
                        << MusicScraperInfos::YearsActive
                        << MusicScraperInfos::ReleaseDate
                        << MusicScraperInfos::Year
                        << MusicScraperInfos::ExtraFanarts
                        << MusicScraperInfos::Discography
                        << MusicScraperInfos::Label;
}

QWidget *UniversalMusicScraper::settingsWidget()
{
    return m_widget;
}

QString UniversalMusicScraper::trim(QString text)
{
    return text.replace(QRegExp("\\s{1,}"), " ").trimmed();
}

bool UniversalMusicScraper::shouldLoad(int info, QList<int> infos, Album *album)
{
    if (!infos.contains(info))
        return false;

    switch (info) {
    case MusicScraperInfos::Title:
        return album->title().isEmpty();
    case MusicScraperInfos::Artist:
        return album->artist().isEmpty();
    case MusicScraperInfos::Review:
        return album->review().isEmpty();
    case MusicScraperInfos::ReleaseDate:
        return album->releaseDate().isEmpty();
    case MusicScraperInfos::Label:
        return album->label().isEmpty();
    case MusicScraperInfos::Rating:
        return album->rating() == 0;
    case MusicScraperInfos::Year:
        return album->year() == 0;
    case MusicScraperInfos::Genres:
        return album->genres().isEmpty();
    case MusicScraperInfos::Styles:
        return album->styles().isEmpty();
    case MusicScraperInfos::Moods:
        return album->moods().isEmpty();
    default:
        break;
    }

    return false;
}

bool UniversalMusicScraper::shouldLoad(int info, QList<int> infos, Artist *artist)
{
    if (!infos.contains(info))
        return false;

    switch (info) {
    case MusicScraperInfos::Name:
        return artist->name().isEmpty();
    case MusicScraperInfos::YearsActive:
        return artist->yearsActive().isEmpty();
    case MusicScraperInfos::Formed:
        return artist->formed().isEmpty();
    case MusicScraperInfos::Born:
        return artist->born().isEmpty();
    case MusicScraperInfos::Died:
        return artist->died().isEmpty();
    case MusicScraperInfos::Disbanded:
        return artist->disbanded().isEmpty();
    case MusicScraperInfos::Biography:
        return artist->biography().isEmpty();
    case MusicScraperInfos::Genres:
        return artist->genres().isEmpty();
    case MusicScraperInfos::Styles:
        return artist->styles().isEmpty();
    case MusicScraperInfos::Moods:
        return artist->moods().isEmpty();
    case MusicScraperInfos::Discography:
        return artist->discographyAlbums().isEmpty();
    default:
        break;
    }

    return false;
}

bool UniversalMusicScraper::infosLeft(QList<int> infos, Album *album)
{
    foreach (int info, infos) {
        if (shouldLoad(info, infos, album))
            return true;
    }
    return false;
}

bool UniversalMusicScraper::infosLeft(QList<int> infos, Artist *artist)
{
    foreach (int info, infos) {
        if (shouldLoad(info, infos, artist))
            return true;
    }
    return false;
}

void UniversalMusicScraper::appendDownloadElement(Artist *artist, QString source, QString type, QUrl url)
{
    DownloadElement elem;
    elem.type = type;
    elem.url = url;
    elem.downloaded = false;
    elem.source = source;
    if (!m_artistDownloads.contains(artist))
        m_artistDownloads.insert(artist, QList<DownloadElement>());
    m_artistDownloads[artist].append(elem);
}

void UniversalMusicScraper::appendDownloadElement(Album *album, QString source, QString type, QUrl url)
{
    DownloadElement elem;
    elem.type = type;
    elem.url = url;
    elem.downloaded = false;
    elem.source = source;
    if (!m_albumDownloads.contains(album))
        m_albumDownloads.insert(album, QList<DownloadElement>());
    m_albumDownloads[album].append(elem);
}
