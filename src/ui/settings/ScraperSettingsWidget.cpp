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
#include "ui/settings/ConcertScraperInfoWidget.h"
#include "ui/settings/ImageProviderInfoWidget.h"
#include "ui/settings/MovieScraperInfoWidget.h"
#include "ui/settings/MusicScraperInfoWidget.h"
#include "ui/settings/ScraperSettingsTable.h"
#include "ui/settings/TvScraperInfoWidget.h"

namespace {

class MovieSettingsTableData : public mediaelch::scraper::SettingsTableData
{
public:
    MovieSettingsTableData(const mediaelch::ManagedMovieScraper& scraper) : m_scraper{scraper} {}
    ~MovieSettingsTableData() override = default;

    QString scraperName() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return scraper->meta().name;
    }

    QWidget* createScraperInfoWidget() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return new MovieScraperInfoWidget(*scraper);
    }

    QWidget* createScraperSettingsWidget() const override { return m_scraper.createView(); }

    bool isAdultScraper() const override { return m_scraper.scraper()->meta().isAdult; }

private:
    const mediaelch::ManagedMovieScraper& m_scraper;
};

class TvSettingsTableData : public mediaelch::scraper::SettingsTableData
{
public:
    TvSettingsTableData(const mediaelch::ManagedTvScraper& scraper) : m_scraper{scraper} {}
    ~TvSettingsTableData() override = default;

    QString scraperName() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return scraper->meta().name;
    }

    QWidget* createScraperInfoWidget() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return new TvScraperInfoWidget(*scraper);
    }

    QWidget* createScraperSettingsWidget() const override { return m_scraper.createView(); }

    bool isAdultScraper() const override { return false; }

private:
    const mediaelch::ManagedTvScraper& m_scraper;
};

class ConcertSettingsTableData : public mediaelch::scraper::SettingsTableData
{
public:
    ConcertSettingsTableData(const mediaelch::ManagedConcertScraper& scraper) : m_scraper{scraper} {}
    ~ConcertSettingsTableData() override = default;

    QString scraperName() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return scraper->meta().name;
    }

    QWidget* createScraperInfoWidget() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return new ConcertScraperInfoWidget(*scraper);
    }

    QWidget* createScraperSettingsWidget() const override { return m_scraper.createView(); }

    bool isAdultScraper() const override { return false; }

private:
    const mediaelch::ManagedConcertScraper& m_scraper;
};

class MusicSettingsTableData : public mediaelch::scraper::SettingsTableData
{
public:
    MusicSettingsTableData(const mediaelch::ManagedMusicScraper& scraper) : m_scraper{scraper} {}
    ~MusicSettingsTableData() override = default;

    QString scraperName() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return scraper->meta().name;
    }

    QWidget* createScraperInfoWidget() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return new MusicScraperInfoWidget(*scraper);
    }

    QWidget* createScraperSettingsWidget() const override { return m_scraper.createView(); }

    bool isAdultScraper() const override { return false; }

private:
    const mediaelch::ManagedMusicScraper& m_scraper;
};

class ImageSettingsTableData : public mediaelch::scraper::SettingsTableData
{
public:
    ImageSettingsTableData(const mediaelch::ManagedImageProvider& scraper) : m_scraper{scraper} {}
    ~ImageSettingsTableData() override = default;

    QString scraperName() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return scraper->meta().name;
    }

    QWidget* createScraperInfoWidget() const override
    {
        auto* scraper = m_scraper.scraper();
        MediaElch_Ensures(scraper != nullptr);
        return new ImageProviderInfoWidget(*scraper);
    }

    QWidget* createScraperSettingsWidget() const override { return m_scraper.createView(); }

    bool isAdultScraper() const override { return false; }

private:
    const mediaelch::ManagedImageProvider& m_scraper;
};


} // namespace

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

    connect(
        this, &ScraperSettingsWidget::saveSettings, this, &ScraperSettingsWidget::onSaveSettings, Qt::DirectConnection);

    ui->scraperSettings->clear();
    ui->scraperSettings->addDelimiter(tr("Movies"), tr("Select a movie scraper in the list."));
    for (const auto& scraper : Manager::instance()->scrapers().allMovieScrapers()) {
        ui->scraperSettings->addScraper(MovieSettingsTableData{scraper});
    }
    ui->scraperSettings->addDelimiter(tr("TV shows"), tr("Select a TV show scraper in the list."));
    for (const auto& scraper : Manager::instance()->scrapers().allTvScrapers()) {
        ui->scraperSettings->addScraper(TvSettingsTableData{scraper});
    }
    ui->scraperSettings->addDelimiter(tr("Concerts"), tr("Select a concert scraper in the list."));
    for (const auto& scraper : Manager::instance()->scrapers().allConcertScrapers()) {
        ui->scraperSettings->addScraper(ConcertSettingsTableData{scraper});
    }
    ui->scraperSettings->addDelimiter(tr("Music"), tr("Select a music scraper in the list."));
    for (const auto& scraper : Manager::instance()->scrapers().allMusicScrapers()) {
        ui->scraperSettings->addScraper(MusicSettingsTableData{scraper});
    }
    ui->scraperSettings->addDelimiter(tr("Images"), tr("Select an image provider in the list."));
    for (const auto& scraper : Manager::instance()->scrapers().allImageProviders()) {
        ui->scraperSettings->addScraper(ImageSettingsTableData{scraper});
    }
}

ScraperSettingsWidget::~ScraperSettingsWidget()
{
    delete ui;
}

void ScraperSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
    ui->customTvScraperSettings->setSettings(settings);
}

void ScraperSettingsWidget::loadSettings()
{
    using namespace mediaelch;

    // QVector instead of QSet to keep order.
    // TODO: not everything is listed, but should be!
    QVector<MovieScraperInfo> infos = {MovieScraperInfo::Title,
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
        MovieScraperInfo::Tags,
        MovieScraperInfo::Studios,
        MovieScraperInfo::Countries,
        MovieScraperInfo::Logo,
        MovieScraperInfo::ClearArt,
        MovieScraperInfo::CdArt,
        MovieScraperInfo::Banner,
        MovieScraperInfo::Thumb};

#ifdef QT_DEBUG
    { // TODO: Maybe a simple macro?
        QSet<MovieScraperInfo> deduplicated{infos.cbegin(), infos.cend()};
        MediaElch_Assert(deduplicated.size() == infos.size());
    }
#endif

    ui->customScraperTable->clearContents();
    ui->customScraperTable->setRowCount(0);

    for (const auto info : infos) {
        int row = ui->customScraperTable->rowCount();
        ui->customScraperTable->insertRow(row);
        ui->customScraperTable->setItem(row, 0, new QTableWidgetItem(titleForMovieScraperInfo(info)));
        ui->customScraperTable->setCellWidget(row, 1, comboForMovieScraperInfo(info));
    }

    ui->customTvScraperSettings->loadSettings();

    ui->scraperSettings->setShowAdultScraper(m_settings->showAdultScrapers());
}

void ScraperSettingsWidget::onSaveSettings()
{
    // Custom movie scraper
    QMap<MovieScraperInfo, QString> customMovieScraper;
    for (int row = 0, n = ui->customScraperTable->rowCount(); row < n; ++row) {
        auto* box = dynamic_cast<QComboBox*>(ui->customScraperTable->cellWidget(row, 1));
        MovieScraperInfo info = MovieScraperInfo(box->itemData(0, Qt::UserRole + 1).toInt());
        QString scraper = box->itemData(box->currentIndex()).toString();
        customMovieScraper.insert(info, scraper);
    }
    m_settings->setCustomMovieScraper(customMovieScraper);

    // ui->scraperSettings->saveSettings();
    ui->customTvScraperSettings->saveSettings();

    m_settings->setShowAdultScrapers(ui->scraperSettings->showAdultScraper());
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
    // TODO: Only list those that are supported by the CustomMovieScraper
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
        for (auto* const img : Manager::instance()->scrapers().imageProviders()) {
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
    case MovieScraperInfo::TvShowLinks: return tr("TV Show Links");
    default: return tr("Unsupported");
    }
}
