#pragma once

#include "media_centers/kodi/MovieXmlWriter.h"

#include <QByteArray>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlWriterV18 : public MovieXmlWriter
{
public:
    MovieXmlWriterV18(Movie& movie);
    QByteArray getMovieXml() override;

private:
    Movie& m_movie;
};

} // namespace kodi
} // namespace mediaelch
