#include "MyLabel.h"

MyLabel::MyLabel(QWidget *parent) :
    QLabel(parent)
{
    m_season = -2;
    m_imageSet = false;
}

void MyLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        if (m_season != -2)
            emit seasonClicked(m_season);
        else
            emit clicked();
    }
    QLabel::mousePressEvent(ev);
}

bool MyLabel::imageSet()
{
    return m_imageSet;
}

int MyLabel::season()
{
    return m_season;
}

void MyLabel::setSeason(int season)
{
    m_season = season;
}

void MyLabel::setImageSet(bool set)
{
    m_imageSet = set;
}
