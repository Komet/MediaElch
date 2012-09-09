#include "TvShowWidget.h"
#include "ui_TvShowWidget.h"

#include <QTimer>
#include "Globals.h"
#include "Manager.h"
#include "MessageBox.h"

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
    connect(ui->episodeWidget, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->episodeWidget, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsStarted(QString,int)), this, SIGNAL(sigDownloadsStarted(QString,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsProgress(int,int,int)), this, SIGNAL(sigDownloadsProgress(int,int,int)));
    connect(ui->tvShowWidget, SIGNAL(sigDownloadsFinished(int)), this, SIGNAL(sigDownloadsFinished(int)));
}

/**
 * @brief TvShowWidget::~TvShowWidget
 */
TvShowWidget::~TvShowWidget()
{
    delete ui;
}

/**
 * @brief Clears the subwidgets
 */
void TvShowWidget::onClear()
{
    ui->episodeWidget->onClear();
    ui->tvShowWidget->onClear();
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
void TvShowWidget::onSetEnabledTrue(TvShow *show)
{
    qDebug() << "Entered";
    if (show)
        qDebug() << "show=" << show->name();
    else
        qDebug() << "Got no show";
    if (show && show->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }

    ui->episodeWidget->onSetEnabled(true);
    ui->tvShowWidget->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetTvShows);
    emit sigSetActionSearchEnabled(true, WidgetTvShows);
}

/**
 * @brief Sets the subwidgets enabled if there are no downloads
 */
void TvShowWidget::onSetEnabledTrue(TvShowEpisode *episode)
{
    qDebug() << "Entered";
    if (episode)
         qDebug() << "episode=" << episode->name();
    else
        qDebug() << "Got no episode";
    if (episode && episode->tvShow() && episode->tvShow()->downloadsInProgress()) {
        qDebug() << "Downloads are in progress";
        return;
    }

    ui->episodeWidget->onSetEnabled(true);
    ui->tvShowWidget->onSetEnabled(true);
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
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
}

/**
 * @brief Delegates the save event to the current subwidget
 */
void TvShowWidget::onSaveInformation()
{
    qDebug() << "Entered, currentIndex=" << ui->stackedWidget->currentIndex();
    if (ui->stackedWidget->currentIndex() == 0)
        ui->tvShowWidget->onSaveInformation();
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->episodeWidget->onSaveInformation();
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

    MessageBox::instance()->showProgressBar(tr("Saving changed TV Shows and Episodes"), Constants::TvShowWidgetSaveProgressMessageId);
    qApp->processEvents();

    for (int i=0, n=shows.count() ; i<n ; ++i) {
        if (shows[i]->hasChanged()) {
            shows[i]->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
            MessageBox::instance()->progressBarProgress(++episodesSaved, episodesToSave, Constants::TvShowWidgetSaveProgressMessageId);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        for (int x=0, y=shows[i]->episodes().count() ; x<y ; ++x) {
            if (shows[i]->episodes().at(x)->hasChanged()) {
                shows[i]->episodes().at(x)->saveData(Manager::instance()->mediaCenterInterfaceTvShow());
                MessageBox::instance()->progressBarProgress(++episodesSaved, episodesToSave, Constants::TvShowWidgetSaveProgressMessageId);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }
    }
    MessageBox::instance()->hideProgressBar(Constants::TvShowWidgetSaveProgressMessageId);
    MessageBox::instance()->showMessage(tr("All TV Shows and Episodes Saved"));
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
