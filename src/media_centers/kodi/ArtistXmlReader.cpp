#include "ArtistXmlReader.h"

#include "globals/Globals.h"
#include "music/Artist.h"

#include <QDate>
#include <QDomElement>
#include <QFileInfo>
#include <QTime>
#include <QUrl>

namespace mediaelch {
namespace kodi {

ArtistXmlReader::ArtistXmlReader(Artist& artist) : m_artist{artist}
{
}

void ArtistXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (!domDoc.elementsByTagName("musicBrainzArtistID").isEmpty()) {
        m_artist.setMbId(MusicBrainzId(domDoc.elementsByTagName("musicBrainzArtistID").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("allmusicid").isEmpty()) {
        m_artist.setAllMusicId(AllMusicId(domDoc.elementsByTagName("allmusicid").at(0).toElement().text()));
    }
    if (!domDoc.elementsByTagName("name").isEmpty()) {
        m_artist.setName(domDoc.elementsByTagName("name").at(0).toElement().text());
    }
    {
        QDomNodeList genreElements = domDoc.elementsByTagName("genre");
        QStringList genres;
        for (int i = 0, n = genreElements.size(); i < n; i++) {
            genres << genreElements.at(i).toElement().text().split(" / ", ElchSplitBehavior::SkipEmptyParts);
        }
        if (!genres.isEmpty()) {
            m_artist.setGenres(genres);
        }
    }
    for (int i = 0, n = domDoc.elementsByTagName("style").size(); i < n; i++) {
        m_artist.addStyle(domDoc.elementsByTagName("style").at(i).toElement().text());
    }
    for (int i = 0, n = domDoc.elementsByTagName("mood").size(); i < n; i++) {
        m_artist.addMood(domDoc.elementsByTagName("mood").at(i).toElement().text());
    }
    if (!domDoc.elementsByTagName("yearsactive").isEmpty()) {
        m_artist.setYearsActive(domDoc.elementsByTagName("yearsactive").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("formed").isEmpty()) {
        m_artist.setFormed(domDoc.elementsByTagName("formed").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("biography").isEmpty()) {
        m_artist.setBiography(domDoc.elementsByTagName("biography").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("born").isEmpty()) {
        m_artist.setBorn(domDoc.elementsByTagName("born").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("died").isEmpty()) {
        m_artist.setDied(domDoc.elementsByTagName("died").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("disbanded").isEmpty()) {
        m_artist.setDisbanded(domDoc.elementsByTagName("disbanded").at(0).toElement().text());
    }

    QDomNodeList thumbElements = domDoc.elementsByTagName("thumb");
    for (int i = 0, n = thumbElements.size(); i < n; i++) {
        QString parentTag = thumbElements.at(i).parentNode().toElement().tagName();
        QDomElement thumb = thumbElements.at(i).toElement();

        Poster p;
        p.originalUrl = thumb.text();
        p.thumbUrl = thumb.attribute("preview").trimmed().isEmpty() ? p.originalUrl : thumb.attribute("preview");
        p.aspect = thumb.attribute("aspect").trimmed();

        if (parentTag == "artist") {
            m_artist.addImage(ImageType::ArtistThumb, p);

        } else if (parentTag == "fanart") {
            m_artist.addImage(ImageType::ArtistFanart, p);
        }
    }

    QDomNodeList albumElements = domDoc.elementsByTagName("album");
    for (int i = 0, n = albumElements.size(); i < n; i++) {
        QDomElement album = albumElements.at(i).toElement();

        DiscographyAlbum a;
        if (!album.elementsByTagName("title").isEmpty()) {
            a.title = album.elementsByTagName("title").at(0).toElement().text();
        }
        if (!album.elementsByTagName("year").isEmpty()) {
            a.year = album.elementsByTagName("year").at(0).toElement().text();
        }
        m_artist.addDiscographyAlbum(a);
    }

    m_artist.setHasChanged(false);
}

} // namespace kodi
} // namespace mediaelch
