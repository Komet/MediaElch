#pragma once

#include "media_centers/kodi/MovieXmlWriter.h"

#include <QByteArray>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlWriterV17 : public MovieXmlWriter
{
public:
    MovieXmlWriterV17(Movie& movie);
    QByteArray getMovieXml() override;

private:
    Movie& m_movie;
};

} // namespace kodi
} // namespace mediaelch
