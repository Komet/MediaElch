#include "MyCheckBox.h"

MyCheckBox::MyCheckBox(QWidget* parent) : QCheckBox(parent)
{
}

void MyCheckBox::setMyData(const QVariant& myData)
{
    m_myData = myData;
}

QVariant MyCheckBox::myData() const
{
    return m_myData;
}
