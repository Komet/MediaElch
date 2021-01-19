#include "ui/tv_show/TvShowCommonWidgets.h"

#include "settings/Settings.h"
#include "ui/small_widgets/MyCheckBox.h"

void TvShowCommonWidgets::toggleInfoBoxesForScraper(const mediaelch::scraper::TvScraper& scraper,
    TvShowUpdateType type,
    QGroupBox* showInfosGroupBox,
    QGroupBox* episodeInfosGroupBox)
{
    const bool enableShow = isShowUpdateType(type);
    const bool enableEpisode = isEpisodeUpdateType(type);

    // More fine-grained box-setup below for episode- and show details.
    episodeInfosGroupBox->setEnabled(enableEpisode);
    showInfosGroupBox->setEnabled(enableShow);

    const auto& meta = scraper.meta();
    const auto showInfos = Settings::instance()->scraperInfosShow(meta.identifier);
    const auto episodeInfos = Settings::instance()->scraperInfosEpisode(meta.identifier);

    const bool showBlocked = showInfosGroupBox->blockSignals(true);
    const bool episodeBlocked = episodeInfosGroupBox->blockSignals(true);

    {
        const auto& showCheckBoxes = showInfosGroupBox->findChildren<MyCheckBox*>();
        for (auto* box : showCheckBoxes) {
            const auto detail = ShowScraperInfo(box->myData().toInt());
            const bool supported = meta.supportedShowDetails.contains(detail);
            box->setChecked(showInfos.contains(detail) && supported);
            box->setEnabled(enableShow && supported);
        }
    }
    {
        const auto& episodeCheckBoxes = episodeInfosGroupBox->findChildren<MyCheckBox*>();
        for (auto* box : episodeCheckBoxes) {
            const auto detail = EpisodeScraperInfo(box->myData().toInt());
            const bool supported = meta.supportedEpisodeDetails.contains(detail);
            box->setChecked(episodeInfos.contains(detail) && supported);
            box->setEnabled(enableEpisode && supported);
        }
    }

    showInfosGroupBox->blockSignals(showBlocked);
    episodeInfosGroupBox->blockSignals(episodeBlocked);
}

SeasonOrder TvShowCommonWidgets::setupSeasonOrderComboBox(const mediaelch::scraper::TvScraper& scraper,
    SeasonOrder defaultSeasonOrder,
    QComboBox* comboSeasonOrder)
{
    const bool blocked = comboSeasonOrder->blockSignals(true);

    const auto& supported = scraper.meta().supportedSeasonOrders;

    comboSeasonOrder->clear();
    if (supported.contains(SeasonOrder::Aired)) {
        comboSeasonOrder->addItem(QObject::tr("Aired order"), static_cast<int>(SeasonOrder::Aired));
    }
    if (supported.contains(SeasonOrder::Dvd)) {
        comboSeasonOrder->addItem(QObject::tr("DVD order"), static_cast<int>(SeasonOrder::Dvd));
    }

    const int index = comboSeasonOrder->findData(static_cast<int>(defaultSeasonOrder));
    if (index > -1) {
        comboSeasonOrder->setCurrentIndex(index);
    } else {
        defaultSeasonOrder = SeasonOrder::Aired;
        qCritical() << "[TvShowSearch] Couldn't find season order element in combo box";
    }

    comboSeasonOrder->blockSignals(blocked);
    return defaultSeasonOrder;
}
