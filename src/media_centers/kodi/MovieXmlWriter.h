#pragma once

#include <QByteArray>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlWriter
{
public:
    virtual ~MovieXmlWriter() = default;
    virtual QByteArray getMovieXml() = 0;
};

} // namespace kodi
} // namespace mediaelch
