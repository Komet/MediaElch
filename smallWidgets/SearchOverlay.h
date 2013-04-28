#ifndef SEARCHOVERLAY_H
#define SEARCHOVERLAY_H

#include <QLabel>
#include <QPaintEvent>
#include <QWidget>

class SearchOverlay : public QWidget
{
    Q_OBJECT
public:
    explicit SearchOverlay(QWidget *parent = 0);
    void setText(QString text);
    void fadeIn();
    void fadeOut();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QLabel *m_label;
};

#endif // SEARCHOVERLAY_H
