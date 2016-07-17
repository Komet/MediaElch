#ifndef MOVIEDUPLICATES_H
#define MOVIEDUPLICATES_H

#include <QWidget>
#include "Movie.h"
#include "../data/MovieProxyModel.h"

namespace Ui {
class MovieDuplicates;
}

class MovieDuplicates : public QWidget
{
    Q_OBJECT

public:
    explicit MovieDuplicates(QWidget *parent = 0);
    ~MovieDuplicates();

private slots:
    void detectDuplicates();
    void onItemActivated(QModelIndex index, QModelIndex previous);

private:
    Ui::MovieDuplicates *ui;
    MovieProxyModel *m_movieProxyModel;
    QMap<Movie*, QList<Movie*> > m_duplicateMovies;
};

#endif // MOVIEDUPLICATES_H
