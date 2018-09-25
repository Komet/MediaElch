#ifndef KODI_MOVIEXMLWRITER_H
#define KODI_MOVIEXMLWRITER_H

#include <QByteArray>
#include <QDomElement>
#include <QString>

class Movie;

namespace Kodi {

class MovieXmlWriter
{
public:
    MovieXmlWriter(Movie &movie);
    QByteArray getMovieXml();

private:
    Movie &m_movie;
};

} // namespace Kodi

#endif // KODI_MOVIEXMLWRITER_H
