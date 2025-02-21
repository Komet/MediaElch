#include "LoadingStreamDetails.h"
#include "ui_LoadingStreamDetails.h"

#include "data/concert/Concert.h"
#include "data/movie/Movie.h"
#include "data/tv_show/TvShowEpisode.h"

LoadingStreamDetails::LoadingStreamDetails(QWidget* parent) : QDialog(parent), ui(new Ui::LoadingStreamDetails)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QFont font = ui->currentFile->font();
#ifdef Q_OS_WIN
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->currentFile->setFont(font);
}

LoadingStreamDetails::~LoadingStreamDetails()
{
    delete ui;
}

void LoadingStreamDetails::loadMovies(QVector<Movie*> movies)
{
    ui->progressBar->setRange(0, qsizetype_to_int(movies.count()));
    ui->progressBar->setValue(0);
    ui->currentFile->clear();
    adjustSize();
    show();
    for (Movie* movie : movies) {
        movie->blockSignals(true);
        const bool success = movie->controller()->loadStreamDetailsFromFile();
        if (success) {
            movie->setChanged(true);
        }
        movie->blockSignals(false);
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        ui->currentFile->setText(movie->title());
        QApplication::processEvents();
    }
    accept();
}

void LoadingStreamDetails::loadConcerts(QVector<Concert*> concerts)
{
    ui->progressBar->setRange(0, qsizetype_to_int(concerts.count()));
    ui->progressBar->setValue(0);
    ui->currentFile->clear();
    adjustSize();
    show();
    for (Concert* concert : concerts) {
        const bool success = concert->controller()->loadStreamDetailsFromFile();
        if (success) {
            concert->setChanged(true);
        }
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        ui->currentFile->setText(concert->title());
        QApplication::processEvents();
    }
    accept();
}

void LoadingStreamDetails::loadTvShowEpisodes(QVector<TvShowEpisode*> episodes)
{
    ui->progressBar->setRange(0, qsizetype_to_int(episodes.count()));
    ui->progressBar->setValue(0);
    ui->currentFile->clear();
    adjustSize();
    show();
    for (TvShowEpisode* episode : episodes) {
        const bool success = episode->loadStreamDetailsFromFile();
        if (success) {
            episode->setChanged(true);
        }
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        ui->currentFile->setText(episode->title());
        QApplication::processEvents();
    }
    accept();
}
