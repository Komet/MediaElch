#pragma once

#include <QDomDocument>
#include <QString>

class Movie;

namespace Kodi {

class MovieXmlReader
{
public:
    MovieXmlReader(Movie& movie);
    void parseNfoDom(QDomDocument domDoc);

private:
    Movie& m_movie;
};

} // namespace Kodi
