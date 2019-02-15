#pragma once

#include <QImage>
#include <QMap>
#include <QMovie>
#include <QSplitter>
#include <QStringList>
#include <QTableWidgetItem>
#include <QWidget>

#include "globals/DownloadManagerElement.h"

class DownloadManager;
class Movie;

namespace Ui {
class SetsWidget;
}

/**
 * @brief The SetsWidget class
 */
class SetsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SetsWidget(QWidget* parent = nullptr);
    ~SetsWidget() override;

public slots:
    void loadSets();
    void saveSet();
    QSplitter* splitter();

signals:
    void setActionSaveEnabled(bool, MainWidgets);
    void sigJumpToMovie(Movie* movie);

private slots:
    void onSetSelected();
    void clear();
    void onSortTitleChanged(QTableWidgetItem* item);
    void onAddMovieSet();
    void onRemoveMovieSet();
    void onAddMovie();
    void onRemoveMovie();
    void chooseSetPoster();
    void chooseSetBackdrop();
    void onPreviewPoster();
    void onPreviewBackdrop();
    void showSetsContextMenu(QPoint point);
    void onSetNameChanged(QTableWidgetItem* item);
    void onDownloadFinished(DownloadManagerElement elem);
    void onJumpToMovie(QTableWidgetItem* item);

private:
    Ui::SetsWidget* ui;
    QMap<QString, QVector<Movie*>> m_sets;
    QMap<QString, QVector<Movie*>> m_moviesToSave;
    QMap<QString, QImage> m_setPosters;
    QMap<QString, QImage> m_setBackdrops;
    QImage m_currentPoster;
    QImage m_currentBackdrop;
    QStringList m_addedSets;
    QMenu* m_tableContextMenu;
    DownloadManager* m_downloadManager;
    QMovie* m_loadingMovie;

    void loadSet(QString set);
};
