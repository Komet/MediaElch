#pragma once

#include <QDomDocument>
#include <QString>

class TvShow;

namespace Kodi {

class TvShowXmlReader
{
public:
    TvShowXmlReader(TvShow &tvShow);
    void parseNfoDom(QDomDocument domDoc);

private:
    TvShow &m_show;
};

} // namespace Kodi
