#ifndef MOVIEDUPLICATES_H
#define MOVIEDUPLICATES_H

#include "../data/MovieProxyModel.h"
#include "data/Movie.h"
#include <QWidget>

namespace Ui {
class MovieDuplicates;
}

class MovieDuplicates : public QWidget
{
    Q_OBJECT

public:
    explicit MovieDuplicates(QWidget *parent = nullptr);
    ~MovieDuplicates() override;

private slots:
    void detectDuplicates();
    void onItemActivated(QModelIndex index, QModelIndex previous);

private:
    Ui::MovieDuplicates *ui;
    MovieProxyModel *m_movieProxyModel;
    QMap<Movie *, QList<Movie *>> m_duplicateMovies;
};

#endif // MOVIEDUPLICATES_H
