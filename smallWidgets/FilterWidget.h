#ifndef FILTERWIDGET_H
#define FILTERWIDGET_H

#include <QKeyEvent>
#include <QListWidget>
#include <QWidget>

#include "globals/Filter.h"
#include "globals/Globals.h"

namespace Ui {
class FilterWidget;
}

/**
 * @brief The FilterWidget class
 * This is the small input in the upper right
 */
class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget *parent = 0);
    ~FilterWidget();
    void setActiveWidget(MainWidgets widget);
signals:
    void sigFilterTextChanged(QString);
    void sigFilterChanged(QList<Filter*>, QString);
private slots:
    void onFilterTextChanged(QString text);
    void onKeyDown();
    void onKeyUp();
    void setupFilters();
    void addSelectedFilter();
    void addFilterFromItem(QListWidgetItem *item);
    void removeLastFilter();
private:
    Ui::FilterWidget *ui;
    QList<Filter*> m_filters;
    QList<Filter*> m_movieFilters;
    QList<Filter*> m_movieGenreFilters;
    QList<Filter*> m_movieStudioFilters;
    QList<Filter*> m_movieCountryFilters;
    QList<Filter*> m_movieYearFilters;
    QList<Filter*> m_movieCertificationFilters;
    QList<Filter*> m_movieDirectorFilters;
    QList<Filter*> m_movieTagsFilters;
    QList<Filter*> m_movieSetsFilters;
    QList<Filter*> m_tvShowFilters;
    QList<Filter*> m_concertFilters;
    QList<Filter*> m_musicFilters;
    QList<Filter*> m_movieLabelFilters;
    QListWidget *m_list;
    QList<Filter*> m_activeFilters;
    MainWidgets m_activeWidget;
    QMap<MainWidgets, QList<Filter*> > m_storedFilters;
    void initFilters();
    void setupMovieFilters();
    void setupTvShowFilters();
    void setupConcertFilters();
    void setupMusicFilters();
    void storeFilters(MainWidgets widget);
    void loadFilters(MainWidgets widget);
};

#endif // FILTERWIDGET_H
