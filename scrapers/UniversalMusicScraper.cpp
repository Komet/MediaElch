#include "UniversalMusicScraper.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGridLayout>
#include <QLabel>
#include <QSettings>
#include <QtScript/QScriptValue>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngine>

#include "../data/Storage.h"
#include "../main/MainWindow.h"

UniversalMusicScraper::UniversalMusicScraper(QObject *parent)
{
    setParent(parent);
    m_tadbApiKey = "7490823590829082posuda";

    m_language = "en";
    m_prefer = "theaudiodb";
    m_lastScraper = "allmusic";
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

void UniversalMusicScraper::searchAlbum(QString artistName, QString searchStr)
{
    QString searchQuery = "release:" + QString(QUrl::toPercentEncoding(searchStr));
    if (!artistName.isEmpty())
        searchQuery += "%20AND%20artist:" + QString(QUrl::toPercentEncoding(artistName));
    QUrl url(QString("http://www.musicbrainz.org/ws/2/release/?query=%1").arg(searchQuery));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchAlbumFinished()));
}

void UniversalMusicScraper::onSearchAlbumFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        QStringList searchIds;
        for (int i=0, n=domDoc.elementsByTagName("release").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("release").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("title").isEmpty())
                name = elem.elementsByTagName("title").at(0).toElement().text();
            else
                continue;

            if (!elem.elementsByTagName("date").isEmpty())
                name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());

            for (int x=0, y=elem.elementsByTagName("release-group").count() ; x<y ; ++x) {
                QDomElement releaseGroupElem = elem.elementsByTagName("release-group").at(x).toElement();
                if (!releaseGroupElem.attribute("id").isEmpty()) {
                    ScraperSearchResult result;
                    result.id = releaseGroupElem.attribute("id");
                    result.name = name;
                    if (!searchIds.contains(result.id)) {
                        results.append(result);
                        searchIds.append(result.id);
                    }
                }
            }
        }
    }
    emit sigSearchDone(results);
}

void UniversalMusicScraper::searchArtist(QString searchStr)
{
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/?query=artist:%1").arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchArtistFinished()));
}

void UniversalMusicScraper::onSearchArtistFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError ) {
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
    artist->clear(infos);
    artist->setMbId(mbId);
    artist->setAllMusicId("");
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/%1?inc=url-rels").arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onArtistRelsFinished()));
}

void UniversalMusicScraper::loadData(QString mbId, Album *album, QList<int> infos)
{
    album->clear(infos);
    album->setMbId(mbId);
    album->setAllMusicId("");
    QUrl url(QString("http://www.musicbrainz.org/ws/2/release-group/%1?inc=url-rels").arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onAlbumRelsFinished()));
}

void UniversalMusicScraper::loadTadbData(QString mbId, Artist *artist, QList<int> infos)
{
    QUrl url(QString("http://www.theaudiodb.com/api/v1/json/%1/artist-mb.php?i=%2").arg(m_tadbApiKey).arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onTadbArtistLoadFinished()));
}

void UniversalMusicScraper::loadTadbData(QString mbId, Album *album, QList<int> infos)
{
    QUrl url(QString("http://www.theaudiodb.com/api/v1/json/%1/album-mb.php?i=%2").arg(m_tadbApiKey).arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onTadbAlbumLoadFinished()));
}

void UniversalMusicScraper::onArtistRelsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("relation").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
            if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                QString url = elem.elementsByTagName("target").at(0).toElement().text();
                QRegExp rx("allmusic\\.com/artist/(.*)$");
                if (rx.indexIn(url) != -1) {
                    artist->setAllMusicId(rx.cap(1));
                    break;
                }
            }
        }
    }

    if (m_prefer == "allmusic")
        loadAmData(artist->allMusicId(), artist, infos);
    else
        loadTadbData(artist->mbId(), artist, infos);
}

void UniversalMusicScraper::onAlbumRelsFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!album)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        QDomDocument domDoc;
        domDoc.setContent(msg);
        for (int i=0, n=domDoc.elementsByTagName("relation").count() ; i<n ; ++i) {
            QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
            if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                QString url = elem.elementsByTagName("target").at(0).toElement().text();
                QRegExp rx("allmusic\\.com/album/(.*)$");
                if (rx.indexIn(url) != -1) {
                    album->setAllMusicId(rx.cap(1));
                    break;
                }
            }
        }
    }

    if (m_prefer == "allmusic")
        loadAmData(album->allMusicId(), album, infos);
    else
        loadTadbData(album->mbId(), album, infos);
}

void UniversalMusicScraper::onTadbArtistLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignTadbInfos(msg, artist, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    if (m_lastScraper == "theaudiodb" || !infosLeft(infos, artist))
        artist->controller()->scraperLoadDone(this);
    else
        loadAmData(artist->allMusicId(), artist, infos);
}

void UniversalMusicScraper::onTadbAlbumLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!album)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignTadbInfos(msg, album, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    if (m_lastScraper == "theaudiodb" || !infosLeft(infos, album))
        album->controller()->scraperLoadDone(this);
    else
        loadAmData(album->allMusicId(), album, infos);
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
        album->setMbId(sc.property("strMusicBrainzID").toString());

    if (shouldLoad(MusicScraperInfos::Title, infos, album) && !sc.property("strAlbum").isNull() && !sc.property("strAlbum").toString().isEmpty())
        album->setTitle(sc.property("strAlbum").toString());

    if (shouldLoad(MusicScraperInfos::Artist, infos, album) && !sc.property("strArtist").isNull())
        album->setArtist(sc.property("strArtist").toString());

    if (shouldLoad(MusicScraperInfos::Rating, infos, album) && !sc.property("intScore").isNull())
        album->setRating(sc.property("intScore").toString().toInt());

    if (shouldLoad(MusicScraperInfos::Year, infos, album) && !sc.property("intYearReleased").isNull() && sc.property("intYearReleased").toInt32() > 0)
        album->setYear(sc.property("intYearReleased").toString().toInt());

    if (shouldLoad(MusicScraperInfos::Genres, infos, album) && !sc.property("strGenre").isNull())
        album->addGenre(sc.property("strGenre").toString());

    if (shouldLoad(MusicScraperInfos::Styles, infos, album) && !sc.property("strStyle").isNull())
        album->addStyle(sc.property("strStyle").toString());

    if (shouldLoad(MusicScraperInfos::Moods, infos, album) && !sc.property("strMood").isNull())
        album->addMood(sc.property("strMood").toString());

    if (shouldLoad(MusicScraperInfos::Review, infos, album)) {
        if (!sc.property("strDescription" + m_language.toUpper()).isNull())
            album->setReview(sc.property("strDescription" + m_language.toUpper()).toString());
        else if (!sc.property("strDescriptionEN").isNull())
            album->setReview(sc.property("strDescriptionEN").toString());
    }
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

    if (shouldLoad(MusicScraperInfos::Genres, infos, artist) && !sc.property("strGenre").isNull())
        artist->addGenre(sc.property("strGenre").toString());

    if (shouldLoad(MusicScraperInfos::Styles, infos, artist) && !sc.property("strStyle").isNull())
        artist->addStyle(sc.property("strStyle").toString());

    if (shouldLoad(MusicScraperInfos::Moods, infos, artist) && !sc.property("strMood").isNull())
        artist->addMood(sc.property("strMood").toString());

    if (shouldLoad(MusicScraperInfos::Biography, infos, artist)) {
        if (!sc.property("strBiography" + m_language.toUpper()).isNull())
            artist->setBiography(sc.property("strBiography" + m_language.toUpper()).toString());
        else if (!sc.property("strBiographyEN").isNull())
            artist->setBiography(sc.property("strBiographyEN").toString());
    }
}

void UniversalMusicScraper::loadAmData(QString allMusicId, Album *album, QList<int> infos)
{
    QUrl url(QString("http://www.allmusic.com/album/%1").arg(allMusicId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onAmAlbumLoadFinished()));
}

void UniversalMusicScraper::loadAmData(QString allMusicId, Artist *artist, QList<int> infos)
{
    QUrl url(QString("http://www.allmusic.com/artist/%1").arg(allMusicId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onAmArtistLoadFinished()));
}

void UniversalMusicScraper::loadAmBiography(QString allMusicId, Artist *artist, QList<int> infos)
{
    QUrl url(QString("http://www.allmusic.com/artist/%1/biography").arg(allMusicId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onAmBiographyLoadFinished()));
}

void UniversalMusicScraper::onAmAlbumLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!album)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignAmInfos(msg, album, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    if (m_lastScraper == "allmusic" || !infosLeft(infos, album))
        album->controller()->scraperLoadDone(this);
    else
        loadTadbData(album->mbId(), album, infos);
}

void UniversalMusicScraper::onAmArtistLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignAmInfos(msg, artist, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    if (shouldLoad(MusicScraperInfos::Biography, infos, artist))
        loadAmBiography(artist->allMusicId(), artist, infos);
    else if (m_lastScraper == "allmusic" || !infosLeft(infos, artist))
        artist->controller()->scraperLoadDone(this);
    else
        loadTadbData(artist->mbId(), artist, infos);
}

void UniversalMusicScraper::onAmBiographyLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignAmBiography(msg, artist, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }

    if (m_lastScraper == "allmusic" || !infosLeft(infos, artist))
        artist->controller()->scraperLoadDone(this);
    else
        loadTadbData(artist->mbId(), artist, infos);
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
        rx.setPattern("<h4>[\\n\\s]*Genre[\\n\\s]*</h4>[\\n\\s]*<div>[\\n\\s]*<a[^>]*>(.*)</a>[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1)
            album->addGenre(trim(rx.cap(1)));
    }

    if (shouldLoad(MusicScraperInfos::Styles, infos, album)) {
        rx.setPattern("<h4>[\\n\\s]*Styles[\\n\\s]*</h4>[\\n\\s]*<div>(.*)</div>");
        if (rx.indexIn(html) != -1) {
            QString styles = rx.cap(1);
            rx.setPattern("<a [^>]*>(.*)</a>");
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
        rx.setPattern("<h4>[\\n\\s]*Genre[\\n\\s]*</h4>[\\n\\s]*<div>[\\n\\s]*<a[^>]*>(.*)</a>[\\n\\s]*</div>");
        if (rx.indexIn(html) != -1)
            artist->addGenre(trim(rx.cap(1)));
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
    m_lastScraper = (m_prefer == "theaudiodb") ? "allmusic" : "theaudiodb";
}

void UniversalMusicScraper::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setValue("Scrapers/UniversalMusicScraper/Language", m_language);
    m_prefer = m_preferBox->itemData(m_preferBox->currentIndex()).toString();
    settings.setValue("Scrapers/UniversalMusicScraper/Prefer", m_prefer);
    m_lastScraper = (m_prefer == "theaudiodb") ? "allmusic" : "theaudiodb";
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
                        << MusicScraperInfos::ExtraFanarts;
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
