#include "MySplitter.h"

#include "smallWidgets/MySplitterHandle.h"

MySplitter::MySplitter(QWidget *parent) :
    QSplitter(parent)
{
}

QSplitterHandle *MySplitter::createHandle()
{
    return new MySplitterHandle(orientation(), this);
}
