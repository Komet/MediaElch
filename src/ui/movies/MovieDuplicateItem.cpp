#include "MovieDuplicateItem.h"
#include "ui_MovieDuplicateItem.h"

#include "data/movie/Movie.h"
#include "globals/Manager.h"

#include "log/Log.h"

MovieDuplicateItem::MovieDuplicateItem(QWidget* parent) : QWidget(parent), ui(new Ui::MovieDuplicateItem)
{
    ui->setupUi(this);
    ui->iconImdbId->setFont(Manager::instance()->iconFont()->font(16));
    ui->iconTmdbId->setFont(Manager::instance()->iconFont()->font(16));
    ui->iconTitle->setFont(Manager::instance()->iconFont()->font(16));
}

MovieDuplicateItem::~MovieDuplicateItem()
{
    delete ui;
}

void MovieDuplicateItem::setMovie(Movie* movie, bool isOriginal)
{
    ui->labelMovieTitle->setText(movie->title());
    ui->labelFiles->setText(movie->files().toNativeStringList().join("\n"));
    if (isOriginal) {
        ui->widget->setStyleSheet("background-color: #f3f3f3;");
        ui->widget_2->setVisible(false);
    }
}

void MovieDuplicateItem::setDuplicateProperties(MovieDuplicate md)
{
    MyIconFont* font = Manager::instance()->iconFont();

    QColor red(169, 68, 66);
    QColor green(60, 118, 61);

    if (md.imdbId) {
        ui->iconImdbId->setPixmap(font->icon("check", green).pixmap(16, 16));
    } else {
        ui->iconImdbId->setPixmap(font->icon("close", red).pixmap(20, 20));
    }

    if (md.tmdbId) {
        ui->iconTmdbId->setPixmap(font->icon("check", green).pixmap(16, 16));
    } else {
        ui->iconTmdbId->setPixmap(font->icon("close", red).pixmap(20, 20));
    }

    if (md.title) {
        ui->iconTitle->setPixmap(font->icon("check", green).pixmap(16, 16));
    } else {
        ui->iconTitle->setPixmap(font->icon("close", red).pixmap(20, 20));
    }
}
