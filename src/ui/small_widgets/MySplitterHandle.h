#pragma once

#include <QPaintEvent>
#include <QResizeEvent>
#include <QSplitterHandle>

class MySplitterHandle : public QSplitterHandle
{
    Q_OBJECT
public:
    explicit MySplitterHandle(Qt::Orientation orientation, QSplitter* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QSize sizeHint() const override;
};
