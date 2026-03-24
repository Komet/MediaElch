#pragma once

#include "data/Actor.h"
#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Locale.h"
#include "data/Rating.h"
#include "data/movie/MovieImages.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QVector>
#include <chrono>

#include "utils/Optional.h"

namespace mediaelch {
namespace scraper {

class ImdbData
{
public:
    ImdbId imdbId{};
    Optional<QString> title;
    Optional<QString> originalTitle;
    Optional<QString> overview;
    Optional<QString> outline;
    Optional<QString> tagline;
    Optional<std::chrono::minutes> runtime{};
    Optional<QDate> released;
    QVector<Rating> ratings{};
    Optional<int> top250;
    Optional<Certification> certification;
    Optional<Poster> poster;
    QVector<Poster> backdrops;
    Optional<QUrl> trailer{};
    QVector<Actor> actors;

    QSet<QString> directors;
    QSet<QString> writers;
    QSet<QString> genres;
    QSet<QString> studios;
    QSet<QString> countries;
    QSet<QString> keywords;

    // Localization fields
    Optional<QString> localizedTitle;
    Optional<Certification> localizedCertification;

    // TV show specific
    Optional<bool> isOngoing;
    Optional<QString> network;
};

struct ImdbEpisodeData
{
    ImdbId imdbId;
    int seasonNumber = -1;
    int episodeNumber = -1;
    Optional<QString> title;
    Optional<QString> overview;
    Optional<QDate> firstAired;
    Optional<Poster> thumbnail;
    QVector<Rating> ratings;
    Optional<std::chrono::minutes> runtime;
    Optional<Certification> certification;
    QSet<QString> directors;
    QSet<QString> writers;
    QVector<Actor> actors;
};

class ImdbJsonParser
{
public:
    /// \brief Parse full title details from a GraphQL API response.
    static ImdbData parseFromGraphQL(const QString& json, const mediaelch::Locale& locale);

    /// \brief Parse episode list from a GraphQL episodes response.
    static QVector<ImdbEpisodeData> parseEpisodesFromGraphQL(const QString& json, const mediaelch::Locale& locale);

    /// \brief Parse season numbers from a GraphQL title details response.
    static QVector<int> parseSeasonsFromGraphQL(const QString& json);

    ~ImdbJsonParser() = default;

    /// Sanitize the given URL. Return value is the same object as the input string.
    static QString sanitizeAmazonMediaUrl(QString url);

private:
    ImdbJsonParser() = default;

    void parseGraphQLTitle(const QJsonObject& title, const mediaelch::Locale& locale);
    void parseGraphQLCredits(const QJsonObject& title);
    void parseGraphQLActors(const QJsonObject& title);

private:
    ImdbData m_data{};
};

} // namespace scraper
} // namespace mediaelch
