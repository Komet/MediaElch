#pragma once

#include <QDialog>
#include <QVector>
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
    explicit LoadingStreamDetails(QWidget* parent = nullptr);
    ~LoadingStreamDetails() override;
    void loadMovies(QVector<Movie*> movies);
    void loadConcerts(QVector<Concert*> concerts);
    void loadTvShowEpisodes(QVector<TvShowEpisode*> episodes);

private:
    Ui::LoadingStreamDetails* ui;
};
