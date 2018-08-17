#ifndef LOADINGSTREAMDETAILS_H
#define LOADINGSTREAMDETAILS_H

#include <QDialog>

#include "data/Concert.h"
#include "data/TvShowEpisode.h"
#include "data/Movie.h"

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

#endif // LOADINGSTREAMDETAILS_H
