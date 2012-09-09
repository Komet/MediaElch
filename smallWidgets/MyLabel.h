#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QMouseEvent>

/**
 * @brief The MyLabel class
 */
class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget *parent = 0);
    void setSeason(int season);
    void setImageSet(bool set);
    bool imageSet();
    int season();
protected:
    void mousePressEvent(QMouseEvent *ev);
signals:
    void clicked();
    void seasonClicked(int);
private:
    int m_season;
    bool m_imageSet;
};

#endif // MYLABEL_H
