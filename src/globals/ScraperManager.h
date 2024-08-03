#pragma once

#include "utils/Meta.h"

#include <QObject>
#include <QString>
#include <QVector>

class Settings;

namespace mediaelch {
namespace scraper {

class MusicScraper;
class ConcertScraper;
class TvScraper;
class MovieScraper;

} // namespace scraper

class ScraperConfiguration;

} // namespace mediaelch

namespace mediaelch {

class ScraperManager : public QObject
{
    Q_OBJECT

public:
    explicit ScraperManager(Settings& settings, QObject* parent = nullptr);
    ~ScraperManager() override;

    ELCH_NODISCARD const QVector<mediaelch::scraper::MovieScraper*>& movieScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::TvScraper*>& tvScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::ConcertScraper*>& concertScrapers();
    ELCH_NODISCARD const QVector<mediaelch::scraper::MusicScraper*>& musicScrapers();

    ELCH_NODISCARD mediaelch::scraper::MovieScraper* movieScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::scraper::ConcertScraper* concertScraper(const QString& identifier);
    ELCH_NODISCARD mediaelch::scraper::TvScraper* tvScraper(const QString& identifier);

private:
    void initMovieScrapers();
    void initTvScrapers();
    void initConcertScrapers();
    void initMusicScrapers();

private:
    Settings& m_settings;

    QVector<mediaelch::scraper::MovieScraper*> m_movieScrapers;
    QVector<mediaelch::scraper::TvScraper*> m_tvScrapers;
    QVector<mediaelch::scraper::ConcertScraper*> m_concertScrapers;
    QVector<mediaelch::scraper::MusicScraper*> m_musicScrapers;

    QVector<mediaelch::ScraperConfiguration*> m_configurations;
};

} // namespace mediaelch
