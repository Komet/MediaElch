#pragma once

#include "media_centers/kodi/KodiXmlWriter.h"

#include <QByteArray>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlWriter : public KodiXmlWriter
{
public:
    MovieXmlWriter(KodiVersion version) : KodiXmlWriter(std::move(version)) {}
    virtual ~MovieXmlWriter() = default;
    virtual QByteArray getMovieXml(bool testMode = false) = 0;
};

class MovieXmlWriterGeneric : public MovieXmlWriter
{
public:
    MovieXmlWriterGeneric(KodiVersion version, Movie& movie);
    QByteArray getMovieXml(bool testMode = false) override;

private:
    Movie& m_movie;
};

} // namespace kodi
} // namespace mediaelch
