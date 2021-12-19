#pragma once

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <chrono>

class TvShow;

namespace mediaelch {

struct ScraperError;

namespace scraper {

class ImdbTvShowParser : public QObject
{
    Q_OBJECT

public:
    ImdbTvShowParser(TvShow& show, ScraperError& error) : m_show{show}, m_error{error} {}

    void parseInfos(const QString& html);

private:
    QString extractTitle(const QString& html);

    QJsonDocument extractMetaJson(const QString& html);
    std::chrono::minutes extractRuntime(const QString& html);

private:
    TvShow& m_show;
    ScraperError& m_error;
};

} // namespace scraper
} // namespace mediaelch
