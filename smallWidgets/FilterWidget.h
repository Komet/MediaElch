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
    void setupFilters(bool forceClear = false);
    void addSelectedFilter();
    void addFilterFromItem(QListWidgetItem *item);
    void removeLastFilter();
private:
    Ui::FilterWidget *ui;
    QList<Filter*> m_filters;
    QListWidget *m_list;
    QList<Filter*> m_activeFilters;
    MainWidgets m_activeWidget;
    void setupMovieFilters();
    void setupTvShowFilters();
    void setupConcertFilters();
};

#endif // FILTERWIDGET_H
