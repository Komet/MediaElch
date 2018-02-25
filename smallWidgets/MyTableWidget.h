#ifndef MYTABLEWIDGET_H
#define MYTABLEWIDGET_H

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QTableWidget>
#include <QUrl>

/**
 * @brief The MyTableWidget class can handle drap and drop events
 */
class MyTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit MyTableWidget(QWidget *parent = nullptr);
protected:
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
signals:
    void sigDroppedImage(QUrl);
};

#endif // MYTABLEWIDGET_H
