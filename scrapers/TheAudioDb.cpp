#include "TheAudioDb.h"

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

TheAudioDb::TheAudioDb(QObject *parent)
{
    setParent(parent);
    m_apiKey = "7490823590829082posuda";

    m_language = "en";
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
    QGridLayout *layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

QNetworkAccessManager *TheAudioDb::qnam()
{
    return &m_qnam;
}

QString TheAudioDb::name()
{
    return QString("TheAudioDB");
}

QString TheAudioDb::identifier()
{
    return QString("theaudiodb");
}

void TheAudioDb::searchAlbum(QString artistName, QString searchStr)
{
    QString searchQuery = "release:" + QString(QUrl::toPercentEncoding(searchStr));
    if (!artistName.isEmpty())
        searchQuery += "%20AND%20artist:" + QString(QUrl::toPercentEncoding(artistName));
    QUrl url(QString("http://www.musicbrainz.org/ws/2/release/?query=%1").arg(searchQuery));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchAlbumFinished()));
}

void TheAudioDb::onSearchAlbumFinished()
{
    QList<ScraperSearchResult> results;
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    reply->deleteLater();
    if (reply->error() == QNetworkReply::NoError ) {
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

            if (!elem.elementsByTagName("date").isEmpty())
                name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());

            for (int x=0, y=elem.elementsByTagName("release-group").count() ; x<y ; ++x) {
                QDomElement releaseGroupElem = elem.elementsByTagName("release-group").at(x).toElement();
                if (!releaseGroupElem.attribute("id").isEmpty()) {
                    ScraperSearchResult result;
                    result.id = releaseGroupElem.attribute("id");
                    result.name = name;
                    results.append(result);
                }
            }
        }
    }
    emit sigSearchDone(results);
}

void TheAudioDb::searchArtist(QString searchStr)
{
    QUrl url(QString("http://www.musicbrainz.org/ws/2/artist/?query=artist:%1").arg(QString(QUrl::toPercentEncoding(searchStr))));
    QNetworkRequest request(url);
    QNetworkReply *reply = qnam()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onSearchArtistFinished()));
}

void TheAudioDb::onSearchArtistFinished()
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

void TheAudioDb::loadData(QString mbId, Artist *artist, QList<int> infos)
{
    artist->clear(infos);
    artist->setMbId(mbId);
    QUrl url(QString("http://www.theaudiodb.com/api/v1/json/%1/artist-mb.php?i=%2").arg(m_apiKey).arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, artist));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onArtistLoadFinished()));
}

void TheAudioDb::loadData(QString mbId, Album *album, QList<int> infos)
{
    album->clear(infos);
    album->setMbId(mbId);
    QUrl url(QString("http://www.theaudiodb.com/api/v1/json/%1/album-mb.php?i=%2").arg(m_apiKey).arg(mbId));
    QNetworkReply *reply = qnam()->get(QNetworkRequest(url));
    reply->setProperty("storage", Storage::toVariant(reply, album));
    reply->setProperty("infosToLoad", Storage::toVariant(reply, infos));
    connect(reply, SIGNAL(finished()), this, SLOT(onAlbumLoadFinished()));
}

void TheAudioDb::onArtistLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Artist *artist = reply->property("storage").value<Storage*>()->artist();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!artist)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, artist, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    artist->controller()->scraperLoadDone(this);
}

void TheAudioDb::onAlbumLoadFinished()
{
    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());
    Album *album = reply->property("storage").value<Storage*>()->album();
    QList<int> infos = reply->property("infosToLoad").value<Storage*>()->infosToLoad();
    reply->deleteLater();
    if (!album)
        return;

    if (reply->error() == QNetworkReply::NoError ) {
        QString msg = QString::fromUtf8(reply->readAll());
        parseAndAssignInfos(msg, album, infos);
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    album->controller()->scraperLoadDone(this);
}

void TheAudioDb::parseAndAssignInfos(QString json, Album *album, QList<int> infos)
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
        album->setMbId(sc.property("strMusicBrainzID").toString());
    if (infos.contains(MusicScraperInfos::Title) && !sc.property("strAlbum").isNull())
        album->setTitle(sc.property("strAlbum").toString());
    if (infos.contains(MusicScraperInfos::Artist) && !sc.property("strArtist").isNull())
        album->setArtist(sc.property("strArtist").toString());
    if (infos.contains(MusicScraperInfos::Rating) && !sc.property("intScore").isNull())
        album->setRating(sc.property("intScore").toString().toInt());
    if (infos.contains(MusicScraperInfos::Year) && !sc.property("intYearReleased").isNull() && sc.property("intYearReleased").toInt32() > 0)
        album->setYear(sc.property("intYearReleased").toString().toInt());
    if (infos.contains(MusicScraperInfos::Genres) && !sc.property("strGenre").isNull())
        album->addGenre(sc.property("strGenre").toString());
    if (infos.contains(MusicScraperInfos::Styles) && !sc.property("strStyle").isNull())
        album->addStyle(sc.property("strStyle").toString());
    if (infos.contains(MusicScraperInfos::Moods) && !sc.property("strMood").isNull())
        album->addMood(sc.property("strMood").toString());
    if (infos.contains(MusicScraperInfos::Review)) {
        if (!sc.property("strDescription" + m_language.toUpper()).toString().isEmpty())
            album->setReview(sc.property("strDescription" + m_language.toUpper()).toString());
        else
            album->setReview(sc.property("strDescriptionEN").toString());
    }
}

void TheAudioDb::parseAndAssignInfos(QString json, Artist *artist, QList<int> infos)
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
    if (infos.contains(MusicScraperInfos::Name) && !sc.property("strArtist").isNull())
        artist->setName(sc.property("strArtist").toString());
    if (infos.contains(MusicScraperInfos::Year) && !sc.property("intDiedYear").isNull())
        artist->setDied(sc.property("intDiedYear").toString());
    if (infos.contains(MusicScraperInfos::Formed) && !sc.property("intFormedYear").isNull())
        artist->setFormed(sc.property("intFormedYear").toString());
    if (infos.contains(MusicScraperInfos::Died) && !sc.property("intBornYear").isNull())
        artist->setBorn(sc.property("intBornYear").toString());
    if (infos.contains(MusicScraperInfos::Disbanded) && !sc.property("strDisbanded").isNull())
        artist->setDisbanded(sc.property("strDisbanded").toString());
    if (infos.contains(MusicScraperInfos::Genres) && !sc.property("strGenre").isNull())
        artist->addGenre(sc.property("strGenre").toString());
    if (infos.contains(MusicScraperInfos::Styles) && !sc.property("strStyle").isNull())
        artist->addStyle(sc.property("strStyle").toString());
    if (infos.contains(MusicScraperInfos::Moods) && !sc.property("strMood").isNull())
        artist->addMood(sc.property("strMood").toString());
    if (infos.contains(MusicScraperInfos::Biography)) {
        if (!sc.property("strDescription" + m_language.toUpper()).toString().isEmpty())
            artist->setBiography(sc.property("strBiography" + m_language.toUpper()).toString());
        else
            artist->setBiography(sc.property("strBiographyEN").toString());
    }
}

bool TheAudioDb::hasSettings()
{
    return true;
}

void TheAudioDb::loadSettings(QSettings &settings)
{
    m_language = settings.value("Scrapers/TheAudioDb/Language", "en").toString();
    for (int i=0, n=m_box->count() ; i<n ; ++i) {
        if (m_box->itemData(i).toString() == m_language)
            m_box->setCurrentIndex(i);
    }
}

void TheAudioDb::saveSettings(QSettings &settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setValue("Scrapers/TheAudioDb/Language", m_language);
}

QList<int> TheAudioDb::scraperSupports()
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
                        << MusicScraperInfos::Cover;
}

QWidget *TheAudioDb::settingsWidget()
{
    return m_widget;
}
