#pragma once

#include "scrapers/api/TmdbApi.h"

mediaelch::scraper::TmdbApi& getTmdbApi();
void waitForTmdbTvInitialized();
