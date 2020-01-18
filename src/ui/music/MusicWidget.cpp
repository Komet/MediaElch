#include "MusicWidget.h"
#include "ui_MusicWidget.h"

#include "globals/Manager.h"
#include "globals/MessageIds.h"
#include "music/Album.h"
#include "music/Artist.h"
#include "ui/notifications/NotificationBox.h"

#include <QDebug>

MusicWidget::MusicWidget(QWidget* parent) : QWidget(parent), ui(new Ui::MusicWidget)
{
    ui->setupUi(this);

    // clang-format off
    connect(ui->artist, &MusicWidgetArtist::sigSetActionSearchEnabled, this, &MusicWidget::sigSetActionSearchEnabled);
    connect(ui->artist, &MusicWidgetArtist::sigSetActionSaveEnabled,   this, &MusicWidget::sigSetActionSaveEnabled);
    connect(ui->artist, &MusicWidgetArtist::sigDownloadsStarted,       this, &MusicWidget::sigDownloadsStarted);
    connect(ui->artist, &MusicWidgetArtist::sigDownloadsProgress,      this, &MusicWidget::sigDownloadsProgress);
    connect(ui->artist, &MusicWidgetArtist::sigDownloadsFinished,      this, &MusicWidget::sigDownloadsFinished);
    connect(ui->album,  &MusicWidgetAlbum::sigSetActionSaveEnabled,    this, &MusicWidget::sigSetActionSaveEnabled);
    connect(ui->album,  &MusicWidgetAlbum::sigSetActionSearchEnabled,  this, &MusicWidget::sigSetActionSearchEnabled);
    connect(ui->album,  &MusicWidgetAlbum::sigDownloadsStarted,        this, &MusicWidget::sigDownloadsStarted);
    connect(ui->album,  &MusicWidgetAlbum::sigDownloadsProgress,       this, &MusicWidget::sigDownloadsProgress);
    connect(ui->album,  &MusicWidgetAlbum::sigDownloadsFinished,       this, &MusicWidget::sigDownloadsFinished);
    // clang-format on
}

MusicWidget::~MusicWidget()
{
    delete ui;
}

void MusicWidget::setBigWindow(bool bigWindow)
{
    Q_UNUSED(bigWindow);
}

void MusicWidget::onArtistSelected(Artist* artist)
{
    ui->artist->setArtist(artist);
    ui->stackedWidget->setCurrentIndex(0);
}

void MusicWidget::onAlbumSelected(Album* album)
{
    ui->album->setAlbum(album);
    ui->stackedWidget->setCurrentIndex(1);
}

void MusicWidget::onSetEnabledTrue(Artist* artist)
{
    Q_UNUSED(artist);

    ui->artist->onSetEnabled(true);
    ui->album->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, MainWidgets::Music);
    emit sigSetActionSearchEnabled(true, MainWidgets::Music);
}

void MusicWidget::onSetEnabledTrue(Album* album)
{
    Q_UNUSED(album);

    ui->artist->onSetEnabled(true);
    ui->album->onSetEnabled(true);
    emit sigSetActionSaveEnabled(true, MainWidgets::Music);
    emit sigSetActionSearchEnabled(true, MainWidgets::Music);
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
    emit sigSetActionSaveEnabled(false, MainWidgets::Music);
    emit sigSetActionSearchEnabled(false, MainWidgets::Music);
}

void MusicWidget::onStartScraperSearch()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        QTimer::singleShot(0, ui->artist, &MusicWidgetArtist::onStartScraperSearch);
    } else if (ui->stackedWidget->currentIndex() == 1) {
        QTimer::singleShot(0, ui->album, &MusicWidgetAlbum::onStartScraperSearch);
    }
}

void MusicWidget::onSaveInformation()
{
    QVector<Artist*> artists = MusicFilesWidget::instance()->selectedArtists();
    QVector<Album*> albums = MusicFilesWidget::instance()->selectedAlbums();
    QVector<Album*> albumsToSave;
    QVector<Artist*> artistsToSave;

    if (artists.count() == 1 && albums.count() == 0 && ui->stackedWidget->currentIndex() == 0) {
        ui->artist->onSaveInformation();
        return;
    }
    if (artists.count() == 0 && albums.count() == 1 && ui->stackedWidget->currentIndex() == 1) {
        ui->album->onSaveInformation();
        return;
    }

    for (Artist* artist : artists) {
        if (artist->hasChanged()) {
            artistsToSave.append(artist);
        }
    }

    for (Album* album : albums) {
        if (album->hasChanged()) {
            albumsToSave.append(album);
        }
    }

    int itemsToSave = artistsToSave.count() + albumsToSave.count();
    int itemsSaved = 0;
    NotificationBox::instance()->showProgressBar(
        tr("Saving changed Artists and Albums"), Constants::MusicWidgetSaveProgressMessageId);
    QApplication::processEvents();

    for (Artist* artist : artistsToSave) {
        artist->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(
            ++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    for (Album* album : albumsToSave) {
        album->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(
            ++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    NotificationBox::instance()->hideProgressBar(Constants::MusicWidgetSaveProgressMessageId);
    NotificationBox::instance()->showSuccess(tr("All Artists and Albums Saved"));
}

void MusicWidget::onSaveAll()
{
    QVector<Album*> albumsToSave;
    QVector<Artist*> artistsToSave;

    for (Artist* artist : Manager::instance()->musicModel()->artists()) {
        if (artist->hasChanged()) {
            artistsToSave.append(artist);
        }
        for (Album* album : artist->albums()) {
            if (album->hasChanged()) {
                albumsToSave.append(album);
            }
        }
    }

    int itemsToSave = artistsToSave.count() + albumsToSave.count();
    int itemsSaved = 0;
    NotificationBox::instance()->showProgressBar(
        tr("Saving changed Artists and Albums"), Constants::MusicWidgetSaveProgressMessageId);
    QApplication::processEvents();

    for (Artist* artist : artistsToSave) {
        artist->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(
            ++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    for (Album* album : albumsToSave) {
        album->controller()->saveData(Manager::instance()->mediaCenterInterface());
        NotificationBox::instance()->progressBarProgress(
            ++itemsSaved, itemsToSave, Constants::MusicWidgetSaveProgressMessageId);
        QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    if (!artistsToSave.isEmpty()) {
        ui->artist->updateArtistInfo();
    }
    if (!albumsToSave.isEmpty()) {
        ui->album->updateAlbumInfo();
    }

    NotificationBox::instance()->hideProgressBar(Constants::MusicWidgetSaveProgressMessageId);
    NotificationBox::instance()->showSuccess(tr("All Artists and Albums Saved"));
}
