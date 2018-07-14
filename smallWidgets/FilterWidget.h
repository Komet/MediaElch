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
    explicit FilterWidget(QWidget *parent = nullptr);
    ~FilterWidget() override;
    void setActiveMainWidget(MainWidgets widget);
signals:
    void sigFilterTextChanged(QString);
    void sigFilterChanged(QList<Filter *>, QString);
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
    MainWidgets m_activeWidget;

    // Available filters for current widget
    QList<Filter *> m_availableFilters;

    // Unique available filters
    QList<Filter *> m_availableMovieFilters;
    QList<Filter *> m_availableTvShowFilters;
    QList<Filter *> m_availableConcertFilters;
    QList<Filter *> m_availableMusicFilters;

    QListWidget *m_list;
    QList<Filter *> m_activeFilters;
    QMap<MainWidgets, QList<Filter *>> m_storedFilters;

    void initAvailableFilters();
    QList<Filter *> setupMovieFilters();
    QList<Filter *> setupTvShowFilters();
    QList<Filter *> setupConcertFilters();
    QList<Filter *> setupMusicFilters();
    void storeFilters(MainWidgets widget);
    void loadFilters(MainWidgets widget);
    void setupFilterListUi();
};

#endif // FILTERWIDGET_H
