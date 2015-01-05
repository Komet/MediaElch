#include "MusicWidget.h"
#include "ui_MusicWidget.h"

MusicWidget::MusicWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicWidget)
{
    ui->setupUi(this);
}

MusicWidget::~MusicWidget()
{
    delete ui;
}

void MusicWidget::setBigWindow(bool bigWindow)
{

}

void MusicWidget::onArtistSelected(Artist *artist)
{

}

void MusicWidget::onAlbumSelected(Album *album)
{

}

void MusicWidget::onSetEnabledTrue(Artist *artist)
{

}

void MusicWidget::onSetEnabledTrue(Album *album)
{

}

void MusicWidget::onClear()
{

}

void MusicWidget::onSetDisabledTrue()
{

}

void MusicWidget::onStartScraperSearch()
{

}

void MusicWidget::onSaveInformation()
{

}

void MusicWidget::onSaveAll()
{

}
