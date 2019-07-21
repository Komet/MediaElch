#pragma once

#include <QDomDocument>
#include <QString>

class TvShow;

namespace mediaelch {
namespace kodi {

class TvShowXmlReader
{
public:
    explicit TvShowXmlReader(TvShow& tvShow);
    void parseNfoDom(QDomDocument domDoc);

private:
    void showThumb(const QDomElement& element);
    void showFanartThumb(const QDomElement& element, QString thumbUrl);

    TvShow& m_show;
};

} // namespace kodi
} // namespace mediaelch
