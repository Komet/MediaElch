#ifndef MYSPLITTERHANDLE_H
#define MYSPLITTERHANDLE_H

#include <QPaintEvent>
#include <QResizeEvent>
#include <QSplitterHandle>

class MySplitterHandle : public QSplitterHandle
{
    Q_OBJECT
public:
    explicit MySplitterHandle(Qt::Orientation orientation, QSplitter *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event);

private:
    QSize sizeHint() const;
};

#endif // MYSPLITTERHANDLE_H
