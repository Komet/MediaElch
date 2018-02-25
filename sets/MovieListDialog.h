#ifndef MOVIELISTDIALOG_H
#define MOVIELISTDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>

#include "movies/Movie.h"

namespace Ui {
class MovieListDialog;
}

/**
 * @brief The MovieListDialog class
 * Displays a list of movies (for the sets widget)
 */
class MovieListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MovieListDialog(QWidget *parent = nullptr);
    ~MovieListDialog();
    QList<Movie*> selectedMovies();
    static MovieListDialog *instance(QWidget *parent = nullptr);

public slots:
    int exec();
    int execWithoutGenre(QString genre);
    int execWithoutCertification(QString certification);

private slots:
    void onAddMovies();
    void onFilterEdited(QString text);

private:
    Ui::MovieListDialog *ui;
    QList<Movie*> m_selectedMovies;
    void reposition();
};

#endif // MOVIELISTDIALOG_H
