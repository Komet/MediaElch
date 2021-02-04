#include "UniversalMusicScraper.h"

#include "data/Storage.h"
#include "ui/main/MainWindow.h"

#include <QDomDocument>
#include <QDomElement>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLabel>
#include <QMutexLocker>

namespace mediaelch {
namespace scraper {

UniversalMusicScraper::UniversalMusicScraper(QObject* parent)
{
    setParent(parent);

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
    m_preferBox->addItem(tr("MusicBrainz"), "musicbrainz");
    m_preferBox->addItem(tr("AllMusic"), "allmusic");
    m_preferBox->addItem(tr("Discogs"), "discogs");
    auto* layout = new QGridLayout(m_widget);
    layout->addWidget(new QLabel(tr("Language")), 0, 0);
    layout->addWidget(m_box, 0, 1);
    layout->addWidget(new QLabel(tr("Prefer")), 1, 0);
    layout->addWidget(m_preferBox, 1, 1);
    layout->setColumnStretch(2, 1);
    layout->setContentsMargins(12, 0, 12, 12);
    m_widget->setLayout(layout);
}

mediaelch::network::NetworkManager* UniversalMusicScraper::network()
{
    return &m_network;
}

QString UniversalMusicScraper::name() const
{
    return QString("Universal Music Scraper");
}

QString UniversalMusicScraper::identifier() const
{
    return ID;
}

void UniversalMusicScraper::searchArtist(QString searchStr)
{
    m_musicBrainzApi.searchForArtist(m_language, searchStr, [this](QString html, ScraperError error) {
        if (error.hasError()) {
            emit sigSearchDone({});
            return;
        }

        QVector<ScraperSearchResult> results;
        QDomDocument domDoc;
        domDoc.setContent(html);
        for (int i = 0, n = domDoc.elementsByTagName("artist").count(); i < n; ++i) {
            QDomElement elem = domDoc.elementsByTagName("artist").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("name").isEmpty()) {
                name = elem.elementsByTagName("name").at(0).toElement().text();
            }
            if (!elem.elementsByTagName("disambiguation").isEmpty()) {
                name.append(QString(" (%1)").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text()));
            }

            if (!name.isEmpty() && !elem.attribute("id").isEmpty()) {
                ScraperSearchResult result;
                result.id = elem.attribute("id");
                result.name = name;
                results.append(result);
            }
        }

        emit sigSearchDone(results);
    });
}

void UniversalMusicScraper::loadData(MusicBrainzId mbId, Artist* artist, QSet<MusicScraperInfo> infos)
{
    // Otherwise deleted images are showing up again
    infos.remove(MusicScraperInfo::ExtraFanarts);
    artist->clear(infos);
    artist->setMbId(mbId);
    artist->setAllMusicId(AllMusicId::NoId);

    m_musicBrainzApi.loadArtist(m_language, mbId, [artist, infos, this](QString html, ScraperError error) {
        QString discogsUrl;
        if (!error.hasError()) {
            QDomDocument domDoc;
            domDoc.setContent(html);
            for (int i = 0, n = domDoc.elementsByTagName("relation").count(); i < n; ++i) {
                QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
                if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                    QString url = elem.elementsByTagName("target").at(0).toElement().text();
                    QRegExp rx("allmusic\\.com/artist/(.*)$");
                    if (rx.indexIn(url) != -1) {
                        artist->setAllMusicId(AllMusicId(rx.cap(1)));
                    }
                }
                if (elem.attribute("type") == "discogs" && elem.elementsByTagName("target").count() > 0) {
                    discogsUrl = elem.elementsByTagName("target").at(0).toElement().text();
                }
            }
        }

        if (!m_artistDownloads.contains(artist)) {
            m_artistDownloads.insert(artist, QVector<DownloadElement>());
        }
        m_artistDownloads[artist].clear();

        const auto& artistMbId = artist->mbId();

        // TODO: Use their API
        // https://wiki.musicbrainz.org/MusicBrainz_API
        appendDownloadElement(artist,
            "musicbrainz",
            "musicbrainz_biography",
            QUrl(QStringLiteral("https://musicbrainz.org/artist/%1/wikipedia-extract").arg(artistMbId.toString())));

        appendDownloadElement(artist, "theaudiodb", "tadb_data", m_theAudioDbApi.makeArtistUrl(artistMbId));
        appendDownloadElement(
            artist, "theaudiodb", "tadb_discography", m_theAudioDbApi.makeArtistDiscographyUrl(artistMbId));

        if (artist->allMusicId().isValid()) {
            const auto& amId = artist->allMusicId();
            appendDownloadElement(artist, "allmusic", "am_data", m_allMusicApi.makeArtistUrl(amId));
            appendDownloadElement(artist, "allmusic", "am_biography", m_allMusicApi.makeArtistBiographyUrl(amId));
        }
        if (!discogsUrl.isEmpty()) {
            appendDownloadElement(
                artist, "discogs", "discogs_data", QUrl(discogsUrl + "?type=Releases&subtype=Albums"));
        }

        for (const DownloadElement& elem : asConst(m_artistDownloads[artist])) {
            QNetworkRequest request(elem.url);
            request.setRawHeader(
                "User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0");
            if (elem.source == "musicbrainz") {
                request.setRawHeader("Accept-Language", m_language.toUtf8());
            }

            QNetworkReply* elemReply = network()->getWithWatcher(request);
            elemReply->setProperty("storage", Storage::toVariant(elemReply, artist));
            elemReply->setProperty("infosToLoad", Storage::toVariant(elemReply, infos));
            connect(elemReply, &QNetworkReply::finished, this, &UniversalMusicScraper::onArtistLoadFinished);
        }
    });
}

void UniversalMusicScraper::onArtistLoadFinished()
{
    QMutexLocker locker(&m_artistMutex);

    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Artist* artist = reply->property("storage").value<Storage*>()->artist();
    QSet<MusicScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->musicInfosToLoad();
    reply->deleteLater();

    if (artist == nullptr) {
        return;
    }

    if (!m_artistDownloads.contains(artist)) {
        return;
    }

    int index = -1;
    for (int i = 0, n = m_artistDownloads[artist].count(); i < n; ++i) {
        if (m_artistDownloads[artist][i].url == reply->url()) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        m_artistDownloads[artist][index].contents = QString::fromUtf8(reply->readAll());
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    m_artistDownloads[artist][index].downloaded = true;

    bool finished = true;
    for (int i = 0, n = m_artistDownloads[artist].count(); i < n; ++i) {
        if (!m_artistDownloads[artist][i].downloaded) {
            finished = false;
            break;
        }
    }

    if (!finished) {
        return;
    }

    for (const DownloadElement& elem : asConst(m_artistDownloads[artist])) {
        if (elem.source != m_prefer) {
            continue;
        }
        processDownloadElement(elem, artist, infos);
    }
    for (const DownloadElement& elem : asConst(m_artistDownloads[artist])) {
        if (elem.source == m_prefer) {
            continue;
        }
        processDownloadElement(elem, artist, infos);
    }

    m_artistDownloads.remove(artist);
    artist->controller()->scraperLoadDone(this);
}

void UniversalMusicScraper::processDownloadElement(DownloadElement elem, Artist* artist, QSet<MusicScraperInfo> infos)
{
    if (elem.type.startsWith("tadb_")) {
        QJsonParseError parseError{};
        const auto parsedJson = QJsonDocument::fromJson(elem.contents.toUtf8(), &parseError).object();
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing music json: " << parseError.errorString();
            return;
        }

        if (elem.type == "tadb_data") {
            m_theAudioDb.parseAndAssignArtist(parsedJson, artist, infos, m_language);
        } else if (elem.type == "tadb_discography") {
            m_theAudioDb.parseAndAssignArtistDiscography(parsedJson, artist, infos);
        }
    } else if (elem.type == "am_data") {
        m_allMusic.parseAndAssignArtist(elem.contents, artist, infos);
    } else if (elem.type == "musicbrainz_biography") {
        m_musicBrainz.parseAndAssignArtist(elem.contents, artist, infos);
    } else if (elem.type == "am_biography") {
        m_allMusic.parseAndAssignArtistBiography(elem.contents, artist, infos);
    } else if (elem.type == "discogs_data") {
        m_discogs.parseAndAssignArtist(elem.contents, artist, infos);
    }
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
    if (cleanSearchStr.isEmpty()) {
        cleanSearchStr = searchStr;
    }

    const auto callback = [this](QString html, ScraperError error) {
        if (error.hasError()) {
            emit sigSearchDone({});
            return;
        }

        QVector<ScraperSearchResult> results;

        QDomDocument domDoc;
        domDoc.setContent(html);
        for (int i = 0, releaseCount = domDoc.elementsByTagName("release").count(); i < releaseCount; ++i) {
            QDomElement elem = domDoc.elementsByTagName("release").at(i).toElement();
            QString name;
            if (!elem.elementsByTagName("title").isEmpty()) {
                name = elem.elementsByTagName("title").at(0).toElement().text();
            } else {
                continue;
            }

            QMap<QString, int> mediumList;
            if (!elem.elementsByTagName("medium-list").isEmpty()) {
                for (int j = 0, n = elem.elementsByTagName("medium-list").count(); j < n; ++j) {
                    QDomElement mediumElem = elem.elementsByTagName("medium-list").at(j).toElement();
                    if (mediumElem.elementsByTagName("format").isEmpty()) {
                        continue;
                    }
                    QString medium = mediumElem.elementsByTagName("format").at(0).toElement().text();
                    mediumList.insert(medium, mediumList.value(medium, 0) + 1);
                }
            }
            QStringList media;
            QMapIterator<QString, int> it(mediumList);
            while (it.hasNext()) {
                it.next();
                if (it.value() > 1) {
                    media << QString("%1x%2").arg(it.value()).arg(it.key());
                } else {
                    media << it.key();
                }
            }

            if (!media.isEmpty()) {
                name += QString(", %1").arg(media.join(" + "));
            }

            if (!elem.elementsByTagName("disambiguation").isEmpty()) {
                name += QString(", %1").arg(elem.elementsByTagName("disambiguation").at(0).toElement().text());
            }

            if (!elem.elementsByTagName("date").isEmpty()) {
                name += QString(" (%1)").arg(elem.elementsByTagName("date").at(0).toElement().text());
            }

            if (!elem.elementsByTagName("country").isEmpty()) {
                name += QString(" %1").arg(elem.elementsByTagName("country").at(0).toElement().text());
            }

            ScraperSearchResult result;
            result.id = elem.attribute("id");
            result.name = name;
            if (!elem.elementsByTagName("release-group").isEmpty()) {
                result.id2 = elem.elementsByTagName("release-group").at(0).toElement().attribute("id");
            }
            results.append(result);
        }

        emit sigSearchDone(results);
    };

    if (artistName.isEmpty()) {
        m_musicBrainzApi.searchForAlbum(m_language, cleanSearchStr, callback);
    } else {
        m_musicBrainzApi.searchForAlbumWithArtist(m_language, cleanSearchStr, artistName, callback);
    }
}

void UniversalMusicScraper::loadData(MusicBrainzId mbAlbumId,
    MusicBrainzId mbReleaseGroupId,
    Album* album,
    QSet<MusicScraperInfo> infos)
{
    album->clear(infos);
    album->setMbAlbumId(mbAlbumId);
    album->setMbReleaseGroupId(mbReleaseGroupId);
    album->setAllMusicId(AllMusicId::NoId);
    m_musicBrainzApi.loadAlbum(m_language, mbAlbumId, [album, infos, this](QString html, ScraperError error) {
        QString discogsUrl;
        if (!error.hasError()) {
            m_musicBrainz.parseAndAssignAlbum(html, album, infos);
            QDomDocument domDoc;
            domDoc.setContent(html);
            for (int i = 0, n = domDoc.elementsByTagName("relation").count(); i < n; ++i) {
                QDomElement elem = domDoc.elementsByTagName("relation").at(i).toElement();
                if (elem.attribute("type") == "allmusic" && elem.elementsByTagName("target").count() > 0) {
                    QString url = elem.elementsByTagName("target").at(0).toElement().text();
                    QRegExp rx("allmusic\\.com/album/(.*)$");
                    if (rx.indexIn(url) != -1) {
                        album->setAllMusicId(AllMusicId(rx.cap(1)));
                    }
                }
                if (elem.attribute("type") == "discogs" && elem.elementsByTagName("target").count() > 0) {
                    discogsUrl = elem.elementsByTagName("target").at(0).toElement().text();
                }
            }
        }

        if (!m_albumDownloads.contains(album)) {
            m_albumDownloads.insert(album, QVector<DownloadElement>());
        }
        m_albumDownloads[album].clear();

        appendDownloadElement(album,
            "theaudiodb",
            "tadb_data",
            QUrl(QString("https://www.theaudiodb.com/api/v1/json/%1/album-mb.php?i=%2")
                     .arg(m_tadbApiKey, album->mbReleaseGroupId().toString())));
        if (album->allMusicId().isValid()) {
            appendDownloadElement(album,
                "allmusic",
                "am_data",
                QString("https://www.allmusic.com/album/%1").arg(album->allMusicId().toString()));
        }
        if (!discogsUrl.isEmpty()) {
            appendDownloadElement(album, "discogs", "discogs_data", QUrl(discogsUrl));
        }

        for (const DownloadElement& elem : asConst(m_albumDownloads[album])) {
            QNetworkRequest request(elem.url);
            request.setRawHeader(
                "User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10; rv:33.0) Gecko/20100101 Firefox/33.0");
            QNetworkReply* elemReply = network()->getWithWatcher(request);
            elemReply->setProperty("storage", Storage::toVariant(elemReply, album));
            elemReply->setProperty("infosToLoad", Storage::toVariant(elemReply, infos));
            connect(elemReply, &QNetworkReply::finished, this, &UniversalMusicScraper::onAlbumLoadFinished);
        }
    });
}

void UniversalMusicScraper::onAlbumLoadFinished()
{
    QMutexLocker locker(&m_albumMutex);

    auto* reply = dynamic_cast<QNetworkReply*>(QObject::sender());
    Album* album = reply->property("storage").value<Storage*>()->album();
    QSet<MusicScraperInfo> infos = reply->property("infosToLoad").value<Storage*>()->musicInfosToLoad();
    reply->deleteLater();
    if (album == nullptr) {
        return;
    }

    if (!m_albumDownloads.contains(album)) {
        return;
    }

    int index = -1;
    for (int i = 0, n = m_albumDownloads[album].count(); i < n; ++i) {
        if (m_albumDownloads[album][i].url == reply->url()) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        m_albumDownloads[album][index].contents = QString::fromUtf8(reply->readAll());
    } else {
        qWarning() << "Network Error (load)" << reply->errorString();
    }
    m_albumDownloads[album][index].downloaded = true;

    bool finished = true;
    for (int i = 0, n = m_albumDownloads[album].count(); i < n; ++i) {
        if (!m_albumDownloads[album][i].downloaded) {
            finished = false;
            break;
        }
    }

    if (!finished) {
        return;
    }

    for (const DownloadElement& elem : asConst(m_albumDownloads[album])) {
        if (elem.source != m_prefer) {
            continue;
        }
        processDownloadElement(elem, album, infos);
    }
    for (const DownloadElement& elem : asConst(m_albumDownloads[album])) {
        if (elem.source == m_prefer) {
            continue;
        }
        processDownloadElement(elem, album, infos);
    }

    m_albumDownloads.remove(album);
    album->controller()->scraperLoadDone(this);
}

void UniversalMusicScraper::processDownloadElement(DownloadElement elem, Album* album, QSet<MusicScraperInfo> infos)
{
    if (elem.type == "tadb_data") {
        QJsonParseError parseError{};
        const auto parsedJson = QJsonDocument::fromJson(elem.contents.toUtf8(), &parseError).object();
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "Error parsing music json: " << parseError.errorString();
            return;
        }

        m_theAudioDb.parseAndAssignAlbum(parsedJson, album, infos, m_language);

    } else if (elem.type == "am_data") {
        m_allMusic.parseAndAssignAlbum(elem.contents, album, infos);

    } else if (elem.type == "discogs_data") {
        m_discogs.parseAndAssignAlbum(elem.contents, album, infos);
    }
}

bool UniversalMusicScraper::hasSettings() const
{
    return true;
}

void UniversalMusicScraper::loadSettings(ScraperSettings& settings)
{
    m_language = settings.language(m_language).toString();
    for (int i = 0, n = m_box->count(); i < n; ++i) {
        if (m_box->itemData(i).toString() == m_language) {
            m_box->setCurrentIndex(i);
        }
    }
    m_prefer = settings.valueString("Prefer", "theaudiodb");
    for (int i = 0, n = m_preferBox->count(); i < n; ++i) {
        if (m_preferBox->itemData(i).toString() == m_prefer) {
            m_preferBox->setCurrentIndex(i);
        }
    }
}

void UniversalMusicScraper::saveSettings(ScraperSettings& settings)
{
    m_language = m_box->itemData(m_box->currentIndex()).toString();
    settings.setString("Language", m_language);
    m_prefer = m_preferBox->itemData(m_preferBox->currentIndex()).toString();
    settings.setString("Prefer", m_prefer);
}

QSet<MusicScraperInfo> UniversalMusicScraper::scraperSupports()
{
    return {MusicScraperInfo::Name,
        MusicScraperInfo::Genres,
        MusicScraperInfo::Styles,
        MusicScraperInfo::Moods,
        MusicScraperInfo::Formed,
        MusicScraperInfo::Born,
        MusicScraperInfo::Died,
        MusicScraperInfo::Disbanded,
        MusicScraperInfo::Biography,
        MusicScraperInfo::Thumb,
        MusicScraperInfo::Fanart,
        MusicScraperInfo::Logo,
        MusicScraperInfo::Title,
        MusicScraperInfo::Artist,
        MusicScraperInfo::Review,
        MusicScraperInfo::Rating,
        MusicScraperInfo::Year,
        MusicScraperInfo::CdArt,
        MusicScraperInfo::Cover,
        MusicScraperInfo::YearsActive,
        MusicScraperInfo::ReleaseDate,
        MusicScraperInfo::Year,
        MusicScraperInfo::ExtraFanarts,
        MusicScraperInfo::Discography,
        MusicScraperInfo::Label};
}

QWidget* UniversalMusicScraper::settingsWidget()
{
    return m_widget;
}

QString UniversalMusicScraper::trim(QString text)
{
    return text.replace(QRegExp("\\s{1,}"), " ").trimmed();
}

bool UniversalMusicScraper::shouldLoad(MusicScraperInfo info, QSet<MusicScraperInfo> infos, Album* album)
{
    if (!infos.contains(info)) {
        return false;
    }

    switch (info) {
    case MusicScraperInfo::Title: return album->title().isEmpty();
    case MusicScraperInfo::Artist: return album->artist().isEmpty();
    case MusicScraperInfo::Review: return album->review().isEmpty();
    case MusicScraperInfo::ReleaseDate: return album->releaseDate().isEmpty();
    case MusicScraperInfo::Label: return album->label().isEmpty();
    case MusicScraperInfo::Rating: return album->rating() < 0.01 && album->rating() > -0.01;
    case MusicScraperInfo::Year: return album->year() == 0;
    case MusicScraperInfo::Genres: return album->genres().isEmpty();
    case MusicScraperInfo::Styles: return album->styles().isEmpty();
    case MusicScraperInfo::Moods: return album->moods().isEmpty();
    default: break;
    }

    return false;
}

bool UniversalMusicScraper::shouldLoad(MusicScraperInfo info, QSet<MusicScraperInfo> infos, Artist* artist)
{
    if (!infos.contains(info)) {
        return false;
    }

    switch (info) {
    case MusicScraperInfo::Name: return artist->name().isEmpty();
    case MusicScraperInfo::YearsActive: return artist->yearsActive().isEmpty();
    case MusicScraperInfo::Formed: return artist->formed().isEmpty();
    case MusicScraperInfo::Born: return artist->born().isEmpty();
    case MusicScraperInfo::Died: return artist->died().isEmpty();
    case MusicScraperInfo::Disbanded: return artist->disbanded().isEmpty();
    case MusicScraperInfo::Biography: return artist->biography().isEmpty();
    case MusicScraperInfo::Genres: return artist->genres().isEmpty();
    case MusicScraperInfo::Styles: return artist->styles().isEmpty();
    case MusicScraperInfo::Moods: return artist->moods().isEmpty();
    case MusicScraperInfo::Discography: return artist->discographyAlbums().isEmpty();
    default: break;
    }

    return false;
}

bool UniversalMusicScraper::infosLeft(QSet<MusicScraperInfo> infos, Album* album)
{
    for (const auto info : infos) {
        if (shouldLoad(info, infos, album)) {
            return true;
        }
    }
    return false;
}

bool UniversalMusicScraper::infosLeft(QSet<MusicScraperInfo> infos, Artist* artist)
{
    for (const auto info : infos) {
        if (shouldLoad(info, infos, artist)) {
            return true;
        }
    }
    return false;
}

void UniversalMusicScraper::appendDownloadElement(Artist* artist, QString source, QString type, QUrl url)
{
    DownloadElement elem;
    elem.type = type;
    elem.url = url;
    elem.downloaded = false;
    elem.source = source;
    if (!m_artistDownloads.contains(artist)) {
        m_artistDownloads.insert(artist, QVector<DownloadElement>());
    }
    m_artistDownloads[artist].append(elem);
}

void UniversalMusicScraper::appendDownloadElement(Album* album, QString source, QString type, QUrl url)
{
    DownloadElement elem;
    elem.type = type;
    elem.url = url;
    elem.downloaded = false;
    elem.source = source;
    if (!m_albumDownloads.contains(album)) {
        m_albumDownloads.insert(album, QVector<DownloadElement>());
    }
    m_albumDownloads[album].append(elem);
}

} // namespace scraper
} // namespace mediaelch
