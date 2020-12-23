#pragma once

#include "globals/Globals.h"
#include "scrapers/tv_show/TvScraper.h"

#include <QComboBox>
#include <QGroupBox>
#include <QString>

class TvShowCommonWidgets
{
public:
    /// \brief Enables/disables the episode/show info boxes
    /// \details Does so to reflect what the given scraper supports and what the user saved.
    static void toggleInfoBoxesForScraper(const mediaelch::scraper::TvScraper& scraper,
        TvShowUpdateType type,
        QGroupBox* showInfosGroupBox,
        QGroupBox* episodeInfosGroupBox);

    /// \brief Fills the given combo box with season order items and returns the selected/active type.
    static SeasonOrder setupSeasonOrderComboBox(const mediaelch::scraper::TvScraper& scraper,
        SeasonOrder defaultSeasonOrder,
        QComboBox* comboSeasonOrder);
};
