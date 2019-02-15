#pragma once

#include <QResizeEvent>
#include <QWidget>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyWidget(QWidget* parent = nullptr);

signals:
    void resized();

protected:
    void resizeEvent(QResizeEvent* event) override;
};
