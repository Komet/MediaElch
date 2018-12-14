#ifndef MYSPLITTER_H
#define MYSPLITTER_H

#include <QSplitter>

class MySplitter : public QSplitter
{
    Q_OBJECT
public:
    explicit MySplitter(QWidget *parent = nullptr);

protected:
    QSplitterHandle *createHandle() override;
};

#endif // MYSPLITTER_H
