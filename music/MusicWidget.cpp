#include "MusicWidget.h"
#include "ui_MusicWidget.h"

#include <QDebug>

MusicWidget::MusicWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicWidget)
{
    ui->setupUi(this);

    connect(ui->artist, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
    connect(ui->artist, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->album, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSaveEnabled(bool,MainWidgets)));
    connect(ui->album, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)), this, SIGNAL(sigSetActionSearchEnabled(bool,MainWidgets)));
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
    ui->stackedWidget->setCurrentIndex(0);
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
    if (ui->stackedWidget->currentIndex() == 0)
        ui->artist->onSaveInformation();
    else if (ui->stackedWidget->currentIndex() == 1)
        ui->album->onSaveInformation();
}

void MusicWidget::onSaveAll()
{
    // @todo: implement
}
