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
    const auto showInfos = Settings::instance()->scraperInfos<ShowScraperInfo>(meta.identifier);
    const auto episodeInfos = Settings::instance()->scraperInfos<EpisodeScraperInfo>(meta.identifier);

    for (auto* box : showInfosGroupBox->findChildren<MyCheckBox*>()) {
        const auto detail = ShowScraperInfo(box->myData().toInt());
        const bool enabled = enableShow && meta.supportedShowDetails.contains(detail);
        const bool checked = enabled && (showInfos.isEmpty() || showInfos.contains(detail));
        box->setChecked(checked);
        box->setEnabled(enabled);
    }
    for (auto* box : episodeInfosGroupBox->findChildren<MyCheckBox*>()) {
        const auto detail = EpisodeScraperInfo(box->myData().toInt());
        const bool enabled = enableEpisode && meta.supportedEpisodeDetails.contains(detail);
        const bool checked = enabled && (showInfos.isEmpty() || episodeInfos.contains(detail));
        box->setChecked(checked);
        box->setEnabled(enabled);
    }
}

SeasonOrder TvShowCommonWidgets::setupSeasonOrderComboBox(const mediaelch::scraper::TvScraper& scraper,
    SeasonOrder defaultSeasonOrder,
    QComboBox* comboSeasonOrder)
{
    comboSeasonOrder->blockSignals(true);

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

    comboSeasonOrder->blockSignals(true);
    return defaultSeasonOrder;
}
