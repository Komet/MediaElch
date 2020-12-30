#pragma once

#include <QByteArray>

class Movie;

namespace mediaelch {
namespace kodi {

class MovieXmlWriter
{
public:
    virtual ~MovieXmlWriter() = default;
    virtual QByteArray getMovieXml(bool testMode = false) = 0;
};

} // namespace kodi
} // namespace mediaelch
