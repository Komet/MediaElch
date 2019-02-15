#include "MySpinBox.h"

MySpinBox::MySpinBox(QWidget* parent) : QSpinBox(parent)
{
}

QString MySpinBox::textFromValue(int val) const
{
    return locale().toString(val);
}
