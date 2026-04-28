#pragma once

#include <QString>

namespace mediaelch {
namespace scraper {

/// \brief GraphQL query strings for the IMDB GraphQL API (https://graphql.imdb.com/).
///
/// These queries are comprehensive and request more fields than MediaElch currently
/// parses. This is intentional — the extra fields (budget, awards, etc.) are available
/// for future use without modifying the API layer.
namespace ImdbGraphQLQueries {

/// \brief Full title details query for movies and TV shows.
/// Fetches all metadata fields in a single request.
/// Variables: $id (String!)
inline const QString TITLE_DETAILS = QStringLiteral(R"(
query TitleDetails($id: ID!) {
  title(id: $id) {
    id
    titleText { text }
    originalTitleText { text }
    titleType { id text }
    releaseDate { day month year }
    runtime { seconds }
    plot { plotText { plainText } }
    plots(first: 10) {
      edges { node { plotText { plainText } plotType } }
    }
    ratingsSummary { aggregateRating voteCount }
    meterRanking { currentRank }
    genres { genres { text id } }
    keywords(first: 100) {
      edges { node { text } }
    }
    certificate { rating }
    certificates(first: 50) {
      edges { node { rating country { id text } } }
    }
    akas(first: 50) {
      edges { node { text country { id text } language { id text } } }
    }
    cast: credits(first: 250, filter: { categories: ["actor", "actress"] }) {
      edges {
        node {
          name { id nameText { text } primaryImage { url } }
          ... on Cast { characters { name } }
        }
      }
    }
    directors: credits(first: 50, filter: { categories: ["director"] }) {
      edges { node { name { nameText { text } } } }
    }
    writers: credits(first: 50, filter: { categories: ["writer"] }) {
      edges { node { name { nameText { text } } } }
    }
    taglines(first: 5) {
      edges { node { text } }
    }
    countriesOfOrigin { countries { id text } }
    companyCredits(first: 20, filter: { categories: ["production"] }) {
      edges { node { company { companyText { text } } category { text } } }
    }
    primaryImage { url width height }
    images(first: 10) {
      edges { node { url width height caption { plainText } } }
    }
    primaryVideos(first: 1) {
      edges { node { id name { value } runtime { value } } }
    }
    metacritic { metascore { score } }
    releaseDates(first: 50) {
      edges { node { day month year country { id text } } }
    }
    episodes {
      isOngoing
      seasons { number }
    }

    # Future fields — included in query but not yet parsed by MediaElch
    productionBudget { budget { amount currency } }
    prestigiousAwardSummary { wins nominations award { text } }
    technicalSpecifications {
      aspectRatios { items { aspectRatio } }
      soundMixes { items { text } }
      colorations { items { text } }
    }
    filmingLocations(first: 10) {
      edges { node { text } }
      total
    }
    moreLikeThisTitles(first: 10) {
      edges { node { id titleText { text } } }
    }
    connections(first: 20) {
      edges {
        node {
          associatedTitle { id titleText { text } }
          category { text }
        }
      }
    }
  }
}
)");

/// \brief Episode listing for a specific season.
/// Variables: $id (ID!), $first (Int!)
inline const QString SEASON_EPISODES = QStringLiteral(R"(
query SeasonEpisodes($id: ID!, $first: Int!) {
  title(id: $id) {
    episodes {
      episodes(first: $first) {
        edges {
          node {
            id
            titleText { text }
            series { displayableEpisodeNumber { displayableSeason { text } episodeNumber { text } } }
            plot { plotText { plainText } }
            releaseDate { day month year }
            ratingsSummary { aggregateRating voteCount }
            runtime { seconds }
            primaryImage { url width height }
            certificate { rating }
            certificates(first: 10) {
              edges { node { rating country { id text } } }
            }
            directors: credits(first: 10, filter: { categories: ["director"] }) {
              edges { node { name { nameText { text } } } }
            }
            writers: credits(first: 10, filter: { categories: ["writer"] }) {
              edges { node { name { nameText { text } } } }
            }
            cast: credits(first: 50, filter: { categories: ["actor", "actress"] }) {
              edges {
                node {
                  name { id nameText { text } primaryImage { url } }
                  ... on Cast { characters { name } }
                }
              }
            }
          }
        }
        pageInfo { hasNextPage endCursor }
      }
    }
  }
}
)");

/// \brief Episode listing filtered by season number.
/// Variables: $id (ID!), $first (Int!), $season (String!)
inline const QString SEASON_EPISODES_FILTERED = QStringLiteral(R"(
query SeasonEpisodesFiltered($id: ID!, $first: Int!, $season: String!) {
  title(id: $id) {
    episodes {
      episodes(first: $first, filter: { includeSeasons: [$season] }) {
        edges {
          node {
            id
            titleText { text }
            series { displayableEpisodeNumber { displayableSeason { text } episodeNumber { text } } }
            plot { plotText { plainText } }
            releaseDate { day month year }
            ratingsSummary { aggregateRating voteCount }
            runtime { seconds }
            primaryImage { url width height }
            certificate { rating }
            certificates(first: 10) {
              edges { node { rating country { id text } } }
            }
            directors: credits(first: 10, filter: { categories: ["director"] }) {
              edges { node { name { nameText { text } } } }
            }
            writers: credits(first: 10, filter: { categories: ["writer"] }) {
              edges { node { name { nameText { text } } } }
            }
            cast: credits(first: 50, filter: { categories: ["actor", "actress"] }) {
              edges {
                node {
                  name { id nameText { text } primaryImage { url } }
                  ... on Cast { characters { name } }
                }
              }
            }
          }
        }
        pageInfo { hasNextPage endCursor }
      }
    }
  }
}
)");

} // namespace ImdbGraphQLQueries
} // namespace scraper
} // namespace mediaelch
