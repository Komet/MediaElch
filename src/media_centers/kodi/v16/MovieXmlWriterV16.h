#pragma once

#include "media_centers/kodi/MovieXmlWriter.h"

#include <QByteArray>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlWriterV16 : public MovieXmlWriter
{
public:
    MovieXmlWriterV16(Movie& movie);
    QByteArray getMovieXml() override;

private:
    Movie& m_movie;
};

} // namespace kodi
} // namespace mediaelch
