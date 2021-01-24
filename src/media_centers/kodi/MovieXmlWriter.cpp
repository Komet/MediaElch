#include "media_centers/kodi/MovieXmlWriter.h"

#include "globals/Helper.h"
#include "media_centers/KodiXml.h"
#include "movies/Movie.h"

#include <QDomComment>
#include <QDomDocument>
#include <QDomElement>
#include <QString>

namespace mediaelch {
namespace kodi {

MovieXmlWriterGeneric::MovieXmlWriterGeneric(KodiVersion version, Movie& movie) :
    MovieXmlWriter(std::move(version)), m_movie{movie}
{
}

QByteArray MovieXmlWriterGeneric::getMovieXml(bool testMode)
{
    using namespace std::chrono_literals;

    QByteArray xmlContent;
    QXmlStreamWriter xml(&xmlContent);
    xml.setAutoFormatting(true);
    xml.writeStartDocument("1.0", true);

    xml.writeStartElement("movie");

    xml.writeTextElement("title", m_movie.name());
    if (!m_movie.originalName().isEmpty()
        && (m_movie.originalName() != m_movie.name() || !ignoreDuplicateOriginalTitle())) {
        xml.writeTextElement("originaltitle", m_movie.originalName());
    }
    if (!m_movie.sortTitle().isEmpty()) {
        xml.writeTextElement("sorttitle", m_movie.sortTitle());
    }

    // rating
    if (!m_movie.ratings().isEmpty()) {
        xml.writeStartElement("ratings");
        bool firstRating = true;
        const auto& ratings = m_movie.ratings();
        for (const Rating& rating : ratings) {
            xml.writeStartElement("rating");
            xml.writeAttribute("default", firstRating ? "true" : "false");
            if (rating.maxRating > 0) {
                xml.writeAttribute("max", QString::number(rating.maxRating));
            }
            xml.writeAttribute("name", rating.source);

            xml.writeTextElement("value", QString::number(rating.rating));
            xml.writeTextElement("votes", QString::number(rating.voteCount));
            xml.writeEndElement();

            firstRating = false;
        }
        xml.writeEndElement();
    }

    xml.writeTextElement("userrating", QString::number(m_movie.userRating()));
    xml.writeTextElement("top250", QString::number(m_movie.top250()));
    xml.writeTextElement("outline", m_movie.outline());
    xml.writeTextElement("plot", m_movie.overview());
    xml.writeTextElement("tagline", m_movie.tagline());
    if (m_movie.runtime() > 0min) {
        xml.writeTextElement("runtime", QString::number(m_movie.runtime().count()));
    }

    if (writeThumbUrlsToNfo()) {
        const auto& posters = m_movie.images().posters();
        for (const Poster& poster : posters) {
            xml.writeStartElement("thumb");
            QString aspect = poster.aspect.isEmpty() ? "poster" : poster.aspect;
            xml.writeAttribute("aspect", aspect);
            xml.writeAttribute("preview", poster.thumbUrl.toString());
            xml.writeCharacters(poster.originalUrl.toString());
            xml.writeEndElement();
        }

        if (!m_movie.images().backdrops().isEmpty()) {
            xml.writeStartElement("fanart");
            const auto& backdrops = m_movie.images().backdrops();
            for (const Poster& poster : backdrops) {
                xml.writeStartElement("thumb");
                xml.writeAttribute("preview", poster.thumbUrl.toString());
                xml.writeCharacters(poster.originalUrl.toString());
                xml.writeEndElement();
            }
            xml.writeEndElement();
        }
    }

    xml.writeTextElement("mpaa", m_movie.certification().toString());
    xml.writeTextElement("playcount", QString("%1").arg(m_movie.playcount()));
    xml.writeTextElement("lastplayed", m_movie.lastPlayed().toString("yyyy-MM-dd HH:mm:ss"));
    // id
    xml.writeTextElement("id", m_movie.imdbId().toString());
    // unique id: IMDb and TMDb
    {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("default", "true");
        xml.writeAttribute("type", "imdb");
        xml.writeCharacters(m_movie.imdbId().toString());
        xml.writeEndElement();
    }
    if (m_movie.tmdbId().isValid()) {
        xml.writeStartElement("uniqueid");
        xml.writeAttribute("type", "tmdb");
        xml.writeCharacters(m_movie.tmdbId().toString());
        xml.writeEndElement();
    }

    KodiXml::writeStringsAsOneTagEach(xml, "genre", m_movie.genres());
    KodiXml::writeStringsAsOneTagEach(xml, "country", m_movie.countries());

    // <set>
    //   <name>...</name>
    //   <overview>...</overview>
    // </set>
    MovieSet set = m_movie.set();
    if (!set.name.isEmpty()) {
        xml.writeStartElement("set");
        xml.writeTextElement("name", set.name);
        xml.writeTextElement("overview", set.overview);
        xml.writeEndElement();
    }

    QStringList writers;
    const auto& writersWithWhiteSpace = m_movie.writer().split(",");
    for (const QString& credit : writersWithWhiteSpace) {
        writers << credit.trimmed();
    }

    KodiXml::writeStringsAsOneTagEach(xml, "credits", writers);

    QStringList directors;
    const auto& directorsWithWhiteSpace = m_movie.director().split(",");
    for (const QString& director : directorsWithWhiteSpace) {
        directors << director.trimmed();
    }

    KodiXml::writeStringsAsOneTagEach(xml, "director", directors);

    xml.writeTextElement("premiered", m_movie.released().toString("yyyy-MM-dd"));
    xml.writeTextElement("year", m_movie.released().toString("yyyy"));

    KodiXml::writeStringsAsOneTagEach(xml,
        "studio",
        useFirstStudioOnly() && !m_movie.studios().isEmpty() ? m_movie.studios().mid(0, 1) : m_movie.studios());
    xml.writeTextElement("trailer", helper::formatTrailerUrl(m_movie.trailer().toString()));

    KodiXml::writeStreamDetails(xml, m_movie.streamDetails(), m_movie.subtitles(), m_movie.streamDetailsLoaded());

    const auto& actors = m_movie.actors();
    for (const Actor* actor : actors) {
        xml.writeStartElement("actor");

        xml.writeTextElement("name", actor->name);
        xml.writeTextElement("role", actor->role);
        xml.writeTextElement("order", QString::number(actor->order));

        if (writeThumbUrlsToNfo()) {
            // create a thumb tag even if its value is empty
            // Kodi does the same
            xml.writeTextElement("thumb", actor->thumb);
        }

        xml.writeEndElement();
    }

    // <resume>
    //   <position>0.000000</position>
    //   <total>0.000000</total>
    // </resume>
    ResumeTime time = m_movie.resumeTime();
    xml.writeStartElement("resume");
    xml.writeTextElement("position", QString::number(time.position));
    xml.writeTextElement("total", QString::number(time.total));
    xml.writeEndElement();

    if (m_movie.dateAdded().isValid()) {
        xml.writeTextElement("dateadded", m_movie.dateAdded().toString("yyyy-MM-dd HH:mm:ss"));
    }
    KodiXml::writeStringsAsOneTagEach(xml, "tag", m_movie.tags());

    if (!testMode) {
        addMediaelchGeneratorTag(xml);
    }

    xml.writeEndElement();
    xml.writeEndDocument();
    return xmlContent;
}

bool MovieXmlWriterGeneric::useFirstStudioOnly() const
{
    return m_useFirstStudioOnly;
}

void MovieXmlWriterGeneric::setUseFirstStudioOnly(bool useFirstStudioOnly)
{
    m_useFirstStudioOnly = useFirstStudioOnly;
}

bool MovieXmlWriterGeneric::ignoreDuplicateOriginalTitle() const
{
    return m_ignoreDuplicateOriginalTitle;
}

void MovieXmlWriterGeneric::setIgnoreDuplicateOriginalTitle(bool ignoreDuplicateOriginalTitle)
{
    m_ignoreDuplicateOriginalTitle = ignoreDuplicateOriginalTitle;
}

} // namespace kodi
} // namespace mediaelch
