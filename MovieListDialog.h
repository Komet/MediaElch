#ifndef MOVIELISTDIALOG_H
#define MOVIELISTDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include "data/Movie.h"

namespace Ui {
class MovieListDialog;
}

class MovieListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MovieListDialog(QWidget *parent = 0);
    ~MovieListDialog();
    Movie *selectedMovie();
    static MovieListDialog *instance(QWidget *parent = 0);

public slots:
    int exec();

private slots:
    void onMovieSelected(QTableWidgetItem *item);

private:
    Ui::MovieListDialog *ui;
    Movie *m_selectedMovie;
};

#endif // MOVIELISTDIALOG_H
