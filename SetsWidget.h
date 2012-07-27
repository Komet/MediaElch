#ifndef SETSWIDGET_H
#define SETSWIDGET_H

#include <QSplitter>
#include <QTableWidgetItem>
#include <QWidget>
#include "data/Movie.h"

namespace Ui {
class SetsWidget;
}

class SetsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SetsWidget(QWidget *parent = 0);
    ~SetsWidget();

public slots:
    void loadSets();
    void saveSet();
    QSplitter *splitter();

signals:
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void onSetSelected();
    void clear();
    void onSortTitleChanged(QTableWidgetItem *item);
    void onAddMovie();
    void onRemoveMovie();
    void chooseSetPoster();
    void chooseSetBackdrop();
    void onPreviewPoster();
    void onPreviewBackdrop();

private:
    Ui::SetsWidget *ui;
    QMap<QString, QList<Movie*> > m_sets;
    QMap<QString, QList<Movie*> > m_moviesToSave;
    QMap<QString, QImage> m_setPosters;
    QMap<QString, QImage> m_setBackdrops;
    QImage m_currentPoster;
    QImage m_currentBackdrop;

    void loadSet(QString set);
};

#endif // SETSWIDGET_H
