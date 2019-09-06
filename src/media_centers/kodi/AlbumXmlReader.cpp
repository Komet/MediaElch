#include "media_centers/kodi/AlbumXmlReader.h"

#include "globals/Globals.h"
#include "music/Album.h"


namespace mediaelch {
namespace kodi {

AlbumXmlReader::AlbumXmlReader(Album& album) : m_album{album}
{
}

void AlbumXmlReader::parseNfoDom(QDomDocument domDoc)
{
    if (!domDoc.elementsByTagName("musicBrainzReleaseGroupID").isEmpty()) {
        m_album.setMbReleaseGroupId(domDoc.elementsByTagName("musicBrainzReleaseGroupID").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("musicBrainzAlbumID").isEmpty()) {
        m_album.setMbAlbumId(domDoc.elementsByTagName("musicBrainzAlbumID").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("allmusicid").isEmpty()) {
        m_album.setAllMusicId(domDoc.elementsByTagName("allmusicid").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("title").isEmpty()) {
        m_album.setTitle(domDoc.elementsByTagName("title").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("artist").isEmpty()) {
        m_album.setArtist(domDoc.elementsByTagName("artist").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("genre").isEmpty()) {
        m_album.setGenres(
            domDoc.elementsByTagName("genre").at(0).toElement().text().split(" / ", QString::SkipEmptyParts));
    }
    for (int i = 0, n = domDoc.elementsByTagName("style").size(); i < n; i++) {
        m_album.addStyle(domDoc.elementsByTagName("style").at(i).toElement().text());
    }
    for (int i = 0, n = domDoc.elementsByTagName("mood").size(); i < n; i++) {
        m_album.addMood(domDoc.elementsByTagName("mood").at(i).toElement().text());
    }
    if (!domDoc.elementsByTagName("review").isEmpty()) {
        m_album.setReview(domDoc.elementsByTagName("review").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("label").isEmpty()) {
        m_album.setLabel(domDoc.elementsByTagName("label").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("releasedate").isEmpty()) {
        m_album.setReleaseDate(domDoc.elementsByTagName("releasedate").at(0).toElement().text());
    }
    if (!domDoc.elementsByTagName("year").isEmpty()) {
        m_album.setYear(domDoc.elementsByTagName("year").at(0).toElement().text().toInt());
    }
    if (!domDoc.elementsByTagName("rating").isEmpty()) {
        m_album.setRating(domDoc.elementsByTagName("rating").at(0).toElement().text().replace(",", ".").toDouble());
    }
    for (int i = 0, n = domDoc.elementsByTagName("thumb").size(); i < n; i++) {
        Poster p;
        p.originalUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().text());
        if (!domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview").isEmpty()) {
            p.thumbUrl = QUrl(domDoc.elementsByTagName("thumb").at(i).toElement().attribute("preview"));
        } else {
            p.thumbUrl = p.originalUrl;
        }
        m_album.addImage(ImageType::AlbumThumb, p);
    }

    m_album.setHasChanged(false);
}

} // namespace kodi
} // namespace mediaelch