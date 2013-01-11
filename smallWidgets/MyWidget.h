#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QResizeEvent>
#include <QWidget>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = 0);

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // MYWIDGET_H
