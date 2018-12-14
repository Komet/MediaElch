#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QResizeEvent>
#include <QWidget>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget *parent = nullptr);

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // MYWIDGET_H
