#ifndef MOVIEDUPLICATEITEM_H
#define MOVIEDUPLICATEITEM_H

#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class MovieDuplicateItem;
}

class Movie;

class MovieDuplicateItem : public QWidget
{
    Q_OBJECT

public:
    explicit MovieDuplicateItem(QWidget *parent = nullptr);
    ~MovieDuplicateItem() override;
    void setDuplicateProperties(MovieDuplicate md);
    void setMovie(Movie *movie, bool isOriginal);

private:
    Ui::MovieDuplicateItem *ui;
};

#endif // MOVIEDUPLICATEITEM_H
