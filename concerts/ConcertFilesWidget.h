#ifndef CONCERTFILESWIDGET_H
#define CONCERTFILESWIDGET_H

#include <QLabel>
#include <QWidget>
#include "data/Concert.h"
#include "data/ConcertModel.h"
#include "data/ConcertProxyModel.h"
#include "data/ConcertDelegate.h"

namespace Ui {
class ConcertFilesWidget;
}

/**
 * @brief The ConcertFilesWidget class
 * This widget displays a list of concert
 * It's a singleton and gets constructed through the gui,
 * the instance can be retrieved through ConcertFilesWidget::instance
 */
class ConcertFilesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConcertFilesWidget(QWidget *parent = 0);
    ~ConcertFilesWidget();
    static ConcertFilesWidget *instance();

public slots:
    void restoreLastSelection();
    void setFilter(QString filter);

signals:
    void noConcertSelected();
    void concertSelected(Concert*);

private slots:
    void itemActivated(QModelIndex index, QModelIndex previous);
    void concertSelectedEmitter();

private:
    Ui::ConcertFilesWidget *ui;
    ConcertProxyModel *m_concertProxyModel;
    ConcertDelegate *m_concertDelegate;
    Concert *m_lastConcert;
    QModelIndex m_lastModelIndex;
    static ConcertFilesWidget *m_instance;
};

#endif // CONCERTFILESWIDGET_H
