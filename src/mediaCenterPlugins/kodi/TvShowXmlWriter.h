#ifndef KODI_TVSHOWXMLWRITER_H
#define KODI_TVSHOWXMLWRITER_H

#include <QByteArray>
#include <QDomElement>
#include <QString>

class TvShow;

namespace Kodi {

class TvShowXmlWriter
{
public:
    TvShowXmlWriter(TvShow &tvShow);
    QByteArray getTvShowXml();

private:
    TvShow &m_show;
};

} // namespace Kodi

#endif // KODI_TVSHOWXMLWRITER_H
