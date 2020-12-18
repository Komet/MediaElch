#pragma once

#include <QJsonObject>

class TvShow;

namespace mediaelch {
namespace scraper {

class TheTvDbShowParser
{
public:
    TheTvDbShowParser(TvShow& show) : m_show{show} {}

    void parseInfos(const QJsonObject& json);
    void parseActors(const QJsonObject& json);
    void parseImages(const QJsonObject& json);

private:
    TvShow& m_show;
};

} // namespace scraper
} // namespace mediaelch
