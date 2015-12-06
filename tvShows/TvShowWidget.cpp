#include "TvShowWidget.h"
#include "ui_TvShowWidget.h"

#include <QTimer>
#include "globals/Globals.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"

/**
 * @brief TvShowWidget::TvShowWidget
 * @param parent
 */
TvShowWidget::TvShowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TvShowWidget)
{
    ui->setupUi(this);

    connect(ui->tvShowWidget, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
    connect(ui->tvShowWidget, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsStarted(QString,int)), this, SIGNAL(sigDownloadsStarted(QString,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsProgress(int,int,int)), this, SIGNAL(sigDownloadsProgress(int,int,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsFinished(int)), this, SIGNAL(sigDownloadsFinished(int)));

    connect(ui->episodeWidget, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->episodeWidget, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));

    connect(ui->seasonWidget, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->seasonWidget, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
}

/**
 * @brief TvShowWidget::~TvShowWidget
 */
TvShowWidget::~TvShowWidget()
{
    delete ui;
}

void TvShowWidget::setBigWindow(bool bigWindow)
{
    ui->tvShowWidget->setBigWindow(bigWindow);
}

/**
 * @brief Clears the subwidgets
 */
void TvShowWidget::onClear()
{
    ui->episodeWidget->onClear();
    ui->tvShowWidget->onClear();
    ui->seasonWidget->onClear();
}

/**
 * @brief Shows the tv show widget and sets the show
 * @param show Current show object
 */
void TvShowWidget::onTvShowSelected(TvShow *show)
{
    qDebug() << "Entered, show=" << show->name();
    ui->stackedWidget->setCurrentIndex(0);
    ui->tvShowWidget->setTvShow(show);
}

void TvShowWidget::onSeasonSelected(TvShow *show, int season)
{
    qDebug() << "Entered, show=" << show->name() << "season=" << season;
    ui->stackedWidget->setCurrentIndex(2);
    ui->seasonWidget->setSeason(show, season);
}

/**
 * @brief Shows the episode widget and set the episode
 * @param episode Current episode object
 */
void TvShowWidget::onEpisodeSelected(TvShowEpisode *episode)
{
    qDebug() << "Entered, episode=" << episode->name();
    ui->stackedWidget->setCurrentIndex(1);
    ui->episodeWidget->setEpisode(episode);
}

/**
 * @brief Sets the subwidgets enabled if there are no downloads
 */
void TvShowWidget::onSetEnabledTrue(TvShow *show, int season)
{
    if (show && show->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }

    ui->episodeWidget->onSetEnabled(true);
    ui->tvShowWidget->onSetEnabled(true);
    ui->seasonWidget->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetTvShows);
    emit sigSetActionSearchEnabled(season == -1, WidgetTvShows);
}

/**
 * @brief Sets the subwidgets enabled if there are no downloads
 */
void TvShowWidget::onSetEnabledTrue(TvShowEpisode *episode)
{
    if (episode && episode->tvShow() && episode->tvShow()->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }

    ui->episodeWidget->onSetEnabled(true);
    ui->tvShowWidget->onSetEnabled(true);
    ui->seasonWidget->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetTvShows);
    emit sigSetActionSearchEnabled(true, WidgetTvShows);
}

/**
 * @brief Sets the subwidgets disabled
 */
void TvShowWidget::onSetDisabledTrue()
{
    qDebug() << "Entered";
    ui->episodeWidget->onSetEnabled(false);
    ui->tvShowWidget->onSetEnabled(false);
    ui->seasonWidget->onSetEnabled(false);
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
}

/**
 * @brief Delegates the save event to the current subwidget
 */
void TvShowWidget::onSaveInformation()
{
    QList<TvShow*> shows = TvShowFilesWidget::instance()->selectedShows();
    QList<TvShowEpisode*> episodes = TvShowFilesWidget::instance()->selectedEpisodes(false);
    QList<TvShow*> seasons = TvShowFilesWidget::instance()->selectedSeasons();

    if (shows.count() == 1 && episodes.count() == 0 && seasons.count() == 0 && ui->stackedWidget->currentIndex() == 0) {
        ui->tvShowWidget->onSaveInformation();
        TvShowFilesWidget::instance()->updateProxy();
        return;
    } else if (shows.count() == 0 && episodes.count() == 1 && seasons.count() == 0 && ui->stackedWidget->currentIndex() == 1) {
        ui->episodeWidget->onSaveInformation();
        TvShowFilesWidget::instance()->updateProxy();
        return;
    } else if (shows.count() == 0 && episodes.count() == 0 && seasons.count() == 1 && ui->stackedWidget->currentIndex() == 2) {
        ui->seasonWidget->onSaveInformation();
        TvShowFilesWidget::instance()->updateProxy();
        return;
    }

    foreach (TvShow *show, seasons) {
        if (!shows.contains(show))
            shows.append(show);
    }

    int itemsToSave = shows.count() + episodes.count();
    int itemsSaved = 0;
    NotificationBox::instance()->showProgressBar(tr("Saving changed TV Shows and Episodes"), Constants::TvShowWidgetSaveProgressMessageId);
    qApp->processEvents();

    for (int i=0, n=shows.count() ; i<n ; ++i) {
        itemsSaved++;
        if (shows.at(i)->hasChanged()) {
            shows.at(i)->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
            NotificationBox::instance()->progressBarProgress(itemsSaved, itemsToSave, Constants::TvShowWidgetSaveProgressMessageId);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

    for (int i=0, n=episodes.count() ; i<n ; ++i) {
        itemsSaved++;
        if (episodes.at(i)->hasChanged()) {
            episodes.at(i)->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
            NotificationBox::instance()->progressBarProgress(itemsSaved, itemsToSave, Constants::TvShowWidgetSaveProgressMessageId);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
    }

    NotificationBox::instance()->hideProgressBar(Constants::TvShowWidgetSaveProgressMessageId);
    NotificationBox::instance()->showMessage(tr("TV Shows and Episodes Saved"));
    TvShowFilesWidget::instance()->updateProxy();
}

/**
 * @brief Saves all changed tv shows and episodes
 */
void TvShowWidget::onSaveAll()
{
    qDebug() << "Entered";
    QList<TvShow*> shows = Manager::instance()->tvShowModel()->tvShows();
    int episodesToSave = 0;
    int episodesSaved = 0;
    for (int i=0, n=shows.count() ; i<n ; ++i) {
        if (shows[i]->hasChanged())
            episodesToSave++;
        for (int x=0, y=shows[i]->episodes().count() ; x<y ; ++x) {
            if (shows[i]->episodes().at(x)->hasChanged())
                episodesToSave++;
        }
    }
    qDebug() << "episodesToSave=" << episodesToSave;

    NotificationBox::instance()->showProgressBar(tr("Saving changed TV Shows and Episodes"), Constants::TvShowWidgetSaveProgressMessageId);
    qApp->processEvents();

    for (int i=0, n=shows.count() ; i<n ; ++i) {
        if (shows[i]->hasChanged()) {
            qDebug() << "SAVING TV SHOW" << shows[i]->name();
            shows[i]->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
            NotificationBox::instance()->progressBarProgress(++episodesSaved, episodesToSave, Constants::TvShowWidgetSaveProgressMessageId);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        for (int x=0, y=shows[i]->episodes().count() ; x<y ; ++x) {
            if (shows[i]->episodes().at(x)->hasChanged()) {
                shows[i]->episodes().at(x)->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
                NotificationBox::instance()->progressBarProgress(++episodesSaved, episodesToSave, Constants::TvShowWidgetSaveProgressMessageId);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }
    }
    NotificationBox::instance()->hideProgressBar(Constants::TvShowWidgetSaveProgressMessageId);
    NotificationBox::instance()->showMessage(tr("All TV Shows and Episodes Saved"));
}

/**
 * @brief Delegates the search to the current subwidget
 */
void TvShowWidget::onStartScraperSearch()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onStartScraperSearch()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->episodeWidget, SLOT(onStartScraperSearch()));
}

void TvShowWidget::updateInfo()
{
    if (ui->stackedWidget->currentIndex() == 0)
        ui->tvShowWidget->updateTvShowInfo();
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->episodeWidget->updateEpisodeInfo();
    else if (ui->stackedWidget->currentIndex() == 2)
        ui->seasonWidget->updateSeasonInfo();
}
