#pragma once

#include "scrapers/api/TmdbTvApi.h"

mediaelch::scraper::TmdbTvApi& getTmdbTvApi();
void waitForTmdbTvInitialized();
