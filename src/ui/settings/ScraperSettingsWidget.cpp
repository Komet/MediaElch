#include "ui/settings/ScraperSettingsWidget.h"
#include "ui_ScraperSettingsWidget.h"

#include "globals/Manager.h"
#include "scrapers/concert/ConcertScraper.h"
#include "scrapers/movie/MovieScraper.h"
#include "scrapers/movie/custom/CustomMovieScraper.h"
#include "scrapers/movie/imdb/ImdbMovie.h"
#include "scrapers/music/MusicScraper.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "settings/Settings.h"

ScraperSettingsWidget::ScraperSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ScraperSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->lblCustomMovieScraperHelp->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->lblCustomMovieScraperHelp->setFont(smallFont);
#endif

    ui->customScraperTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->customScraperTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->customScraperTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    int scraperCounter = 0;
    for (auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        if (scraper->hasSettings()) {
            auto* name = new QLabel("<b>" + scraper->meta().name + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            m_scraperRows.insert(scraper, scraperCounter);
            scraperCounter++;
        }
    }

    // TODO:
    // TV scraper settings.

    for (auto* scraper : Manager::instance()->scrapers().concertScrapers()) {
        if (scraper->hasSettings()) {
            auto* name = new QLabel("<b>" + scraper->meta().name + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }
    for (auto* scraper : Manager::instance()->scrapers().musicScrapers()) {
        if (scraper->hasSettings()) {
            auto* name = new QLabel("<b>" + scraper->name() + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    for (auto* scraper : Manager::instance()->imageProviders()) {
        if (scraper->hasSettings()) {
            auto* name = new QLabel("<b>" + scraper->meta().name + "</b>");
            name->setAlignment(Qt::AlignRight);
            name->setStyleSheet("margin-top: 3px;");
            ui->gridLayoutScrapers->addWidget(name, scraperCounter, 0);
            ui->gridLayoutScrapers->addWidget(scraper->settingsWidget(), scraperCounter, 1);
            scraperCounter++;
        }
    }

    // clang-format off
    connect(ui->chkEnableAdultScrapers, &QAbstractButton::clicked, this, &ScraperSettingsWidget::onShowAdultScrapers);
    // clang-format on
}

ScraperSettingsWidget::~ScraperSettingsWidget()
{
    delete ui;
}

void ScraperSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
    ui->tvScraperSettings->setSettings(settings);
    ui->customTvScraperSettings->setSettings(settings);
}

void ScraperSettingsWidget::loadSettings()
{
    ui->chkEnableAdultScrapers->setChecked(m_settings->showAdultScrapers());

    QSet<MovieScraperInfo> infos = {MovieScraperInfo::Title,
        MovieScraperInfo::Set,
        MovieScraperInfo::Tagline,
        MovieScraperInfo::Rating,
        MovieScraperInfo::Released,
        MovieScraperInfo::Runtime,
        MovieScraperInfo::Director,
        MovieScraperInfo::Writer,
        MovieScraperInfo::Certification,
        MovieScraperInfo::Trailer,
        MovieScraperInfo::Overview,
        MovieScraperInfo::Poster,
        MovieScraperInfo::Backdrop,
        MovieScraperInfo::Actors,
        MovieScraperInfo::Genres,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Logo,
        MovieScraperInfo::ClearArt,
        MovieScraperInfo::CdArt,
        MovieScraperInfo::Banner,
        MovieScraperInfo::Thumb};

    ui->customScraperTable->clearContents();
    ui->customScraperTable->setRowCount(0);

    for (const auto info : infos) {
        int row = ui->customScraperTable->rowCount();
        ui->customScraperTable->insertRow(row);
        ui->customScraperTable->setItem(row, 0, new QTableWidgetItem(titleForMovieScraperInfo(info)));
        ui->customScraperTable->setCellWidget(row, 1, comboForMovieScraperInfo(info));
    }

    ui->tvScraperSettings->loadSettings();
    ui->customTvScraperSettings->loadSettings();

    onShowAdultScrapers();
}

void ScraperSettingsWidget::saveSettings()
{
    m_settings->setShowAdultScrapers(ui->chkEnableAdultScrapers->isChecked());

    // Custom movie scraper
    QMap<MovieScraperInfo, QString> customMovieScraper;
    for (int row = 0, n = ui->customScraperTable->rowCount(); row < n; ++row) {
        auto* box = dynamic_cast<QComboBox*>(ui->customScraperTable->cellWidget(row, 1));
        MovieScraperInfo info = MovieScraperInfo(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        customMovieScraper.insert(info, scraper);
    }
    m_settings->setCustomMovieScraper(customMovieScraper);

    ui->tvScraperSettings->saveSettings();
    ui->customTvScraperSettings->saveSettings();
}

void ScraperSettingsWidget::onShowAdultScrapers()
{
    bool show = ui->chkEnableAdultScrapers->isChecked();
    for (const auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        if (scraper->meta().isAdult && scraper->hasSettings()) {
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 0)->widget()->setVisible(show);
            ui->gridLayoutScrapers->itemAtPosition(m_scraperRows.value(scraper), 1)->widget()->setVisible(show);
        }
    }
}

QComboBox* ScraperSettingsWidget::comboForMovieScraperInfo(const MovieScraperInfo info)
{
    QString currentScraper = m_settings->customMovieScraper().value(info, "notset");

    auto* box = new QComboBox();
    int index = 0;
    if (info != MovieScraperInfo::Title) {
        box->addItem(tr("Don't use"), "");
        box->setItemData(0, static_cast<int>(info), Qt::UserRole + 1);
        index = 1;
    }
    for (auto* scraper : Manager::instance()->scrapers().movieScrapers()) {
        if (scraper->meta().identifier == mediaelch::scraper::CustomMovieScraper::ID) {
            continue;
        }
        if (scraper->scraperNativelySupports().contains(info)) {
            box->addItem(scraper->meta().name, scraper->meta().identifier);
            box->setItemData(index, static_cast<int>(info), Qt::UserRole + 1);
            if (scraper->meta().identifier == currentScraper || (currentScraper == "notset" && index == 1)) {
                box->setCurrentIndex(index);
            }
            index++;
        }
    }

    QSet<MovieScraperInfo> images{MovieScraperInfo::Backdrop,
        MovieScraperInfo::Logo,
        MovieScraperInfo::ClearArt,
        MovieScraperInfo::CdArt,
        MovieScraperInfo::Banner,
        MovieScraperInfo::Thumb,
        MovieScraperInfo::Poster};

    if (images.contains(info)) {
        for (auto* const img : Manager::instance()->imageProviders()) {
            if (img->meta().identifier == mediaelch::scraper::FanartTv::ID) {
                box->addItem(img->meta().name, img->meta().identifier);
                box->setItemData(index, static_cast<int>(info), Qt::UserRole + 1);
                if (img->meta().identifier == currentScraper || (currentScraper == "notset" && index == 1)) {
                    box->setCurrentIndex(index);
                }
                index++;
                break;
            }
        }
    }

    return box;
}

QString ScraperSettingsWidget::titleForMovieScraperInfo(MovieScraperInfo info)
{
    switch (info) {
    case MovieScraperInfo::Title: return tr("Title");
    case MovieScraperInfo::Tagline: return tr("Tagline");
    case MovieScraperInfo::Rating: return tr("Rating");
    case MovieScraperInfo::Released: return tr("Released");
    case MovieScraperInfo::Runtime: return tr("Runtime");
    case MovieScraperInfo::Certification: return tr("Certification");
    case MovieScraperInfo::Trailer: return tr("Trailer");
    case MovieScraperInfo::Overview: return tr("Plot");
    case MovieScraperInfo::Poster: return tr("Poster");
    case MovieScraperInfo::Backdrop: return tr("Fanart");
    case MovieScraperInfo::Actors: return tr("Actors");
    case MovieScraperInfo::Genres: return tr("Genres");
    case MovieScraperInfo::Studios: return tr("Studios");
    case MovieScraperInfo::Countries: return tr("Countries");
    case MovieScraperInfo::Writer: return tr("Writer");
    case MovieScraperInfo::Director: return tr("Director");
    case MovieScraperInfo::Tags: return tr("Tags");
    case MovieScraperInfo::Set: return tr("Set");
    case MovieScraperInfo::Logo: return tr("Logo");
    case MovieScraperInfo::CdArt: return tr("Disc Art");
    case MovieScraperInfo::ClearArt: return tr("Clear Art");
    case MovieScraperInfo::Banner: return tr("Banner");
    case MovieScraperInfo::Thumb: return tr("Thumb");
    default: return tr("Unsupported");
    }
}
