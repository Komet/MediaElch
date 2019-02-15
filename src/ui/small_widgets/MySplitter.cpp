#include "MySplitter.h"

#include "ui/small_widgets/MySplitterHandle.h"

MySplitter::MySplitter(QWidget* parent) : QSplitter(parent)
{
}

QSplitterHandle* MySplitter::createHandle()
{
    return new MySplitterHandle(orientation(), this);
}
