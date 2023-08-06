#pragma once

#include "data/Filter.h"
#include "globals/Globals.h"

#include <QListWidget>
#include <QWidget>

namespace Ui {
class FilterWidget;
}

/// \brief   A widget for filtering files.
/// \details This is the small input in the upper right corner of MediaElch.
class FilterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FilterWidget(QWidget* parent = nullptr);
    ~FilterWidget() override;
    void setActiveMainWidget(MainWidgets widget);

signals:
    void sigFilterTextChanged(QString filterText);
    void sigFilterChanged(QVector<Filter*> filters, QString filterText);

private slots:
    void onFilterTextChanged(QString text);
    void onKeyDown();
    void onKeyUp();
    void setupFilters();
    void addSelectedFilter();
    void addFilterFromItem(QListWidgetItem* item);
    void removeLastFilter();
    void clearFilters();

private:
    Ui::FilterWidget* ui = nullptr;
    MainWidgets m_activeWidget;

    // Available filters for current widget
    QVector<Filter*> m_availableFilters;

    // Store filters so that they can be deleted in the destructor.
    QVector<Filter*> m_tempFilterStore;

    // Unique available filters
    QVector<Filter*> m_availableMovieFilters;
    QVector<Filter*> m_availableTvShowFilters;
    QVector<Filter*> m_availableConcertFilters;
    QVector<Filter*> m_availableMusicFilters;

    QListWidget* m_list = nullptr;
    QVector<Filter*> m_activeFilters;
    QMap<MainWidgets, QVector<Filter*>> m_storedFilters;

private:
    void clearTempFilterStore();
    void initAvailableFilters();
    ELCH_NODISCARD QVector<Filter*> setupMovieFilters();
    ELCH_NODISCARD QVector<Filter*> setupTvShowFilters();
    ELCH_NODISCARD QVector<Filter*> setupConcertFilters();
    ELCH_NODISCARD QVector<Filter*> setupMusicFilters();
    void storeFilters(MainWidgets widget);
    void loadFilters(MainWidgets widget);
    void setupFilterListUi();
};
