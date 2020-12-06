#include "MyLabel.h"

#include "globals/Globals.h"

MyLabel::MyLabel(QWidget* parent) : QLabel(parent), m_season{-2}, m_imageSet{false}
{
}

/**
 * \brief Emits the clicked or seasonClicked signal
 */
void MyLabel::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::LeftButton) {
        if (m_season != -2) {
            emit seasonClicked(m_season);
        } else {
            emit clicked();
        }
    }
    QLabel::mousePressEvent(ev);
}

/**
 * \brief Returns if an image was set
 * \return Was an image set
 */
bool MyLabel::imageSet() const
{
    return m_imageSet;
}

/**
 * \brief Returns the season number
 * \return Season number
 */
int MyLabel::season() const
{
    return m_season;
}

/**
 * \brief Sets the season number
 * \param season Number of the season
 */
void MyLabel::setSeason(int season)
{
    m_season = season;
}

/**
 * \brief Sets if an image was set
 */
void MyLabel::setImageSet(bool set)
{
    m_imageSet = set;
}
