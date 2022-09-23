#pragma once

#include "scrapers/ScraperError.h"
#include "utils/Meta.h"

#include <QJsonDocument>
#include <QObject>
#include <QString>
#include <chrono>

class TvShow;

namespace mediaelch {

namespace scraper {

class ImdbTvShowParser : public QObject
{
    Q_OBJECT

public:
    ImdbTvShowParser(TvShow& show) : m_show{show} {}

    ELCH_NODISCARD ScraperError parseInfos(const QString& html);

private:
    QString extractTitle(const QString& html);

    QJsonDocument extractMetaJson(const QString& html, ScraperError& error);
    std::chrono::minutes extractRuntime(const QString& html);

private:
    TvShow& m_show;
};

} // namespace scraper
} // namespace mediaelch
