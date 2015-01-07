#include "MusicWidgetAlbum.h"
#include "ui_MusicWidgetAlbum.h"

MusicWidgetAlbum::MusicWidgetAlbum(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicWidgetAlbum)
{
    ui->setupUi(this);
}

MusicWidgetAlbum::~MusicWidgetAlbum()
{
    delete ui;
}

void MusicWidgetAlbum::setAlbum(Album *album)
{
    m_album = album;
    updateAlbumInfo();
    emit sigSetActionSearchEnabled(true, WidgetMusic);
    emit sigSetActionSaveEnabled(true, WidgetMusic);
}

void MusicWidgetAlbum::onSetEnabled(bool enabled)
{

}

void MusicWidgetAlbum::onClear()
{

}

void MusicWidgetAlbum::onSaveInformation()
{

}

void MusicWidgetAlbum::onStartScraperSearch()
{

}

void MusicWidgetAlbum::updateAlbumInfo()
{

}
