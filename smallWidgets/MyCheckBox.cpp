#include "MyCheckBox.h"

MyCheckBox::MyCheckBox(QWidget *parent) :
    QCheckBox(parent)
{
}

void MyCheckBox::setMyData(const QVariant &data)
{
    m_myData = data;
}

QVariant MyCheckBox::myData() const
{
    return m_myData;
}
