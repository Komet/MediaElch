#pragma once

#include "scrapers/tv_show/thetvdb/TheTvDbApi.h"

mediaelch::scraper::TheTvDbApi& getTheTvDbApi();
void waitForTheTvDbInitialized();
