#include "ui/settings/TvScraperSettingsWidget.h"
#include "ui_TvScraperSettingsWidget.h"

#include "globals/Manager.h"
#include "scrapers/concert/ConcertScraperInterface.h"
#include "scrapers/movie/CustomMovieScraper.h"
#include "scrapers/movie/IMDB.h"
#include "scrapers/movie/MovieScraperInterface.h"
#include "scrapers/tv_show/TheTvDb.h"
#include "scrapers/tv_show/TvScraperInterface.h"
#include "settings/Settings.h"

TvScraperSettingsWidget::TvScraperSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::TvScraperSettingsWidget)
{
    ui->setupUi(this);

    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tvScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tvScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

TvScraperSettingsWidget::~TvScraperSettingsWidget()
{
    delete ui;
}

void TvScraperSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void TvScraperSettingsWidget::loadSettings()
{
    QSet<ShowScraperInfos> tvInfos = {ShowScraperInfos::Title,
        ShowScraperInfos::Rating,
        ShowScraperInfos::FirstAired,
        ShowScraperInfos::Runtime,
        ShowScraperInfos::Director,
        ShowScraperInfos::Writer,
        ShowScraperInfos::Certification,
        ShowScraperInfos::Overview,
        ShowScraperInfos::Genres,
        ShowScraperInfos::Tags,
        ShowScraperInfos::Actors};

    ui->tvScraperTable->clearContents();
    ui->tvScraperTable->setRowCount(0);

    for (const auto info : tvInfos) {
        int row = ui->tvScraperTable->rowCount();
        ui->tvScraperTable->insertRow(row);
        ui->tvScraperTable->setItem(row, 0, new QTableWidgetItem(titleForTvScraperInfo(info)));
        ui->tvScraperTable->setCellWidget(row, 1, comboForTvScraperInfo(info));
    }
}

void TvScraperSettingsWidget::saveSettings()
{
    QMap<ShowScraperInfos, QString> tvScraper;
    for (int row = 0, n = ui->tvScraperTable->rowCount(); row < n; ++row) {
        auto box = dynamic_cast<QComboBox*>(ui->tvScraperTable->cellWidget(row, 1));
        ShowScraperInfos info = ShowScraperInfos(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        tvScraper.insert(info, scraper);
    }
    m_settings->setCustomTvScraper(tvScraper);
}

QComboBox* TvScraperSettingsWidget::comboForTvScraperInfo(const ShowScraperInfos info)
{
    QString currentScraper = m_settings->customTvScraper().value(info, "notset");

    auto box = new QComboBox();
    box->addItem("The TV DB", TheTvDb::scraperIdentifier);
    box->setItemData(0, static_cast<int>(info), Qt::UserRole + 1);

    box->addItem("IMDB", IMDB::scraperIdentifier);
    box->setItemData(1, static_cast<int>(info), Qt::UserRole + 1);

    if (currentScraper == IMDB::scraperIdentifier) {
        box->setCurrentIndex(1);
    }

    return box;
}

QString TvScraperSettingsWidget::titleForTvScraperInfo(const ShowScraperInfos info)
{
    switch (info) {
    case ShowScraperInfos::Title: return tr("Title");
    case ShowScraperInfos::Rating: return tr("Rating");
    case ShowScraperInfos::FirstAired: return tr("First Aired");
    case ShowScraperInfos::Runtime: return tr("Runtime");
    case ShowScraperInfos::Director: return tr("Director");
    case ShowScraperInfos::Writer: return tr("Writer");
    case ShowScraperInfos::Certification: return tr("Certification");
    case ShowScraperInfos::Overview: return tr("Plot");
    case ShowScraperInfos::Genres: return tr("Genres");
    case ShowScraperInfos::Tags: return tr("Tags");
    case ShowScraperInfos::Actors: return tr("Actors");
    default: return tr("Unsupported");
    }
}
