#ifndef MYTABLEVIEW_H
#define MYTABLEVIEW_H

#include <QResizeEvent>
#include <QTableView>

class MyTableView : public QTableView
{
    Q_OBJECT
public:
    explicit MyTableView(QWidget *parent = 0);
    
protected:
    void resizeEvent(QResizeEvent *event);
signals:
    void resized(QSize);
};

#endif // MYTABLEVIEW_H
