#ifndef MYSPINBOX_H
#define MYSPINBOX_H

#include <QSpinBox>

class MySpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit MySpinBox(QWidget *parent = nullptr);

protected:
    QString textFromValue(int val) const override;
};

#endif // MYSPINBOX_H
