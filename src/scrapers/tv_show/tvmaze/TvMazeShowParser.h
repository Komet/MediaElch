#pragma once

#include <QJsonDocument>

class TvShow;

namespace mediaelch {
namespace scraper {

class TvMazeShowParser
{
public:
    explicit TvMazeShowParser(TvShow& show) : m_show{show} {}

    void parseInfos(const QJsonDocument& json);

private:
    TvShow& m_show;
};

} // namespace scraper
} // namespace mediaelch
