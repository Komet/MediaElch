#ifndef MYGROUPBOX_H
#define MYGROUPBOX_H

#include <QGroupBox>
#include <QResizeEvent>

class MyGroupBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit MyGroupBox(QWidget *parent = 0);
    
protected:
    void resizeEvent(QResizeEvent *event);
signals:
    void resized(QSize);
};

#endif // MYGROUPBOX_H
