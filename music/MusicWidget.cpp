#include "MusicWidget.h"
#include "ui_MusicWidget.h"

#include <QDebug>
#include "../globals/Manager.h"
#include "../notifications/NotificationBox.h"

MusicWidget::MusicWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicWidget)
{
    ui->setupUi(this);

    connect(ui->artist, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
    connect(ui->artist, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->artist, SIGNAL(sigDownloadsStarted(QString,int)), this, SIGNAL(sigDownloadsStarted(QString,int)));
    connect(ui->artist, SIGNAL(sigDownloadsProgress(int,int,int)), this, SIGNAL(sigDownloadsProgress(int,int,int)));
    connect(ui->artist, SIGNAL(sigDownloadsFinished(int)), this, SIGNAL(sigDownloadsFinished(int)));
    connect(ui->album, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->album, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
    connect(ui->album, SIGNAL(sigDownloadsStarted(QString,int)), this, SIGNAL(sigDownloadsStarted(QString,int)));
    connect(ui->album, SIGNAL(sigDownloadsProgress(int,int,int)), this, SIGNAL(sigDownloadsProgress(int,int,int)));
    connect(ui->album, SIGNAL(sigDownloadsFinished(int)), this, SIGNAL(sigDownloadsFinished(int)));
}

MusicWidget::~MusicWidget()
{
    delete ui;
}

void MusicWidget::setBigWindow(bool bigWindow)
{
    Q_UNUSED(bigWindow);
}

void MusicWidget::onArtistSelected(Artist *artist)
{
    ui->artist->setArtist(artist);
    ui->stackedWidget->setCurrentIndex(0);
}

void MusicWidget::onAlbumSelected(Album *album)
{
    ui->album->setAlbum(album);
    ui->stackedWidget->setCurrentIndex(1);
}

void MusicWidget::onSetEnabledTrue(Artist *artist)
{
    Q_UNUSED(artist);

    ui->artist->onSetEnabled(true);
    ui->album->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetMusic);
    emit sigSetActionSearchEnabled(true, WidgetMusic);
}

void MusicWidget::onSetEnabledTrue(Album *album)
{
    Q_UNUSED(album);

    ui->artist->onSetEnabled(true);
    ui->album->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, WidgetMusic);
    emit sigSetActionSearchEnabled(true, WidgetMusic);
}

void MusicWidget::onClear()
{
    ui->artist->onClear();
    ui->album->onClear();
}

void MusicWidget::onSetDisabledTrue()
{
    ui->artist->onSetEnabled(false);
    ui->album->onSetEnabled(false);
    emit sigSetActionSaveEnabled(false, WidgetMusic);
    emit sigSetActionSearchEnabled(false, WidgetMusic);
}

void MusicWidget::onStartScraperSearch()
{
    if (ui->stackedWidget->currentIndex() == 0)
        QTimer::singleShot(0, ui->artist, SLOT(onStartScraperSearch()));
    else if (ui->stackedWidget->currentIndex() == 1)
        QTimer::singleShot(0, ui->album, SLOT(onStartScraperSearch()));
}

void MusicWidget::onSaveInformation()
{
    QList<Artist*> artists = MusicFilesWidget::instance()->selectedArtists();
    QList<Album*> albums = MusicFilesWidget::instance()->selectedAlbums();
    QList<Album*> albumsToSave;
    QList<Artist*> artistsToSave;

    if (artists.count() == 1 && albums.count() == 0 && ui->stackedWidget->currentIndex() == 0) {
        ui->artist->onSaveInformation();
        return;
    } else if (artists.count() == 0 && albums.count() == 1 && ui->stackedWidget->currentIndex() == 1) {
        ui->album->onSaveInformation();
        return;
    }

    foreach (Artist *artist, artists) {
        if (artist->hasChanged())
            artistsToSave.append(artist);
    }

    foreach (Album *album, albums) {
        if (album->hasChanged())
            albumsToSave.append(album);
    }

    int itemsToSave = artistsToSave.count() + albumsToSave.count();
    int itemsSaved = 0;
    NotificationBox::instance()->showProgressBar(tr("Saving changed Artists and Albums"), Constants::MusicWidgetSaveProgressMessageId);
    qApp->processEvents();

    foreach (Artist *artist, artistsToSave) {
        artist->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    foreach (Album *album, albumsToSave) {
        album->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    NotificationBox::instance()->hideProgressBar(Constants::MusicWidgetSaveProgressMessageId);
    NotificationBox::instance()->showMessage(tr("All Artists and Albums Saved"));
}

void MusicWidget::onSaveAll()
{
    QList<Album*> albumsToSave;
    QList<Artist*> artistsToSave;

    foreach (Artist *artist, Manager::instance()->musicModel()->artists()) {
        if (artist->hasChanged())
            artistsToSave.append(artist);
        foreach (Album *album, artist->albums()) {
            if (album->hasChanged())
                albumsToSave.append(album);
        }
    }

    int itemsToSave = artistsToSave.count() + albumsToSave.count();
    int itemsSaved = 0;
    NotificationBox::instance()->showProgressBar(tr("Saving changed Artists and Albums"), Constants::MusicWidgetSaveProgressMessageId);
    qApp->processEvents();

    foreach (Artist *artist, artistsToSave) {
        artist->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    foreach (Album *album, albumsToSave) {
        album->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (!artistsToSave.isEmpty())
        ui->artist->updateArtistInfo();
    if (!albumsToSave.isEmpty())
        ui->album->updateAlbumInfo();

    NotificationBox::instance()->hideProgressBar(Constants::MusicWidgetSaveProgressMessageId);
    NotificationBox::instance()->showMessage(tr("All Artists and Albums Saved"));
}
