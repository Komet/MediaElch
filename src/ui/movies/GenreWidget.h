#pragma once

#include "globals/Globals.h"

#include <QMenu>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QWidget>

namespace Ui {
class GenreWidget;
}

class Movie;

/**
 * @brief The GenreWidget class
 */
class GenreWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GenreWidget(QWidget* parent = nullptr);
    ~GenreWidget() override;

signals:
    void setActionSaveEnabled(bool, MainWidgets);
    void sigJumpToMovie(Movie*);

public slots:
    void onSaveInformation();
    void loadGenres();
    QSplitter* splitter();

private slots:
    void addGenre();
    void deleteGenre();
    void onGenreNameChanged(QTableWidgetItem* item);
    void onGenreSelected();
    void addMovie();
    void removeMovie();
    void showGenresContextMenu(QPoint point);
    void onJumpToMovie(QTableWidgetItem* item);

private:
    Ui::GenreWidget* ui;
    QMenu* m_tableContextMenu;
    QStringList m_addedGenres;

    void clear();
};
