#include "TvShowMultiScrapeDialog.h"
#include "ui_TvShowMultiScrapeDialog.h"

#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "scrapers/tv_show/TvScraper.h"
#include "scrapers/tv_show/custom/CustomTvScraper.h"
#include "scrapers/tv_show/imdb/ImdbTv.h"
#include "scrapers/tv_show/thetvdb/TheTvDb.h"
#include "scrapers/tv_show/tmdb/TmdbTv.h"
#include "scrapers/tv_show/tvmaze/TvMaze.h"
#include "ui/tv_show/TvShowCommonWidgets.h"

#include <QDebug>
#include <utility>

using namespace mediaelch;

/// \brief Get the appropriate ID for the given scraper. Used for the "with ID only" feature.
static scraper::ShowIdentifier getShowIdentifierForScraper(const scraper::TvScraper& scraper, const TvShow& show)
{
    using namespace mediaelch::scraper;
    const QString& scraperId = scraper.meta().identifier;

    if (scraperId == TheTvDb::ID) {
        return ShowIdentifier(show.tvdbId());
    }
    if (scraperId == ImdbTv::ID) {
        return ShowIdentifier(show.imdbId());
    }
    if (scraperId == TvMaze::ID) {
        return ShowIdentifier(show.tvmazeId());
    }
    if (scraperId == TmdbTv::ID || scraperId == CustomTvScraper::ID) {
        // The CustomTvScraper depends on TMDb
        return ShowIdentifier(show.tmdbId());
    }
    return ShowIdentifier();
}

static bool hasValidIdForScraper(const scraper::TvScraper& scraper, const TvShow& show)
{
    using namespace mediaelch::scraper;
    const QString& scraperId = scraper.meta().identifier;

    if (scraperId == TheTvDb::ID) {
        return show.tvdbId().isValid();
    }
    if (scraperId == ImdbTv::ID) {
        return show.imdbId().isValid();
    }
    if (scraperId == TvMaze::ID) {
        return show.tvmazeId().isValid();
    }
    if (scraperId == TmdbTv::ID || scraperId == CustomTvScraper::ID) {
        // The CustomTvScraper depends on TMDb
        return show.tmdbId().isValid();
    }
    return false;
}

TvShowMultiScrapeDialog::TvShowMultiScrapeDialog(QVector<TvShow*> shows,
    QVector<TvShowEpisode*> episodes,
    QWidget* parent) :
    QDialog(parent),
    ui(new Ui::TvShowMultiScrapeDialog),
    m_shows{std::move(shows)},
    m_episodes{std::move(episodes)},
    m_downloadManager{new DownloadManager(this)}
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->itemCounter->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->itemCounter->setFont(font);

    m_showQueue.append(m_shows.toList());
    m_episodeQueue.append(m_episodes.toList());

    ui->chkActors->setMyData(static_cast<int>(ShowScraperInfo::Actors));
    ui->chkBanner->setMyData(static_cast<int>(ShowScraperInfo::Banner));
    ui->chkCertification->setMyData(static_cast<int>(ShowScraperInfo::Certification));
    ui->chkFanart->setMyData(static_cast<int>(ShowScraperInfo::Fanart));
    ui->chkFirstAired->setMyData(static_cast<int>(ShowScraperInfo::FirstAired));
    ui->chkGenres->setMyData(static_cast<int>(ShowScraperInfo::Genres));
    ui->chkTags->setMyData(static_cast<int>(ShowScraperInfo::Tags));
    ui->chkNetwork->setMyData(static_cast<int>(ShowScraperInfo::Network));
    ui->chkOverview->setMyData(static_cast<int>(ShowScraperInfo::Overview));
    ui->chkPoster->setMyData(static_cast<int>(ShowScraperInfo::Poster));
    ui->chkRating->setMyData(static_cast<int>(ShowScraperInfo::Rating));
    ui->chkSeasonPoster->setMyData(static_cast<int>(ShowScraperInfo::SeasonPoster));
    ui->chkSeasonFanart->setMyData(static_cast<int>(ShowScraperInfo::SeasonBackdrop));
    ui->chkSeasonBanner->setMyData(static_cast<int>(ShowScraperInfo::SeasonBanner));
    ui->chkSeasonThumb->setMyData(static_cast<int>(ShowScraperInfo::SeasonThumb));
    ui->chkTitle->setMyData(static_cast<int>(ShowScraperInfo::Title));
    ui->chkExtraArts->setMyData(static_cast<int>(ShowScraperInfo::ExtraArts));
    ui->chkRuntime->setMyData(static_cast<int>(ShowScraperInfo::Runtime));
    ui->chkStatus->setMyData(static_cast<int>(ShowScraperInfo::Status));
    ui->chkThumb->setMyData(static_cast<int>(ShowScraperInfo::Thumb));

    ui->chkEpisodeActors->setMyData(static_cast<int>(EpisodeScraperInfo::Actors));
    ui->chkEpisodeCertification->setMyData(static_cast<int>(EpisodeScraperInfo::Certification));
    ui->chkEpisodeDirector->setMyData(static_cast<int>(EpisodeScraperInfo::Director));
    ui->chkEpisodeFirstAired->setMyData(static_cast<int>(EpisodeScraperInfo::FirstAired));
    ui->chkEpisodeNetwork->setMyData(static_cast<int>(EpisodeScraperInfo::Network));
    ui->chkEpisodeOverview->setMyData(static_cast<int>(EpisodeScraperInfo::Overview));
    ui->chkEpisodeRating->setMyData(static_cast<int>(EpisodeScraperInfo::Rating));
    ui->chkEpisodeTags->setMyData(static_cast<int>(EpisodeScraperInfo::Tags));
    ui->chkEpisodeThumbnail->setMyData(static_cast<int>(EpisodeScraperInfo::Thumbnail));
    ui->chkEpisodeTitle->setMyData(static_cast<int>(EpisodeScraperInfo::Title));
    ui->chkEpisodeWriter->setMyData(static_cast<int>(EpisodeScraperInfo::Writer));

    for (MyCheckBox* box : ui->showInfosGroupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onShowInfoToggled);
        }
    }

    for (MyCheckBox* box : ui->episodeInfosGroupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onEpisodeInfoToggled);
        }
    }

    // clang-format off
    connect(ui->chkUnCheckAll,        &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onChkAllShowInfosToggled);
    connect(ui->chkEpisodeUnCheckAll, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onChkAllEpisodeInfosToggled);
    connect(ui->btnStartScraping,     &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onStartScraping);

    auto indexChanged = elchOverload<int>(&QComboBox::currentIndexChanged);
    connect(ui->comboScraper,     indexChanged, this, &TvShowMultiScrapeDialog::onScraperChanged);
    connect(ui->comboSeasonOrder, indexChanged, this, &TvShowMultiScrapeDialog::onSeasonOrderChanged);
    connect(ui->comboLanguage,    &LanguageCombo::languageChanged, this, &TvShowMultiScrapeDialog::onLanguageChanged);

    auto queuedUnique = static_cast<Qt::ConnectionType>(Qt::QueuedConnection | Qt::UniqueConnection);
    connect(m_downloadManager, &DownloadManager::sigElemDownloaded,    this, &TvShowMultiScrapeDialog::onDownloadFinished, queuedUnique);
    connect(m_downloadManager, &DownloadManager::allDownloadsFinished, this, &TvShowMultiScrapeDialog::scrapeNext,         queuedUnique);
    // clang-format on
}

TvShowMultiScrapeDialog::~TvShowMultiScrapeDialog()
{
    delete ui;
}

QVector<TvShow*> TvShowMultiScrapeDialog::shows() const
{
    return m_shows;
}

QVector<TvShowEpisode*> TvShowMultiScrapeDialog::episodes() const
{
    return m_episodes;
}

int TvShowMultiScrapeDialog::exec()
{
    ui->itemCounter->setVisible(false);
    ui->btnCancel->setVisible(true);
    ui->btnClose->setVisible(false);
    ui->btnStartScraping->setEnabled(true);
    ui->chkAutoSave->setEnabled(true);
    ui->chkOnlyId->setEnabled(true);
    ui->comboSeasonOrder->setEnabled(true);
    ui->progressAll->setValue(0);
    ui->progressItem->setValue(0);
    ui->showInfosGroupBox->setEnabled(true);
    ui->episodeInfosGroupBox->setEnabled(true);
    ui->title->clear();

    m_currentEpisode = nullptr;
    m_currentShow = nullptr;

    adjustSize();

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyId->setChecked(Settings::instance()->multiScrapeOnlyWithId());

    setupScraperDropdown();
    setupLanguageDropdown();
    setupSeasonOrderComboBox();

    // Set active tab: Either episode or show depending on what shall be loaded.
    ui->tabWidget->setCurrentWidget(!isShowUpdateType(updateType()) ? ui->tabEpisodeDetails : ui->tabShowDetails);

    updateCheckBoxes();

    return QDialog::exec();
}

void TvShowMultiScrapeDialog::accept()
{
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyId->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::accept();
}

void TvShowMultiScrapeDialog::reject()
{
    m_downloadManager->abortDownloads();

    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyId->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::reject();
}

void TvShowMultiScrapeDialog::onShowInfoToggled()
{
    m_showDetailsToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->showInfosGroupBox->findChildren<MyCheckBox*>()) {
        if (box->isEnabled() && box->myData().toInt() > 0) {
            if (box->isChecked()) {
                m_showDetailsToLoad.insert(ShowScraperInfo(box->myData().toInt()));
            } else {
                allToggled = false;
            }
        }
    }

    ui->chkUnCheckAll->setChecked(allToggled);
    ui->btnStartScraping->setEnabled(!m_episodeDetailsToLoad.isEmpty() || !m_showDetailsToLoad.isEmpty());

    if (isShowUpdateType(updateType())) {
        // only store details if we want to load the show
        // otherwise these details will be lost because all checkboxes may be unchecked
        Settings::instance()->setScraperInfosShow(m_currentScraper->meta().identifier, m_showDetailsToLoad);
    }
}

void TvShowMultiScrapeDialog::onEpisodeInfoToggled()
{
    m_episodeDetailsToLoad.clear();
    bool allToggled = true;
    for (MyCheckBox* box : ui->episodeInfosGroupBox->findChildren<MyCheckBox*>()) {
        if (box->isEnabled() && box->myData().toInt() > 0) {
            if (box->isChecked()) {
                m_episodeDetailsToLoad.insert(EpisodeScraperInfo(box->myData().toInt()));
            } else {
                allToggled = false;
            }
        }
    }

    ui->chkEpisodeUnCheckAll->setChecked(allToggled);
    ui->btnStartScraping->setEnabled(!m_episodeDetailsToLoad.isEmpty() || !m_showDetailsToLoad.isEmpty());

    if (isEpisodeUpdateType(updateType())) {
        // only store details if we want to load episodes
        // otherwise these details will be lost because all checkboxes may be unchecked
        Settings::instance()->setScraperInfosEpisode(m_currentScraper->meta().identifier, m_episodeDetailsToLoad);
    }
}

void TvShowMultiScrapeDialog::onChkAllShowInfosToggled()
{
    const bool checked = ui->chkUnCheckAll->isChecked();
    for (MyCheckBox* box : ui->showInfosGroupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(checked);
        }
    }
    onShowInfoToggled();
}

void TvShowMultiScrapeDialog::onChkAllEpisodeInfosToggled()
{
    const bool checked = ui->chkEpisodeUnCheckAll->isChecked();
    for (MyCheckBox* box : ui->episodeInfosGroupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0 && box->isEnabled()) {
            box->setChecked(checked);
        }
    }
    onEpisodeInfoToggled();
}

void TvShowMultiScrapeDialog::onStartScraping()
{
    ui->showInfosGroupBox->setEnabled(false);
    ui->episodeInfosGroupBox->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyId->setEnabled(false);
    ui->comboSeasonOrder->setEnabled(false);
    ui->comboLanguage->setEnabled(false);
    ui->comboScraper->setEnabled(false);
    ui->txtScraperLog->clear();

    ui->itemCounter->setText(QStringLiteral("0/%1").arg(m_showQueue.count() + m_episodeQueue.count()));
    ui->itemCounter->setVisible(true);
    ui->progressAll->setMaximum(m_showQueue.count() + m_episodeQueue.count());

    logToUser(tr("Start scraping using \"%1\"").arg(m_currentScraper->meta().name));

    scrapeNext();
}

void TvShowMultiScrapeDialog::scrapeNext()
{
    qDebug() << "[TvShowMultiScrapeDialog] Scrape next item";
    using namespace mediaelch::scraper;

    saveCurrentItem();

    if (m_showQueue.isEmpty() && m_episodeQueue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    m_currentShow = nullptr;
    m_currentEpisode = nullptr;

    if (!m_showQueue.isEmpty()) {
        m_currentShow = m_showQueue.dequeue();
        ui->title->setText(m_currentShow->title().trimmed());

    } else if (!m_episodeQueue.isEmpty()) {
        m_currentEpisode = m_episodeQueue.dequeue();
        ui->title->setText(m_currentEpisode->title().trimmed());
    }

    const int sum = m_shows.count() + m_episodes.count();
    ui->itemCounter->setText(QStringLiteral("%1/%2").arg(sum - m_showQueue.count() - m_episodeQueue.count()).arg(sum));

    ui->progressAll->setValue(ui->progressAll->maximum() - m_showQueue.size() - m_episodeQueue.count() - 1);
    ui->progressItem->setValue(0);

    // Check if the show/episode has an ID that suits the current scraper.
    // If not and the "only with ID" checkbox is enabled, skip this show/episode.
    if (ui->chkOnlyId->isChecked()) {
        const TvShow* show = m_currentShow;
        if (show == nullptr && m_currentEpisode != nullptr) {
            show = m_currentEpisode->tvShow();
        }
        if (show != nullptr) {
            const auto id = getShowIdentifierForScraper(*m_currentScraper, *show);
            if (id.str().isEmpty()) {
                logToUser(tr("Skipping show \"%1\" because it does not have a valid ID."));
                scrapeNext();
                return;
            }
        }
    }

    if (m_currentShow != nullptr) {
        const auto id = getShowIdentifierForScraper(*m_currentScraper, *m_currentShow);

        // no useful id: search first
        if (id.str().isEmpty()) {
            // Most scrapers do not support a year, so we remove it.
            // Because the title may still be the filename / folder name, we also replace
            // the dot with space.
            // TODO: Use some common utility function for sanitization.
            QString searchQuery = m_currentShow->title().replace(".", " ").trimmed();
            searchQuery = ShowSearchJob::extractTitleAndYear(searchQuery).first;

            logToUser(tr("Search for TV show \"%1\" because no valid ID was found.").arg(searchQuery));
            ShowSearchJob::Config config{searchQuery, m_locale, Settings::instance()->showAdultScrapers()};
            auto* searchJob = m_currentScraper->search(config);
            connect(searchJob, &ShowSearchJob::sigFinished, this, &TvShowMultiScrapeDialog::onSearchFinished);
            searchJob->execute();

        } else {
            logToUser(tr("Scraping next TV show with ID \"%1\".").arg(id.str()));
            connect(m_currentShow.data(),
                &TvShow::sigLoaded,
                this,
                &TvShowMultiScrapeDialog::onInfoLoadDone,
                Qt::UniqueConnection);
            m_currentShow->scrapeData(m_currentScraper,
                id,
                m_locale,
                m_seasonOrder,
                TvShowUpdateType::Show,
                m_showDetailsToLoad,
                m_episodeDetailsToLoad);
        }

    } else if (m_currentEpisode != nullptr) {
        connect(m_currentEpisode.data(),
            &TvShowEpisode::sigLoaded,
            this,
            &TvShowMultiScrapeDialog::onEpisodeLoadDone,
            Qt::UniqueConnection);

        const QString title = m_currentEpisode->tvShow()->title();
        auto id = getShowIdentifierForScraper(*m_currentScraper, *m_currentEpisode->tvShow());

        if (id.str().isEmpty() && m_showIds.contains(title)) {
            id = m_showIds.value(title);
        }

        if (id.str().isEmpty()) {
            logToUser(tr("Search for TV show \"%1\" because no valid show ID was found for the episode.")
                          .arg(m_currentEpisode->tvShow()->title()));
            ShowSearchJob::Config config{
                m_currentEpisode->tvShow()->title(), m_locale, Settings::instance()->showAdultScrapers()};
            auto* searchJob = m_currentScraper->search(config);
            connect(searchJob, &ShowSearchJob::sigFinished, this, &TvShowMultiScrapeDialog::onSearchFinished);
            searchJob->execute();

        } else {
            logToUser(tr("S%1E%2: Scraping next episode with show ID \"%3\".")
                          .arg(m_currentEpisode->seasonNumber().toPaddedString(),
                              m_currentEpisode->episodeNumber().toPaddedString(),
                              id.str()));
            m_currentEpisode->scrapeData(m_currentScraper, m_locale, id, m_seasonOrder, m_episodeDetailsToLoad);
        }

    } else {
        qCritical() << "[TvShowMultiScrapeDialog] Cannot scrape next! No further items to process but initial check "
                       "did not notice it!";
        onScrapingFinished();
    }
}

void TvShowMultiScrapeDialog::saveCurrentItem()
{
    if (!ui->chkAutoSave->isChecked()) {
        return;
    }

    if (m_currentShow != nullptr) {
        m_currentShow->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    }

    if (m_currentEpisode != nullptr) {
        m_currentEpisode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    }
}

TvShowUpdateType TvShowMultiScrapeDialog::updateType() const
{
    // Create "fake" update type because the type is based on whether a show was selected or not.
    return m_shows.isEmpty() ? TvShowUpdateType::AllEpisodes : TvShowUpdateType::ShowAndAllEpisodes;
}

void TvShowMultiScrapeDialog::logToUser(const QString& msg)
{
    ui->txtScraperLog->appendPlainText(msg);
}

void TvShowMultiScrapeDialog::onSearchFinished(scraper::ShowSearchJob* searchJob)
{
    searchJob->deleteLater();

    if (searchJob->hasError()) {
        logToUser(tr("Error while searching for TV show: \"%1\"").arg(searchJob->error().message));
        scrapeNext();
        return;
    }
    if (searchJob->results().isEmpty()) {
        logToUser(tr("Did not find any results for search term \"%1\".").arg(searchJob->config().query));
        scrapeNext();
        return;
    }

    const auto id = searchJob->results().first().identifier;
    if (m_currentShow != nullptr) {
        logToUser(tr("Scraping next TV show with ID \"%1\".").arg(id.str()));
        m_showIds.insert(m_currentShow->title(), id);

        // TODO: deduplicate code with scrapeNext()
        connect(m_currentShow.data(),
            &TvShow::sigLoaded,
            this,
            &TvShowMultiScrapeDialog::onInfoLoadDone,
            Qt::UniqueConnection);
        m_currentShow->scrapeData(m_currentScraper,
            id,
            m_locale,
            m_seasonOrder,
            TvShowUpdateType::Show,
            m_showDetailsToLoad,
            m_episodeDetailsToLoad);

    } else if (m_currentEpisode != nullptr) {
        logToUser(tr("S%1E%2: Scraping next episode with show ID \"%3\".")
                      .arg(m_currentEpisode->seasonNumber().toPaddedString(),
                          m_currentEpisode->episodeNumber().toPaddedString(),
                          id.str()));
        m_showIds.insert(m_currentEpisode->tvShow()->title(), id);

        // TODO: deduplicate code with scrapeNext()
        connect(m_currentEpisode.data(),
            &TvShowEpisode::sigLoaded,
            this,
            &TvShowMultiScrapeDialog::onEpisodeLoadDone,
            Qt::UniqueConnection);
        m_currentEpisode->scrapeData(m_currentScraper, m_locale, id, m_seasonOrder, m_episodeDetailsToLoad);
    }
}

void TvShowMultiScrapeDialog::onScrapingFinished()
{
    logToUser(tr("Done."));

    ui->itemCounter->setVisible(false);
    int numberOfShows = m_shows.count();
    int numberOfEpisodes = m_episodes.count();
    if (ui->chkOnlyId->isChecked()) {
        numberOfShows = 0;
        numberOfEpisodes = 0;
        for (TvShow* show : m_shows) {
            if (hasValidIdForScraper(*m_currentScraper, *show)) {
                numberOfShows++;
            }
        }
        for (TvShowEpisode* episode : m_episodes) {
            if (episode->tvShow() != nullptr && hasValidIdForScraper(*m_currentScraper, *episode->tvShow())) {
                numberOfEpisodes++;
            }
        }
    }

    QString shows = tr("%n TV shows", "", numberOfShows);
    QString episodes = tr("%n episodes", "", numberOfEpisodes);
    if (numberOfShows > 0 && numberOfEpisodes > 0) {
        ui->title->setText(tr("Scraping of %1 and %2 has finished.").arg(shows).arg(episodes));
    } else if (numberOfShows > 0) {
        ui->title->setText(tr("Scraping of %1 has finished.").arg(shows));
    } else if (numberOfEpisodes > 0) {
        ui->title->setText(tr("Scraping of %1 has finished.").arg(episodes));
    }

    ui->progressAll->setValue(ui->progressAll->maximum());
    ui->btnCancel->setVisible(false);
    ui->btnClose->setVisible(true);
    ui->btnStartScraping->setVisible(false);
}

void TvShowMultiScrapeDialog::onInfoLoadDone(TvShow* show, QSet<ShowScraperInfo> details)
{
    Q_UNUSED(details);

    if (show != m_currentShow) {
        qCritical() << "[TvShowMultiScrapeDialog] TV show has changed mid-scrape-process!";
        return;
    }

    if (show->showMissingEpisodes()) {
        show->clearMissingEpisodes();
        show->fillMissingEpisodes();
    }

    logToUser(tr("Finished scraping details of TV show \"%1\".").arg(show->title()));

    QVector<ImageType> types = {ImageType::TvShowClearArt,
        ImageType::TvShowLogos,
        ImageType::TvShowCharacterArt,
        ImageType::TvShowThumb,
        ImageType::TvShowSeasonThumb};

    if (show->tvdbId().isValid() && details.contains(ShowScraperInfo::ExtraArts)) {
        logToUser(tr("Start loading extra fanart from TheTvDb for TV show with ID \"%1\".") //
                      .arg(show->tvdbId().toString()));
        connect(Manager::instance()->fanartTv(),
            &mediaelch::scraper::ImageProvider::sigTvShowImagesLoaded,
            this,
            &TvShowMultiScrapeDialog::onLoadDone,
            Qt::UniqueConnection);
        Manager::instance()->fanartTv()->tvShowImages(show, show->tvdbId(), types, m_locale);

    } else {
        onLoadDone(show, {});
    }
}

void TvShowMultiScrapeDialog::onLoadDone(TvShow* show, QMap<ImageType, QVector<Poster>> posters)
{
    if (show != m_currentShow) {
        qCritical() << "[TvShowMultiScrapeDialog] TV show has changed mid-scrape-process!";
        return;
    }

    int downloadsSize = 0;
    if (!show->posters().isEmpty() && m_showDetailsToLoad.contains(ShowScraperInfo::Poster)) {
        addDownload(ImageType::TvShowPoster, show->posters().at(0).originalUrl, show);
        downloadsSize++;
    }

    if (!show->backdrops().isEmpty() && m_showDetailsToLoad.contains(ShowScraperInfo::Fanart)) {
        addDownload(ImageType::TvShowBackdrop, show->backdrops().at(0).originalUrl, show);
        downloadsSize++;
    }

    if (!show->banners().isEmpty() && m_showDetailsToLoad.contains(ShowScraperInfo::Banner)) {
        addDownload(ImageType::TvShowBanner, show->banners().at(0).originalUrl, show);
        downloadsSize++;
    }

    QVector<SeasonNumber> thumbsForSeasons;
    QMapIterator<ImageType, QVector<Poster>> it(posters);
    while (it.hasNext()) {
        it.next();
        if (m_showDetailsToLoad.contains(ShowScraperInfo::ExtraArts) && it.key() == ImageType::TvShowClearArt
            && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowClearArt, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_showDetailsToLoad.contains(ShowScraperInfo::ExtraArts) && it.key() == ImageType::TvShowCharacterArt
                   && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowCharacterArt, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_showDetailsToLoad.contains(ShowScraperInfo::ExtraArts) && it.key() == ImageType::TvShowLogos
                   && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowLogos, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_showDetailsToLoad.contains(ShowScraperInfo::ExtraArts) && it.key() == ImageType::TvShowThumb
                   && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowThumb, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_showDetailsToLoad.contains(ShowScraperInfo::SeasonThumb)
                   && it.key() == ImageType::TvShowSeasonThumb && !it.value().isEmpty()) {
            for (const Poster& p : it.value()) {
                if (thumbsForSeasons.contains(p.season)) {
                    continue;
                }
                if (!show->seasons().contains(p.season)) {
                    continue;
                }

                addDownload(ImageType::TvShowSeasonThumb, p.originalUrl, show, p.season);
                downloadsSize++;
                thumbsForSeasons.append(p.season);
            }
        }
    }

    if (m_showDetailsToLoad.contains(ShowScraperInfo::Actors) && Settings::instance()->downloadActorImages()) {
        for (Actor* actor : show->actors()) {
            if (actor->thumb.isEmpty()) {
                continue;
            }
            addDownload(ImageType::Actor, QUrl(actor->thumb), show, actor);
            downloadsSize++;
        }
    }

    for (SeasonNumber season : show->seasons()) {
        if (!show->seasonPosters(season).isEmpty() && m_showDetailsToLoad.contains(ShowScraperInfo::SeasonPoster)) {
            addDownload(ImageType::TvShowSeasonPoster, show->seasonPosters(season).at(0).originalUrl, show, season);
            downloadsSize++;
        }
        if (!show->seasonBackdrops(season).isEmpty() && m_showDetailsToLoad.contains(ShowScraperInfo::SeasonBackdrop)) {
            addDownload(ImageType::TvShowSeasonBackdrop, show->seasonBackdrops(season).at(0).originalUrl, show, season);
            downloadsSize++;
        }
        if (!show->seasonBanners(season).isEmpty() && m_showDetailsToLoad.contains(ShowScraperInfo::SeasonBanner)) {
            addDownload(ImageType::TvShowSeasonBanner, show->seasonBanners(season).at(0).originalUrl, show, season);
            downloadsSize++;
        }
    }

    if (downloadsSize > 0) {
        ui->progressItem->setMaximum(downloadsSize);
    } else {
        scrapeNext();
    }
}

void TvShowMultiScrapeDialog::addDownload(ImageType imageType, QUrl url, TvShow* show, SeasonNumber season)
{
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = std::move(url);
    d.season = season;
    d.show = show;
    m_downloadManager->addDownload(d);
}

void TvShowMultiScrapeDialog::addDownload(ImageType imageType, QUrl url, TvShow* show, Actor* actor)
{
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = std::move(url);
    d.actor = actor;
    d.show = show;
    m_downloadManager->addDownload(d);
}

void TvShowMultiScrapeDialog::addDownload(ImageType imageType, QUrl url, TvShowEpisode* episode)
{
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = std::move(url);
    d.episode = episode;
    d.directDownload = true;
    m_downloadManager->addDownload(d);
}

void TvShowMultiScrapeDialog::onDownloadFinished(DownloadManagerElement elem)
{
    if (elem.show != nullptr) {
        int left = m_downloadManager->downloadsLeftForShow(m_currentShow);
        ui->progressItem->setValue(ui->progressItem->maximum() - left);
        qDebug() << "Download finished" << left << ui->progressItem->maximum();

        if (TvShow::seasonImageTypes().contains(elem.imageType)) {
            if (elem.imageType == ImageType::TvShowSeasonBackdrop) {
                helper::resizeBackdrop(elem.data);
            }
            ImageCache::instance()->invalidateImages(
                Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType, elem.season));
            elem.show->setSeasonImage(elem.season, elem.imageType, elem.data);
        } else if (elem.imageType != ImageType::Actor) {
            if (elem.imageType == ImageType::TvShowBackdrop) {
                helper::resizeBackdrop(elem.data);
            }
            ImageCache::instance()->invalidateImages(
                Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType));
            elem.show->setImage(elem.imageType, elem.data);
        }
    } else if ((elem.episode != nullptr) && elem.imageType == ImageType::TvShowEpisodeThumb) {
        elem.episode->setThumbnailImage(elem.data);
        // scrapeNext();
    }
}

void TvShowMultiScrapeDialog::setupSeasonOrderComboBox()
{
    m_seasonOrder = TvShowCommonWidgets::setupSeasonOrderComboBox(
        *m_currentScraper, Settings::instance()->seasonOrder(), ui->comboSeasonOrder);
}

void TvShowMultiScrapeDialog::updateCheckBoxes()
{
    TvShowCommonWidgets::toggleInfoBoxesForScraper(
        *m_currentScraper, updateType(), ui->showInfosGroupBox, ui->episodeInfosGroupBox);

    onShowInfoToggled();
    onEpisodeInfoToggled();
}

void TvShowMultiScrapeDialog::onScraperChanged(int index)
{
    if (index < 0 || index >= Manager::instance()->scrapers().movieScrapers().size()) {
        qCritical() << "[TvShowMultiScrapeDialog] Selected invalid scraper:" << index;
        showError(tr("Internal inconsistency: Selected an invalid scraper!"));
        return;
    }

    const QString scraperId = ui->comboScraper->itemData(index, Qt::UserRole).toString();
    qDebug() << "[TvShowMultiScrapeDialog] Selected scraper:" << scraperId;
    m_currentScraper = Manager::instance()->scrapers().tvScraper(scraperId);

    if (m_currentScraper == nullptr) {
        qFatal("[TvShowSearchWidget] Couldn't get scraper from manager");
    }

    // Save so that the scraper is auto-selected the next time.
    Settings::instance()->setCurrentTvShowScraper(scraperId);

    setupLanguageDropdown();
    setupSeasonOrderComboBox();
    updateCheckBoxes();
}

void TvShowMultiScrapeDialog::onLanguageChanged()
{
    const auto& meta = m_currentScraper->meta();
    m_locale = ui->comboLanguage->currentLocale();

    // Save immediately.
    ScraperSettings* scraperSettings = Settings::instance()->scraperSettings(meta.identifier);
    scraperSettings->setLanguage(m_locale);
    scraperSettings->save();
}

void TvShowMultiScrapeDialog::onSeasonOrderChanged(int index)
{
    bool ok = false;
    const int order = ui->comboSeasonOrder->itemData(index, Qt::UserRole).toInt(&ok);
    if (!ok) {
        qCritical() << "[TvShowMultiScrapeDialog] Invalid index for SeasonOrder";
        return;
    }
    m_seasonOrder = SeasonOrder(order);
    Settings::instance()->setSeasonOrder(m_seasonOrder);
}

void TvShowMultiScrapeDialog::showError(const QString& message)
{
    ui->lblError->setText(message);
    ui->lblError->show();
}

void TvShowMultiScrapeDialog::setupScraperDropdown()
{
    ui->comboScraper->blockSignals(true);
    ui->comboScraper->clear();

    for (const mediaelch::scraper::TvScraper* scraper : Manager::instance()->scrapers().tvScrapers()) {
        ui->comboScraper->addItem(scraper->meta().name, scraper->meta().identifier);
    }

    // Get the last selected scraper.
    const QString& currentScraperId = Settings::instance()->currentTvShowScraper();
    mediaelch::scraper::TvScraper* currentScraper = Manager::instance()->scrapers().tvScraper(currentScraperId);

    // The ID may not be a valid scraper. Default to first available scraper.
    if (currentScraper != nullptr) {
        m_currentScraper = currentScraper;
    } else {
        m_currentScraper = Manager::instance()->scrapers().tvScrapers().first();
    }

    const int index = ui->comboScraper->findData(m_currentScraper->meta().identifier);
    ui->comboScraper->setCurrentIndex(index);
    ui->comboScraper->blockSignals(false);
}

void TvShowMultiScrapeDialog::setupLanguageDropdown()
{
    if (m_currentScraper == nullptr) {
        ui->comboLanguage->setInvalid();
        qCritical() << "[TvShowSearch] Cannot set language dropdown in TV show search widget";
        showError(tr("Internal inconsistency: Cannot set language dropdown in TV show search widget!"));
        return;
    }

    const auto& meta = m_currentScraper->meta();
    m_locale = Settings::instance()->scraperSettings(meta.identifier)->language(meta.defaultLocale);
    ui->comboLanguage->setupLanguages(meta.supportedLanguages, m_locale);
}

void TvShowMultiScrapeDialog::onEpisodeLoadDone()
{
    auto* episode = dynamic_cast<TvShowEpisode*>(QObject::sender());
    if (episode == nullptr) {
        return;
    }

    logToUser(tr("S%2E%3: Finished scraping episode details. Title is: \"%1\".")
                  .arg(episode->title(),
                      m_currentEpisode->seasonNumber().toPaddedString(),
                      m_currentEpisode->episodeNumber().toPaddedString()));

    if (m_episodeDetailsToLoad.contains(EpisodeScraperInfo::Thumbnail) && !episode->thumbnail().isEmpty()) {
        addDownload(ImageType::TvShowEpisodeThumb, episode->thumbnail(), episode);
    } else {
        scrapeNext();
    }
}
