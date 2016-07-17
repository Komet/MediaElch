#ifndef MOVIEDUPLICATEITEM_H
#define MOVIEDUPLICATEITEM_H

#include <QWidget>
#include "../globals/Globals.h"
#include "Movie.h"

namespace Ui {
class MovieDuplicateItem;
}

class MovieDuplicateItem : public QWidget
{
    Q_OBJECT

public:
    explicit MovieDuplicateItem(QWidget *parent = 0);
    ~MovieDuplicateItem();
    void setDuplicateProperties(MovieDuplicate md);
    void setMovie(Movie *movie, bool isOriginal);

private:
    Ui::MovieDuplicateItem *ui;
};

#endif // MOVIEDUPLICATEITEM_H
