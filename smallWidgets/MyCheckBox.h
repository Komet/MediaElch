#ifndef MYCHECKBOX_H
#define MYCHECKBOX_H

#include <QCheckBox>
#include <QVariant>

class MyCheckBox : public QCheckBox
{
    Q_OBJECT
public:
    explicit MyCheckBox(QWidget *parent = 0);
    void setMyData(const QVariant &data);
    QVariant myData() const;

private:
    QVariant m_myData;
};

#endif // MYCHECKBOX_H
