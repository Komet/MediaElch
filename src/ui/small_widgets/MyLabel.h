#pragma once

#include <QLabel>
#include <QMouseEvent>

/**
 * \brief The MyLabel class
 */
class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(QWidget* parent = nullptr);
    void setSeason(int season);
    void setImageSet(bool set);
    bool imageSet() const;
    int season() const;

protected:
    void mousePressEvent(QMouseEvent* ev) override;
signals:
    void clicked();
    void seasonClicked(int);

private:
    int m_season;
    bool m_imageSet;
};
