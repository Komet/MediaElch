#pragma once

#include "scrapers/tmdb/TmdbApi.h"

mediaelch::scraper::TmdbApi& getTmdbApi();
void waitForTmdbTvInitialized();
