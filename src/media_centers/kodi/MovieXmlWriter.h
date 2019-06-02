#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Movie;

namespace kodi {

class MovieXmlWriter
{
public:
    MovieXmlWriter(Movie& movie);
    QByteArray getMovieXml();

private:
    Movie& m_movie;
};

} // namespace kodi
