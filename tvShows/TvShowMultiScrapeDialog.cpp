#include "TvShowMultiScrapeDialog.h"
#include "ui_TvShowMultiScrapeDialog.h"

#include <QDebug>
#include "data/ImageCache.h"
#include "data/TvScraperInterface.h"
#include "globals/Helper.h"
#include "globals/Manager.h"

TvShowMultiScrapeDialog::TvShowMultiScrapeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TvShowMultiScrapeDialog)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->itemCounter->font();
#ifdef Q_OS_WIN32
    font.setPointSize(font.pointSize()-1);
#else
    font.setPointSize(font.pointSize()-2);
#endif
    ui->itemCounter->setFont(font);

    m_executed = false;
    m_currentShow = 0;
    m_currentEpisode = 0;

    ui->chkActors->setMyData(TvShowScraperInfos::Actors);
    ui->chkBanner->setMyData(TvShowScraperInfos::Banner);
    ui->chkCertification->setMyData(TvShowScraperInfos::Certification);
    ui->chkDirector->setMyData(TvShowScraperInfos::Director);
    ui->chkFanart->setMyData(TvShowScraperInfos::Fanart);
    ui->chkFirstAired->setMyData(TvShowScraperInfos::FirstAired);
    ui->chkGenres->setMyData(TvShowScraperInfos::Genres);
    ui->chkNetwork->setMyData(TvShowScraperInfos::Network);
    ui->chkOverview->setMyData(TvShowScraperInfos::Overview);
    ui->chkPoster->setMyData(TvShowScraperInfos::Poster);
    ui->chkRating->setMyData(TvShowScraperInfos::Rating);
    ui->chkSeasonPoster->setMyData(TvShowScraperInfos::SeasonPoster);
    ui->chkSeasonFanart->setMyData(TvShowScraperInfos::SeasonBackdrop);
    ui->chkSeasonBanner->setMyData(TvShowScraperInfos::SeasonBanner);
    ui->chkSeasonThumb->setMyData(TvShowScraperInfos::SeasonThumb);
    ui->chkEpisodeThumbnail->setMyData(TvShowScraperInfos::Thumbnail);
    ui->chkTitle->setMyData(TvShowScraperInfos::Title);
    ui->chkWriter->setMyData(TvShowScraperInfos::Writer);
    ui->chkExtraArts->setMyData(TvShowScraperInfos::ExtraArts);
    ui->chkRuntime->setMyData(TvShowScraperInfos::Runtime);
    ui->chkStatus->setMyData(TvShowScraperInfos::Status);

    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0)
            connect(box, SIGNAL(clicked()), this, SLOT(onChkToggled()));
    }

    connect(ui->chkUnCheckAll, SIGNAL(clicked()), this, SLOT(onChkAllToggled()));
    connect(ui->btnStartScraping, SIGNAL(clicked()), this, SLOT(onStartScraping()));
    connect(ui->chkDvdOrder, SIGNAL(clicked()), this, SLOT(onChkDvdOrderToggled()));

    m_scraperInterface = Manager::instance()->tvScrapers().at(0);

    m_downloadManager = new DownloadManager(this);
    connect(m_downloadManager, SIGNAL(sigElemDownloaded(DownloadManagerElement)), this, SLOT(onDownloadFinished(DownloadManagerElement)));
    connect(m_downloadManager, SIGNAL(allDownloadsFinished()), this, SLOT(onDownloadsFinished()));
}

TvShowMultiScrapeDialog::~TvShowMultiScrapeDialog()
{
    delete ui;
}

TvShowMultiScrapeDialog *TvShowMultiScrapeDialog::instance(QWidget *parent)
{
    static TvShowMultiScrapeDialog *m_instance = 0;
    if (m_instance == 0)
        m_instance = new TvShowMultiScrapeDialog(parent);
    return m_instance;
}

QList<TvShow *> TvShowMultiScrapeDialog::shows() const
{
    return m_shows;
}

void TvShowMultiScrapeDialog::setShows(const QList<TvShow *> &shows)
{
    m_shows = shows;
}

QList<TvShowEpisode *> TvShowMultiScrapeDialog::episodes() const
{
    return m_episodes;
}

void TvShowMultiScrapeDialog::setEpisodes(const QList<TvShowEpisode *> &episodes)
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
    m_currentEpisode = 0;
    m_currentShow = 0;
    m_showIds.clear();
    m_executed = true;
    setChkBoxesEnabled();
    adjustSize();

    ui->chkAutoSave->setChecked(Settings::instance()->multiScrapeSaveEach());
    ui->chkOnlyId->setChecked(Settings::instance()->multiScrapeOnlyWithId());
    ui->chkDvdOrder->setChecked(Settings::instance()->tvShowDvdOrder());

    return QDialog::exec();
}

void TvShowMultiScrapeDialog::accept()
{
    disconnect(m_scraperInterface, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)));
    m_executed = false;
    Settings::instance()->setMultiScrapeOnlyWithId(ui->chkOnlyId->isChecked());
    Settings::instance()->setMultiScrapeSaveEach(ui->chkAutoSave->isChecked());
    Settings::instance()->saveSettings();
    QDialog::accept();
}

void TvShowMultiScrapeDialog::reject()
{
    m_downloadManager->abortDownloads();

    disconnect(m_scraperInterface, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)));
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
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isEnabled() && box->isChecked() && box->myData().toInt() > 0)
            m_infosToLoad.append(box->myData().toInt());
        if (box->isEnabled() && !box->isChecked() && box->myData().toInt() > 0)
            allToggled = false;
    }

    ui->chkUnCheckAll->setChecked(allToggled);
    ui->btnStartScraping->setEnabled(!m_infosToLoad.isEmpty());
}

void TvShowMultiScrapeDialog::onChkAllToggled()
{
    bool checked = ui->chkUnCheckAll->isChecked();
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->isEnabled() && box->myData().toInt() > 0)
            box->setChecked(checked);
    }
    onChkToggled();
}

void TvShowMultiScrapeDialog::setChkBoxesEnabled()
{
    foreach (MyCheckBox *box, ui->groupBox->findChildren<MyCheckBox*>()) {
        if (box->myData().toInt() > 0) {
            if (box->property("type").toString() == "both" || (box->property("type").toString() == "episode" && m_episodes.count() > 0) || (box->property("type").toString() == "show" && m_shows.count() > 0))
                box->setEnabled(true);
            else
                box->setEnabled(false);
        }
    }
    onChkToggled();
}

void TvShowMultiScrapeDialog::onStartScraping()
{
    disconnect(m_scraperInterface, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)));

    ui->groupBox->setEnabled(false);
    ui->btnStartScraping->setEnabled(false);
    ui->chkAutoSave->setEnabled(false);
    ui->chkOnlyId->setEnabled(false);
    ui->chkDvdOrder->setEnabled(false);

    connect(m_scraperInterface, SIGNAL(sigSearchDone(QList<ScraperSearchResult>)), this, SLOT(onSearchFinished(QList<ScraperSearchResult>)), Qt::UniqueConnection);

    m_showQueue.append(m_shows);
    m_episodeQueue.append(m_episodes);

    ui->itemCounter->setText(QString("0/%1").arg(m_showQueue.count() + m_episodeQueue.count()));
    ui->itemCounter->setVisible(true);
    ui->progressAll->setMaximum(m_showQueue.count() + m_episodeQueue.count());
    scrapeNext();
}

void TvShowMultiScrapeDialog::scrapeNext()
{
    if (!m_executed)
        return;

    if (m_currentShow && ui->chkAutoSave->isChecked())
        m_currentShow->saveData(Manager::instance()->mediaCenterInterfaceTvShow());

    if (m_currentEpisode && ui->chkAutoSave->isChecked())
        m_currentEpisode->saveData(Manager::instance()->mediaCenterInterfaceTvShow());

    if (m_showQueue.isEmpty() && m_episodeQueue.isEmpty()) {
        onScrapingFinished();
        return;
    }

    m_currentShow = 0;
    m_currentEpisode = 0;

    if (!m_showQueue.isEmpty()) {
        m_currentShow = m_showQueue.dequeue();
        ui->title->setText(m_currentShow->name());
    } else if (!m_episodeQueue.isEmpty()) {
        m_currentEpisode = m_episodeQueue.dequeue();
        ui->title->setText(m_currentEpisode->name());
    }

    int sum = m_shows.count() + m_episodes.count();
    ui->itemCounter->setText(QString("%1/%2").arg(sum-m_showQueue.count()-m_episodeQueue.count()).arg(sum));

    ui->progressAll->setValue(ui->progressAll->maximum()-m_showQueue.size()-m_episodeQueue.count()-1);
    ui->progressItem->setValue(0);

    if (ui->chkOnlyId->isChecked() && ((m_currentShow && m_currentShow->tvdbId() == "") || (m_currentEpisode && m_currentEpisode->tvShow()->tvdbId() == ""))) {
        scrapeNext();
        return;
    }

    if (m_currentShow) {
        connect(m_currentShow, SIGNAL(sigLoaded(TvShow*)), this, SLOT(onInfoLoadDone(TvShow*)), Qt::UniqueConnection);
        if (m_currentShow->tvdbId().isEmpty())
            m_scraperInterface->search(m_currentShow->name());
        else
            m_currentShow->loadData(m_currentShow->tvdbId(), m_scraperInterface, UpdateShow, m_infosToLoad);
    } else if (m_currentEpisode) {
        connect(m_currentEpisode, SIGNAL(sigLoaded()), this, SLOT(onEpisodeLoadDone()), Qt::UniqueConnection);
        if (!m_currentEpisode->tvShow()->tvdbId().isEmpty())
            m_currentEpisode->loadData(m_currentEpisode->tvShow()->tvdbId(), m_scraperInterface, m_infosToLoad);
        else if (m_showIds.contains(m_currentEpisode->tvShow()->name()))
            m_currentEpisode->loadData(m_showIds.value(m_currentEpisode->tvShow()->name()), m_scraperInterface, m_infosToLoad);
        else
            m_scraperInterface->search(m_currentEpisode->tvShow()->name());
    }
}

void TvShowMultiScrapeDialog::onSearchFinished(QList<ScraperSearchResult> results)
{
    if (!m_executed)
        return;
    if (results.isEmpty()) {
        scrapeNext();
        return;
    }

    if (m_currentShow) {
        m_showIds.insert(m_currentShow->name(), results.first().id);
        m_currentShow->loadData(results.first().id, m_scraperInterface, UpdateShow, m_infosToLoad);
    } else if (m_currentEpisode) {
        m_showIds.insert(m_currentEpisode->tvShow()->name(), results.first().id);
        m_currentEpisode->loadData(results.first().id, m_scraperInterface, m_infosToLoad);
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
        foreach (TvShow *show, m_shows) {
            if (!show->tvdbId().isEmpty())
                numberOfShows++;
        }
        foreach (TvShowEpisode *episode, m_episodes) {
            if (!episode->tvShow()->tvdbId().isEmpty())
                numberOfEpisodes++;
        }
    }

    QString shows = tr("%n tv shows", "", numberOfShows);
    QString episodes = tr("%n episodes", "", numberOfEpisodes);
    if (numberOfShows > 0 && numberOfEpisodes > 0)
        ui->title->setText(tr("Scraping of %1 and %2 has finished.").arg(shows).arg(episodes));
    else if (numberOfShows > 0)
        ui->title->setText(tr("Scraping of %1 has finished.").arg(shows));
    else if (numberOfEpisodes > 0)
        ui->title->setText(tr("Scraping of %1 has finished.").arg(episodes));

    ui->progressAll->setValue(ui->progressAll->maximum());
    ui->btnCancel->setVisible(false);
    ui->btnClose->setVisible(true);
    ui->btnStartScraping->setVisible(false);
}

void TvShowMultiScrapeDialog::onInfoLoadDone(TvShow *show)
{
    if (!m_executed)
        return;

    if (show != m_currentShow)
        return;

    if (show->showMissingEpisodes()) {
        show->clearMissingEpisodes();
        show->fillMissingEpisodes();
    }

    QList<int> types;
    types << ImageType::TvShowClearArt << ImageType::TvShowLogos << ImageType::TvShowCharacterArt << ImageType::TvShowThumb << ImageType::TvShowSeasonThumb;
    if (!show->tvdbId().isEmpty() && m_infosToLoad.contains(TvShowScraperInfos::ExtraArts)) {
        Manager::instance()->fanartTv()->tvShowImages(show, show->tvdbId(), types);
        connect(Manager::instance()->fanartTv(), SIGNAL(sigImagesLoaded(TvShow*,QMap<int,QList<Poster> >)), this, SLOT(onLoadDone(TvShow*,QMap<int,QList<Poster> >)), Qt::UniqueConnection);
    } else {
        QMap<int, QList<Poster> > map;
        onLoadDone(show, map);
    }
}

void TvShowMultiScrapeDialog::onLoadDone(TvShow *show, QMap<int, QList<Poster> > posters)
{
    if (!m_executed)
        return;

    if (show != m_currentShow)
        return;

    int downloadsSize = 0;
    if (show->posters().size() > 0 && m_infosToLoad.contains(TvShowScraperInfos::Poster)) {
        addDownload(ImageType::TvShowPoster, show->posters().at(0).originalUrl, show);
        downloadsSize++;
    }

    if (show->backdrops().size() > 0 && m_infosToLoad.contains(TvShowScraperInfos::Fanart)) {
        addDownload(ImageType::TvShowBackdrop, show->backdrops().at(0).originalUrl, show);
        downloadsSize++;
    }

    if (show->banners().size() > 0 && show->infosToLoad().contains(TvShowScraperInfos::Banner)) {
        addDownload(ImageType::TvShowBanner, show->banners().at(0).originalUrl, show);
        downloadsSize++;
    }

    QList<int> thumbsForSeasons;
    QMapIterator<int, QList<Poster> > it(posters);
    while (it.hasNext()) {
        it.next();
        if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowClearArt && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowClearArt, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowCharacterArt && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowCharacterArt, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowLogos && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowLogos, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::ExtraArts) && it.key() == ImageType::TvShowThumb && !it.value().isEmpty()) {
            addDownload(ImageType::TvShowThumb, it.value().at(0).originalUrl, show);
            downloadsSize++;
        } else if (m_infosToLoad.contains(TvShowScraperInfos::SeasonThumb) && it.key() == ImageType::TvShowSeasonThumb && !it.value().isEmpty()) {
            foreach (Poster p, it.value()) {
                if (thumbsForSeasons.contains(p.season))
                    continue;
                if (!show->seasons().contains(p.season))
                    continue;

                addDownload(ImageType::TvShowSeasonThumb, p.originalUrl, show, p.season);
                downloadsSize++;
                thumbsForSeasons.append(p.season);
            }
        }
    }

    if (m_infosToLoad.contains(TvShowScraperInfos::Actors) && Settings::instance()->downloadActorImages()) {
        QList<Actor*> actors = show->actorsPointer();
        for (int i=0, n=actors.size() ; i<n ; ++i) {
            if (actors.at(i)->thumb.isEmpty())
                continue;
            addDownload(ImageType::Actor, QUrl(actors.at(i)->thumb), show, actors.at(i));
            downloadsSize++;
        }
    }

    foreach (int season, show->seasons()) {
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

    if (downloadsSize > 0)
        ui->progressItem->setMaximum(downloadsSize);
    else
        scrapeNext();
}

void TvShowMultiScrapeDialog::addDownload(int imageType, QUrl url, TvShow *show, int season)
{
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = url;
    d.season = season;
    d.show = show;
    m_downloadManager->addDownload(d);
}

void TvShowMultiScrapeDialog::addDownload(int imageType, QUrl url, TvShow *show, Actor *actor)
{
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = url;
    d.actor = actor;
    d.show = show;
    m_downloadManager->addDownload(d);
}

void TvShowMultiScrapeDialog::addDownload(int imageType, QUrl url, TvShowEpisode *episode)
{
    DownloadManagerElement d;
    d.imageType = imageType;
    d.url = url;
    d.episode = episode;
    d.directDownload = true;
    m_downloadManager->addDownload(d);
}

void TvShowMultiScrapeDialog::onDownloadFinished(DownloadManagerElement elem)
{
    if (!m_executed)
        return;

    if (elem.show) {
        int left = m_downloadManager->downloadsLeftForShow(m_currentShow);
        ui->progressItem->setValue(ui->progressItem->maximum()-left);
        qDebug() << "Download finished" << left << ui->progressItem->maximum();

        if (TvShow::seasonImageTypes().contains(elem.imageType)) {
            if (elem.imageType == ImageType::TvShowSeasonBackdrop)
                Helper::instance()->resizeBackdrop(elem.data);
            ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType, elem.season));
            elem.show->setSeasonImage(elem.season, elem.imageType, elem.data);
        } else if (elem.imageType != ImageType::Actor) {
            if (elem.imageType == ImageType::TvShowBackdrop)
                Helper::instance()->resizeBackdrop(elem.data);
            ImageCache::instance()->invalidateImages(Manager::instance()->mediaCenterInterface()->imageFileName(elem.show, elem.imageType));
            elem.show->setImage(elem.imageType, elem.data);
        }
    } else if (elem.episode && elem.imageType == ImageType::TvShowEpisodeThumb) {
        elem.episode->setThumbnailImage(elem.data);
        scrapeNext();
    }
}

void TvShowMultiScrapeDialog::onDownloadsFinished()
{
    if (!m_executed)
        return;

    scrapeNext();
}


void TvShowMultiScrapeDialog::onChkDvdOrderToggled()
{
    Settings::instance()->setTvShowDvdOrder(ui->chkDvdOrder->isChecked());
}

void TvShowMultiScrapeDialog::onEpisodeLoadDone()
{
    if (!m_executed)
        return;

    TvShowEpisode *episode = static_cast<TvShowEpisode*>(QObject::sender());
    if (!episode)
        return;

    if (m_infosToLoad.contains(TvShowScraperInfos::Thumbnail) && !episode->thumbnail().isEmpty())
        addDownload(ImageType::TvShowEpisodeThumb, episode->thumbnail(), episode);
    else
        scrapeNext();
}
