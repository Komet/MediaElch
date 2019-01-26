#pragma once

#include <QDialog>
#include <QList>
#include <QWidget>

class Concert;
class Movie;
class TvShowEpisode;

namespace Ui {
class LoadingStreamDetails;
}

class LoadingStreamDetails : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingStreamDetails(QWidget *parent = nullptr);
    ~LoadingStreamDetails() override;
    void loadMovies(QList<Movie *> movies);
    void loadConcerts(QList<Concert *> concerts);
    void loadTvShowEpisodes(QList<TvShowEpisode *> episodes);

private:
    Ui::LoadingStreamDetails *ui;
};
