#pragma once

#include "scrapers/tv_show/tmdb/TmdbTvApi.h"

mediaelch::scraper::TmdbTvApi& getTmdbTvApi();
void waitForTmdbTvInitialized();
