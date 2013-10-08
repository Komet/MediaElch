#ifndef MYSPLITTER_H
#define MYSPLITTER_H

#include <QSplitter>

class MySplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit MySplitter(QWidget *parent = 0);

protected:
     QSplitterHandle *createHandle();
};

#endif // MYSPLITTER_H
