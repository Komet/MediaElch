#include "TvShowWidget.h"
#include "ui_TvShowWidget.h"

#include <QTimer>
#include "Manager.h"
#include "MessageBox.h"

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

TvShowWidget::~TvShowWidget()
{
    delete ui;
}

void TvShowWidget::onClear()
{
    ui->episodeWidget->onClear();
    ui->tvShowWidget->onClear();
}

void TvShowWidget::onTvShowSelected(TvShow *show)
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->tvShowWidget->setTvShow(show);
}

void TvShowWidget::onEpisodeSelected(TvShowEpisode *episode)
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->episodeWidget->setEpisode(episode);
}

void TvShowWidget::onSetEnabledTrue(TvShow *show)
{
    if (show && show->downloadsInProgress())
        return;

    ui->episodeWidget->onSetEnabled(true);
    ui->tvShowWidget->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetTvShows);
    emit sigSetActionSearchEnabled(true, WidgetTvShows);
}

void TvShowWidget::onSetEnabledTrue(TvShowEpisode *episode)
{
    if (episode && episode->tvShow() && episode->tvShow()->downloadsInProgress())
        return;

    ui->episodeWidget->onSetEnabled(true);
    ui->tvShowWidget->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetTvShows);
    emit sigSetActionSearchEnabled(true, WidgetTvShows);
}

void TvShowWidget::onSetDisabledTrue()
{
    ui->episodeWidget->onSetEnabled(false);
    ui->tvShowWidget->onSetEnabled(false);
    emit sigSetActionSaveEnabled(false, WidgetTvShows);
    emit sigSetActionSearchEnabled(false, WidgetTvShows);
}

void TvShowWidget::onSaveInformation()
{
    if (ui->stackedWidget->currentIndex() == 0)
        ui->tvShowWidget->onSaveInformation();
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->episodeWidget->onSaveInformation();
}

void TvShowWidget::onSaveAll()
{
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

    MessageBox::instance()->showProgressBar(tr("Saving changed TV Shows and Episodes"), Constants::TvShowWidgetSaveProgressMessageId);
    qApp->processEvents();

    for (int i=0, n=shows.count() ; i<n ; ++i) {
        if (shows[i]->hasChanged()) {
            shows[i]->saveData(Manager::instance()->mediaCenterInterface());
            MessageBox::instance()->progressBarProgress(++episodesSaved, episodesToSave, Constants::TvShowWidgetSaveProgressMessageId);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        }
        for (int x=0, y=shows[i]->episodes().count() ; x<y ; ++x) {
            if (shows[i]->episodes().at(x)->hasChanged()) {
                shows[i]->episodes().at(x)->saveData(Manager::instance()->mediaCenterInterface());
                MessageBox::instance()->progressBarProgress(++episodesSaved, episodesToSave, Constants::TvShowWidgetSaveProgressMessageId);
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }
    }
    MessageBox::instance()->hideProgressBar(Constants::TvShowWidgetSaveProgressMessageId);
    MessageBox::instance()->showMessage(tr("All TV Shows and Episodes Saved"));
}

void TvShowWidget::onStartScraperSearch()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->tvShowWidget, SLOT(onStartScraperSearch()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->episodeWidget, SLOT(onStartScraperSearch()));
}
