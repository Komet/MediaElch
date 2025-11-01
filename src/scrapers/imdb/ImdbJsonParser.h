#pragma once

#include "data/Actor.h"
#include "data/Certification.h"
#include "data/ImdbId.h"
#include "data/Locale.h"
#include "data/Rating.h"
#include "data/movie/MovieImages.h"

#include <QJsonDocument>
#include <QJsonValue>
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
    Optional<int> top250{-1};
    Optional<Certification> certification;
    QVector<Poster> posters;
    Optional<QUrl> trailer{};
    QVector<Actor> actors;

    QSet<QString> directors;
    QSet<QString> writers;
    QSet<QString> genres;
    QSet<QString> studios;
    QSet<QString> countries;
    QSet<QString> tags;
};

class ImdbJsonParser
{
public:
    static ImdbData parseFromReferencePage(const QString& html, const mediaelch::Locale& preferredLocale);

    ~ImdbJsonParser() = default;

private:
    ImdbJsonParser() = default;

    void parserAndAssignDetails(const QJsonDocument& json, const mediaelch::Locale& preferredLocale);
    void parseAndAssignDirectors(const QJsonDocument& json);
    void parseAndStoreActors(const QJsonDocument& json);
    void parseAndAssignWriters(const QJsonDocument& json);

    /// Sanitize the given URL. Return value is the same object as the input string.
    static QString sanitizeAmazonMediaUrl(QString url);
    static QJsonDocument extractJsonFromHtml(const QString& html);
    static QJsonValue followJsonPath(const QJsonDocument& json, const QVector<QString>& paths);
    static QJsonValue followJsonPath(const QJsonObject& json, const QVector<QString>& paths);

private:
    ImdbData m_data{};
};


} // namespace scraper
} // namespace mediaelch
