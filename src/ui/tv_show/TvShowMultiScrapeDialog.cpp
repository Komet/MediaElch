#include "TvShowMultiScrapeDialog.h"
#include "ui_TvShowMultiScrapeDialog.h"

#include <QDebug>
#include <utility>

#include "data/ImageCache.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "scrapers/tv_show/TvScraperInterface.h"

TvShowMultiScrapeDialog::TvShowMultiScrapeDialog(QWidget* parent) : QDialog(parent), ui(new Ui::TvShowMultiScrapeDialog)
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

    m_executed = false;
    m_currentShow = nullptr;
    m_currentEpisode = nullptr;

    ui->chkActors->setMyData(static_cast<int>(TvShowScraperInfos::Actors));
    ui->chkBanner->setMyData(static_cast<int>(TvShowScraperInfos::Banner));
    ui->chkCertification->setMyData(static_cast<int>(TvShowScraperInfos::Certification));
    ui->chkDirector->setMyData(static_cast<int>(TvShowScraperInfos::Director));
    ui->chkFanart->setMyData(static_cast<int>(TvShowScraperInfos::Fanart));
    ui->chkFirstAired->setMyData(static_cast<int>(TvShowScraperInfos::FirstAired));
    ui->chkGenres->setMyData(static_cast<int>(TvShowScraperInfos::Genres));
    ui->chkTags->setMyData(static_cast<int>(TvShowScraperInfos::Tags));
    ui->chkNetwork->setMyData(static_cast<int>(TvShowScraperInfos::Network));
    ui->chkOverview->setMyData(static_cast<int>(TvShowScraperInfos::Overview));
    ui->chkPoster->setMyData(static_cast<int>(TvShowScraperInfos::Poster));
    ui->chkRating->setMyData(static_cast<int>(TvShowScraperInfos::Rating));
    ui->chkSeasonPoster->setMyData(static_cast<int>(TvShowScraperInfos::SeasonPoster));
    ui->chkSeasonFanart->setMyData(static_cast<int>(TvShowScraperInfos::SeasonBackdrop));
    ui->chkSeasonBanner->setMyData(static_cast<int>(TvShowScraperInfos::SeasonBanner));
    ui->chkSeasonThumb->setMyData(static_cast<int>(TvShowScraperInfos::SeasonThumb));
    ui->chkEpisodeThumbnail->setMyData(static_cast<int>(TvShowScraperInfos::Thumbnail));
    ui->chkTitle->setMyData(static_cast<int>(TvShowScraperInfos::Title));
    ui->chkWriter->setMyData(static_cast<int>(TvShowScraperInfos::Writer));
    ui->chkExtraArts->setMyData(static_cast<int>(TvShowScraperInfos::ExtraArts));
    ui->chkRuntime->setMyData(static_cast<int>(TvShowScraperInfos::Runtime));
    ui->chkStatus->setMyData(static_cast<int>(TvShowScraperInfos::Status));

    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            connect(box, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onChkToggled);
        }
    }

    connect(ui->chkUnCheckAll, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onChkAllToggled);
    connect(ui->btnStartScraping, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onStartScraping);
    connect(ui->chkDvdOrder, &QAbstractButton::clicked, this, &TvShowMultiScrapeDialog::onChkDvdOrderToggled);

    m_scraperInterface = Manager::instance()->tvScrapers().at(0);

    m_downloadManager = new DownloadManager(this);
    connect(m_downloadManager, &DownloadManager::sigElemDownloaded, this, &TvShowMultiScrapeDialog::onDownloadFinished);
    connect(m_downloadManager, SIGNAL(allDownloadsFinished()), this, SLOT(onDownloadsFinished()));
}

TvShowMultiScrapeDialog::~TvShowMultiScrapeDialog()
{
    delete ui;
}

TvShowMultiScrapeDialog* TvShowMultiScrapeDialog::instance(QWidget* parent)
{
    static TvShowMultiScrapeDialog* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new TvShowMultiScrapeDialog(parent);
    }
    return m_instance;
}

QVector<TvShow*> TvShowMultiScrapeDialog::shows() const
{
    return m_shows;
}

void TvShowMultiScrapeDialog::setShows(const QVector<TvShow*>& shows)
{
    m_shows = shows;
}

QVector<TvShowEpisode*> TvShowMultiScrapeDialog::episodes() const
{
    return m_episodes;
}

void TvShowMultiScrapeDialog::setEpisodes(const QVector<TvShowEpisode*>& episodes)
{
    m_episodes = episodes;
}

int TvShowMultiScrapeDialog::exec()
{
    m_showQueue.clear();
    m_episodeQueue.clear();
    ui->itemCounter->setVisible(false);
    ui->btnCancel->setVisible(true);
    ui->btnClose->setVisible(false);
    ui->btnStartScraping->setVisible(true);
    ui->btnStartScraping->setEnabled(true);
    ui->chkAutoSave->setEnabled(true);
    ui->chkOnlyId->setEnabled(true);
    ui->chkDvdOrder->setEnabled(true);
    ui->progressAll->setValue(0);
    ui->progressItem->setValue(0);
    ui->groupBox->setEnabled(true);
    ui->title->clear();
    m_currentEpisode = nullptr;
    m_currentShow = nullptr;
    m_showIds.clear();
    m_executed = true;
    setCheckBoxesEnabled();
    adjustSize();

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyId->setChecked(Settings::instance()->multiScrapeOnlyWithId());
    ui->chkDvdOrder->setChecked(Settings::instance()->tvShowDvdOrder());

    return QDialog::exec();
}

void TvShowMultiScrapeDialog::accept()
{
    disconnect(
        m_scraperInterface, &TvScraperInterface::sigSearchDone, this, &TvShowMultiScrapeDialog::onSearchFinished);
    m_executed = false;
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyId->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::accept();
}

void TvShowMultiScrapeDialog::reject()
{
    m_downloadManager->abortDownloads();

    disconnect(
        m_scraperInterface, &TvScraperInterface::sigSearchDone, this, &TvShowMultiScrapeDialog::onSearchFinished);
    m_executed = false;

    m_showQueue.clear();
    m_episodeQueue.clear();

    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyId->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::reject();
}

void TvShowMultiScrapeDialog::onChkToggled()
{
    m_infosToLoad.clear();
    bool allToggled = true;
    for (const auto box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isEnabled() && box->isChecked() && box->myData().toInt() > 0) {
            m_infosToLoad.append(TvShowScraperInfos(box->myData().toInt()));
        }
        if (box->isEnabled() && !box->isChecked() && box->myData().toInt() > 0) {
            allToggled = false;
        }
    }

    ui->chkUnCheckAll->setChecked(allToggled);
    ui->btnStartScraping->setEnabled(!m_infosToLoad.isEmpty());
}

void TvShowMultiScrapeDialog::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isEnabled() && box->myData().toInt() > 0) {
            box->setChecked(checked);
        }
    }
    onChkToggled();
}

void TvShowMultiScrapeDialog::setCheckBoxesEnabled()
{
    for (MyCheckBox* box : ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            if (box->property("type").toString() == "both"
                || (box->property("type").toString() == "episode" && m_episodes.count() > 0)
                || (box->property("type").toString() == "show" && m_shows.count() > 0)) {
                box->setEnabled(true);
            } else {
                box->setEnabled(false);
            }
        }
    }
    onChkToggled();
}

void TvShowMultiScrapeDialog::onStartScraping()
{
    disconnect(
        m_scraperInterface, &TvScraperInterface::sigSearchDone, this, &TvShowMultiScrapeDialog::onSearchFinished);

    ui->groupBox->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyId->setEnabled(false);
    ui->chkDvdOrder->setEnabled(false);

    connect(m_scraperInterface,
        &TvScraperInterface::sigSearchDone,
        this,
        &TvShowMultiScrapeDialog::onSearchFinished,
        Qt::UniqueConnection);

    m_showQueue.append(m_shows.toList());
    m_episodeQueue.append(m_episodes.toList());

    ui->itemCounter->setText(QString("0/%1").arg(m_showQueue.count() + m_episodeQueue.count()));
    ui->itemCounter->setVisible(true);
    ui->progressAll->setMaximum(m_showQueue.count() + m_episodeQueue.count());
    scrapeNext();
}

void TvShowMultiScrapeDialog::scrapeNext()
{
    if (!m_executed) {
        return;
    }

    if ((m_currentShow != nullptr) && ui->chkAutoSave->isChecked()) {
        m_currentShow->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    }

    if ((m_currentEpisode != nullptr) && ui->chkAutoSave->isChecked()) {
        m_currentEpisode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
    }

    if (m_showQueue.isEmpty() && m_episodeQueue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    m_currentShow = nullptr;
    m_currentEpisode = nullptr;

    if (!m_showQueue.isEmpty()) {
        m_currentShow = m_showQueue.dequeue();
        ui->title->setText(m_currentShow->name().trimmed());
    } else if (!m_episodeQueue.isEmpty()) {
        m_currentEpisode = m_episodeQueue.dequeue();
        ui->title->setText(m_currentEpisode->name().trimmed());
    }

    int sum = m_shows.count() + m_episodes.count();
    ui->itemCounter->setText(QString("%1/%2").arg(sum - m_showQueue.count() - m_episodeQueue.count()).arg(sum));

    ui->progressAll->setValue(ui->progressAll->maximum() - m_showQueue.size() - m_episodeQueue.count() - 1);
    ui->progressItem->setValue(0);

    if (ui->chkOnlyId->isChecked()
        && (((m_currentShow != nullptr) && !m_currentShow->tvdbId().isValid())
            || ((m_currentEpisode != nullptr) && !m_currentEpisode->tvShow()->tvdbId().isValid()))) {
        scrapeNext();
        return;
    }

    if (m_currentShow != nullptr) {
        connect(m_currentShow.data(),
            &TvShow::sigLoaded,
            this,
            &TvShowMultiScrapeDialog::onInfoLoadDone,
            Qt::UniqueConnection);
        if (!m_currentShow->tvdbId().isValid()) {
            m_scraperInterface->search(m_currentShow->name().trimmed());
        } else {
            m_currentShow->loadData(m_currentShow->tvdbId(), m_scraperInterface, TvShowUpdateType::Show, m_infosToLoad);
        }
    } else if (m_currentEpisode != nullptr) {
        connect(m_currentEpisode.data(),
            &TvShowEpisode::sigLoaded,
            this,
            &TvShowMultiScrapeDialog::onEpisodeLoadDone,
            Qt::UniqueConnection);
        if (m_currentEpisode->tvShow()->tvdbId().isValid()) {
            m_currentEpisode->loadData(m_currentEpisode->tvShow()->tvdbId(), m_scraperInterface, m_infosToLoad);
        } else if (m_showIds.contains(m_currentEpisode->tvShow()->name().trimmed())) {
            m_currentEpisode->loadData(
                m_showIds.value(m_currentEpisode->tvShow()->name().trimmed()), m_scraperInterface, m_infosToLoad);
        } else {
            m_scraperInterface->search(m_currentEpisode->tvShow()->name().trimmed());
        }
    }
}

void TvShowMultiScrapeDialog::onSearchFinished(QVector<ScraperSearchResult> results)
{
    if (!m_executed) {
        return;
    }
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    if (m_currentShow != nullptr) {
        m_showIds.insert(m_currentShow->name(), TvDbId(results.first().id));
        m_currentShow->loadData(TvDbId(results.first().id), m_scraperInterface, TvShowUpdateType::Show, m_infosToLoad);
    } else if (m_currentEpisode != nullptr) {
        m_showIds.insert(m_currentEpisode->tvShow()->name(), TvDbId(results.first().id));
        m_currentEpisode->loadData(TvDbId(results.first().id), m_scraperInterface, m_infosToLoad);
    }
}

void TvShowMultiScrapeDialog::onScrapingFinished()
{
    ui->itemCounter->setVisible(false);
    int numberOfShows = m_shows.count();
    int numberOfEpisodes = m_episodes.count();
    if (ui->chkOnlyId->isChecked()) {
        numberOfShows = 0;
        numberOfEpisodes = 0;
        for (TvShow* show : m_shows) {
            if (show->tvdbId().isValid()) {
                numberOfShows++;
            }
        }
        for (TvShowEpisode* episode : m_episodes) {
            if (episode->tvShow()->tvdbId().isValid()) {
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

void TvShowMultiScrapeDialog::onInfoLoadDone(TvShow* show)
{
    if (!m_executed) {
        return;
    }

    if (show != m_currentShow) {
        return;
    }

    if (show->showMissingEpisodes()) {
        show->clearMissingEpisodes();
        show->fillMissingEpisodes();
    }

    QVector<ImageType> types = {ImageType::TvShowClearArt,
        ImageType::TvShowLogos,
        ImageType::TvShowCharacterArt,
        ImageType::TvShowThumb,
        ImageType::TvShowSeasonThumb};
    if (show->tvdbId().isValid() && m_infosToLoad.contains(TvShowScraperInfos::ExtraArts)) {
        Manager::instance()->fanartTv()->tvShowImages(show, show->tvdbId(), types);
        connect(Manager::instance()->fanartTv(),
            &ImageProviderInterface::sigTvShowImagesLoaded,
            this,
            &TvShowMultiScrapeDialog::onLoadDone,
            Qt::UniqueConnection);
    } else {
        QMap<ImageType, QVector<Poster>> map;
        onLoadDone(show, map);
    }
}

void TvShowMultiScrapeDialog::onLoadDone(TvShow* show, QMap<ImageType, QVector<Poster>> posters)
{
    if (!m_executed) {
        return;
    }

    if (show != m_currentShow) {
        return;
    }

    int downloadsSize = 0;
    if (!show->posters().empty() && m_infosToLoad.contains(TvShowScraperInfos::Poster)) {
        addDownload(ImageType::TvShowPoster, show->posters().at(0).originalUrl, show);
        downloadsSize++;
    }

    if (!show->backdrops().empty() && m_infosToLoad.contains(TvShowScraperInfos::Fanart)) {
        addDownload(ImageType::TvShowBackdrop, show->backdrops().at(0).originalUrl, show);
        downloadsSize++;
    }

    if (!show->banners().empty() && show->infosToLoad().contains(TvShowScraperInfos::Banner)) {
        addDownload(ImageType::TvShowBanner, show->banners().at(0).originalUrl, show);
        downloadsSize++;
    }

    QVector<SeasonNumber> thumbsForSeasons;
    QMapIterator<ImageType, QVector<Poster>> it(posters);
    while (it.hasNext()) {
        it.next();
        if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowClearArt
            && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowClearArt, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowCharacterArt
                   && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowCharacterArt, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowLogos
                   && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowLogos, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowThumb
                   && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowThumb, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::SeasonThumb) && it.key() == ImageType::TvShowSeasonThumb
                   && !it.value().isEmpty()) {
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

    if (m_infosToLoad.contains(TvShowScraperInfos::Actors) && Settings::instance()->downloadActorImages()) {
        for (Actor* actor : show->actors()) {
            if (actor->thumb.isEmpty()) {
                continue;
            }
            addDownload(ImageType::Actor, QUrl(actor->thumb), show, actor);
            downloadsSize++;
        }
    }

    for (SeasonNumber season : show->seasons()) {
        if (!show->seasonPosters(season).isEmpty() && m_infosToLoad.contains(TvShowScraperInfos::SeasonPoster)) {
            addDownload(ImageType::TvShowSeasonPoster, show->seasonPosters(season).at(0).originalUrl, show, season);
            downloadsSize++;
        }
        if (!show->seasonBackdrops(season).isEmpty() && m_infosToLoad.contains(TvShowScraperInfos::SeasonBackdrop)) {
            addDownload(ImageType::TvShowSeasonBackdrop, show->seasonBackdrops(season).at(0).originalUrl, show, season);
            downloadsSize++;
        }
        if (!show->seasonBanners(season).isEmpty() && show->infosToLoad().contains(TvShowScraperInfos::SeasonBanner)) {
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
    if (!m_executed) {
        return;
    }

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
        scrapeNext();
    }
}

void TvShowMultiScrapeDialog::onDownloadsFinished()
{
    if (!m_executed) {
        return;
    }

    scrapeNext();
}


void TvShowMultiScrapeDialog::onChkDvdOrderToggled()
{
    Settings::instance()->setTvShowDvdOrder(ui->chkDvdOrder->isChecked());
}

void TvShowMultiScrapeDialog::onEpisodeLoadDone()
{
    if (!m_executed) {
        return;
    }

    auto episode = dynamic_cast<TvShowEpisode*>(QObject::sender());
    if (episode == nullptr) {
        return;
    }

    if (m_infosToLoad.contains(TvShowScraperInfos::Thumbnail) && !episode->thumbnail().isEmpty()) {
        addDownload(ImageType::TvShowEpisodeThumb, episode->thumbnail(), episode);
    } else {
        scrapeNext();
    }
}
