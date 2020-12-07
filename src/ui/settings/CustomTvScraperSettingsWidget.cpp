#include "ui/settings/CustomTvScraperSettingsWidget.h"
#include "ui_CustomTvScraperSettingsWidget.h"

#include "globals/Manager.h"
#include "globals/ScraperInfos.h"
#include "scrapers/tv_show/custom/CustomTvScraper.h"
#include "settings/Settings.h"

CustomTvScraperSettingsWidget::CustomTvScraperSettingsWidget(QWidget* parent) :
    QWidget(parent), ui(new Ui::CustomTvScraperSettingsWidget)
{
    ui->setupUi(this);

    ui->customTvScraperShowDetails->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->customTvScraperShowDetails->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->customTvScraperShowDetails->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    ui->customTvScraperEpisodeDetails->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->customTvScraperEpisodeDetails->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->customTvScraperEpisodeDetails->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

CustomTvScraperSettingsWidget::~CustomTvScraperSettingsWidget()
{
    delete ui;
}

void CustomTvScraperSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void CustomTvScraperSettingsWidget::loadSettings()
{
    // vector to keep ordering
    QVector<ShowScraperInfo> tvInfos = {ShowScraperInfo::Actors,
        ShowScraperInfo::Banner,
        ShowScraperInfo::Certification,
        ShowScraperInfo::Fanart,
        ShowScraperInfo::FirstAired,
        ShowScraperInfo::Genres,
        ShowScraperInfo::Network,
        ShowScraperInfo::Overview,
        ShowScraperInfo::Poster,
        ShowScraperInfo::Rating,
        ShowScraperInfo::SeasonPoster,
        ShowScraperInfo::Title,
        ShowScraperInfo::Tags,
        ShowScraperInfo::ExtraArts,
        ShowScraperInfo::SeasonBackdrop,
        ShowScraperInfo::SeasonBanner,
        ShowScraperInfo::ExtraFanarts,
        ShowScraperInfo::Thumb,
        ShowScraperInfo::SeasonThumb,
        ShowScraperInfo::Runtime,
        ShowScraperInfo::Status};
    QVector<EpisodeScraperInfo> episodeInfos = {EpisodeScraperInfo::Actors,
        EpisodeScraperInfo::Certification,
        EpisodeScraperInfo::Director,
        EpisodeScraperInfo::FirstAired,
        EpisodeScraperInfo::Network,
        EpisodeScraperInfo::Overview,
        EpisodeScraperInfo::Rating,
        EpisodeScraperInfo::Tags,
        EpisodeScraperInfo::Thumbnail,
        EpisodeScraperInfo::Title,
        EpisodeScraperInfo::Writer};

    ui->customTvScraperShowDetails->clearContents();
    ui->customTvScraperShowDetails->setRowCount(0);

    ui->customTvScraperEpisodeDetails->clearContents();
    ui->customTvScraperEpisodeDetails->setRowCount(0);

    for (const ShowScraperInfo info : tvInfos) {
        const int row = ui->customTvScraperShowDetails->rowCount();
        ui->customTvScraperShowDetails->insertRow(row);
        ui->customTvScraperShowDetails->setItem(
            row, 0, new QTableWidgetItem(mediaelch::scraperInfoToTranslatedString(info)));
        ui->customTvScraperShowDetails->setCellWidget(row, 1, comboForTvScraperInfo(info));
    }

    for (const EpisodeScraperInfo info : episodeInfos) {
        const int row = ui->customTvScraperEpisodeDetails->rowCount();
        ui->customTvScraperEpisodeDetails->insertRow(row);
        ui->customTvScraperEpisodeDetails->setItem(
            row, 0, new QTableWidgetItem(mediaelch::scraperInfoToTranslatedString(info)));
        ui->customTvScraperEpisodeDetails->setCellWidget(row, 1, comboForEpisodeInfo(info));
    }
}

void CustomTvScraperSettingsWidget::saveSettings()
{
    {
        QMap<ShowScraperInfo, QString> tvScraper;
        const int n = ui->customTvScraperShowDetails->rowCount();

        for (int row = 0; row < n; ++row) {
            auto* box = dynamic_cast<QComboBox*>(ui->customTvScraperShowDetails->cellWidget(row, 1));
            const int value = box->itemData(0, Qt::UserRole + 1).toInt();
            if (value > 0) {
                auto info = ShowScraperInfo(value);
                const QString scraper = box->itemData(box->currentIndex()).toString();
                tvScraper.insert(info, scraper);
            }
        }
        m_settings->setCustomTvScraperShow(tvScraper);
    }
    {
        QMap<EpisodeScraperInfo, QString> tvScraper;
        const int n = ui->customTvScraperEpisodeDetails->rowCount();

        for (int row = 0; row < n; ++row) {
            auto* box = dynamic_cast<QComboBox*>(ui->customTvScraperEpisodeDetails->cellWidget(row, 1));
            const int value = box->itemData(0, Qt::UserRole + 1).toInt();
            if (value > 0) {
                auto info = EpisodeScraperInfo(value);
                const QString scraper = box->itemData(box->currentIndex()).toString();
                tvScraper.insert(info, scraper);
            }
        }
        m_settings->setCustomTvScraperEpisode(tvScraper);
    }
}

QComboBox* CustomTvScraperSettingsWidget::comboForTvScraperInfo(ShowScraperInfo info)
{
    const auto& scrapers = mediaelch::scraper::CustomTvScraper::supportedScraperIds();
    const int infoInt = static_cast<int>(info);

    auto* box = new QComboBox();

    int scraperCount = 0;
    for (const QString& scraperId : scrapers) {
        auto* scraper = Manager::instance()->scrapers().tvScraper(scraperId);

        if (scraper == nullptr) {
            qCritical() << "[CustomTvScraperSettingsWidget] Could not get scraper with ID:" << scraperId;

        } else if (scraper->meta().supportedShowDetails.contains(info)) {
            box->addItem(scraper->meta().name, scraperId);
            box->setItemData(0, infoInt, Qt::UserRole + 1);
            ++scraperCount;
        }
    }

    if (scraperCount == 0) {
        box->addItem(tr("No Scraper Available"), "noscraper");
        box->setItemData(0, -1, Qt::UserRole + 1);
    }

    const QString id = m_settings->customTvScraperShow().value(info, scrapers.first());
    int index = box->findData(id, Qt::UserRole);
    index = index > 0 ? index : 0;
    box->setCurrentIndex(index);

    return box;
}

QComboBox* CustomTvScraperSettingsWidget::comboForEpisodeInfo(EpisodeScraperInfo info)
{
    const auto& scrapers = mediaelch::scraper::CustomTvScraper::supportedScraperIds();
    const int infoInt = static_cast<int>(info);

    auto* box = new QComboBox();

    int scraperCount = 0;
    for (const QString& scraperId : scrapers) {
        auto* scraper = Manager::instance()->scrapers().tvScraper(scraperId);

        if (scraper == nullptr) {
            qCritical() << "[CustomTvScraperSettingsWidget] Could not get scraper with ID:" << scraperId;

        } else if (scraper->meta().supportedEpisodeDetails.contains(info)) {
            box->addItem(scraper->meta().name, scraper->meta().identifier);
            box->setItemData(0, infoInt, Qt::UserRole + 1);
            ++scraperCount;
        }
    }

    if (scraperCount == 0) {
        box->addItem(tr("No Scraper Available"), "noscraper");
        box->setItemData(0, -1, Qt::UserRole + 1);
    }

    const QString id = m_settings->customTvScraperEpisode().value(info, scrapers.first());
    int index = box->findData(id, Qt::UserRole);
    index = index > 0 ? index : 0;
    box->setCurrentIndex(index);

    return box;
}
