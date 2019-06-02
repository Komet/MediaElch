#pragma once

#include <QByteArray>
#include <QDomElement>
#include <QString>

class TvShow;

namespace kodi {

class TvShowXmlWriter
{
public:
    TvShowXmlWriter(TvShow& tvShow);
    QByteArray getTvShowXml();

private:
    TvShow& m_show;
};

} // namespace kodi
