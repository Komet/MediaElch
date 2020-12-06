#pragma once

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <chrono>

class TvShow;
struct ScraperLoadError;

namespace mediaelch {
namespace scraper {

class ImdbTvShowParser : public QObject
{
    Q_OBJECT

public:
    ImdbTvShowParser(TvShow& show, ScraperLoadError& error) : m_show{show}, m_error{error} {}

    void parseInfos(const QString& html);

private:
    QString extractTitle(const QString& html);

    QJsonDocument extractMetaJson(const QString& html);
    std::chrono::minutes extractRuntime(const QString& html);

private:
    TvShow& m_show;
    ScraperLoadError& m_error;
};

} // namespace scraper
} // namespace mediaelch
