#pragma once

#include <QLabel>
#include <QPaintEvent>
#include <QWidget>

class SearchOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit SearchOverlay(QWidget* parent = nullptr);
    void setText(QString text);
    void fadeIn();
    void fadeOut();

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    QLabel* m_label;
};
