#pragma once

#include <QString>

class TvShow;

namespace mediaelch {
namespace scraper {

class TheTvDbShowParser
{
public:
    TheTvDbShowParser(TvShow& show) : m_show{show} {}

    void parseInfos(const QString& json);
    void parseActors(const QString& json);
    void parseImages(const QString& json);

private:
    TvShow& m_show;
};

} // namespace scraper
} // namespace mediaelch
