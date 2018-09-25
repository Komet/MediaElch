#include "ArtistXmlReader.h"

#include "globals/Globals.h"
#include "music/Artist.h"

#include <QDate>
#include <QDomElement>
#include <QFileInfo>
#include <QTime>
#include <QUrl>

namespace Kodi {

ArtistXmlReader::ArtistXmlReader(Artist &actor) : m_artist{actor}
{
}

void ArtistXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (!domDoc.elementsByTagName("musicBrainzArtistID").isEmpty()) {
        m_artist.setMbId(domDoc.elementsByTagName("musicBrainzArtistID").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("allmusicid").isEmpty()) {
        m_artist.setAllMusicId(domDoc.elementsByTagName("allmusicid").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("name").isEmpty()) {
        m_artist.setName(domDoc.elementsByTagName("name").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("genre").isEmpty()) {
        m_artist.setGenres(
            domDoc.elementsByTagName("genre").at(0).toElement().text().split(" / ", QString::SkipEmptyParts));
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

    for (int i = 0, n = domDoc.elementsByTagName("thumb").size(); i < n; i++) {
        QString parentTag = domDoc.elementsByTagName("thumb").at(i).parentNode().toElement().tagName();
        if (parentTag == "artist") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            if (!domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview").isEmpty()) {
                p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            } else {
                p.thumbUrl = p.originalUrl;
            }
            m_artist.addImage(ImageType::ArtistThumb, p);
        } else if (parentTag == "fanart") {
            Poster p;
            p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
            if (!domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview").isEmpty()) {
                p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
            } else {
                p.thumbUrl = p.originalUrl;
            }
            m_artist.addImage(ImageType::ArtistFanart, p);
        }
    }

    for (int i = 0, n = domDoc.elementsByTagName("album").size(); i < n; i++) {
        DiscographyAlbum a;
        if (!domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("title").isEmpty()) {
            a.title =
                domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("title").at(0).toElement().text();
        }
        if (!domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("year").isEmpty()) {
            a.year =
                domDoc.elementsByTagName("album").at(i).toElement().elementsByTagName("year").at(0).toElement().text();
        }
        m_artist.addDiscographyAlbum(a);
    }

    m_artist.setHasChanged(false);
}

} // namespace Kodi
