#pragma once

#include "globals/Filter.h"
#include "globals/Globals.h"

#include <QKeyEvent>
#include <QListWidget>
#include <QWidget>

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
    explicit FilterWidget(QWidget* parent = nullptr);
    ~FilterWidget() override;
    void setActiveMainWidget(MainWidgets widget);
signals:
    void sigFilterTextChanged(QString);
    void sigFilterChanged(QVector<Filter*>, QString);
private slots:
    void onFilterTextChanged(QString text);
    void onKeyDown();
    void onKeyUp();
    void setupFilters();
    void addSelectedFilter();
    void addFilterFromItem(QListWidgetItem* item);
    void removeLastFilter();

private:
    Ui::FilterWidget* ui = nullptr;
    MainWidgets m_activeWidget;

    // Available filters for current widget
    QVector<Filter*> m_availableFilters;

    // Unique available filters
    QVector<Filter*> m_availableMovieFilters;
    QVector<Filter*> m_availableTvShowFilters;
    QVector<Filter*> m_availableConcertFilters;
    QVector<Filter*> m_availableMusicFilters;

    QListWidget* m_list = nullptr;
    QVector<Filter*> m_activeFilters;
    QMap<MainWidgets, QVector<Filter*>> m_storedFilters;

    void initAvailableFilters();
    QVector<Filter*> setupMovieFilters();
    QVector<Filter*> setupTvShowFilters();
    QVector<Filter*> setupConcertFilters();
    QVector<Filter*> setupMusicFilters();
    void storeFilters(MainWidgets widget);
    void loadFilters(MainWidgets widget);
    void setupFilterListUi();
};
