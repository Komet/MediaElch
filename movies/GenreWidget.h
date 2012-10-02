#ifndef GENREWIDGET_H
#define GENREWIDGET_H

#include <QContextMenuEvent>
#include <QMenu>
#include <QSplitter>
#include <QTableWidgetItem>
#include <QWidget>
#include "globals/Globals.h"

namespace Ui {
class GenreWidget;
}

/**
 * @brief The GenreWidget class
 */
class GenreWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GenreWidget(QWidget *parent = 0);
    ~GenreWidget();

signals:
    void setActionSaveEnabled(bool, MainWidgets);

public slots:
    void onSaveInformation();
    void loadGenres();
    QSplitter *splitter();

protected:
    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void deleteGenre();
    void onGenreNameChanged(QTableWidgetItem *item);
    void onGenreSelected();
    void addMovie();
    void removeMovie();

private:
    Ui::GenreWidget *ui;
    QMenu *m_tableContextMenu;

    void clear();
};

#endif // GENREWIDGET_H
