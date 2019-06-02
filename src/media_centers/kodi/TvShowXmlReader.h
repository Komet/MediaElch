#pragma once

#include <QDomDocument>
#include <QString>

class TvShow;

namespace kodi {

class TvShowXmlReader
{
public:
    TvShowXmlReader(TvShow& tvShow);
    void parseNfoDom(QDomDocument domDoc);

private:
    TvShow& m_show;
};

} // namespace kodi
