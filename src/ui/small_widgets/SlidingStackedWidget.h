#pragma once

#include <QDebug>
#include <QEasingCurve>
#include <QStackedWidget>
#include <QWidget>
#include <QtGui>

/**
 * \brief SlidingStackedWidget is a class that is derived from QtStackedWidget
 *        and allows smooth side shifting of widgets, in addition
 *        to the original hard switching from one to another as offered by
 *        QStackedWidget itself.
 * Thanks to http://www.developer.nokia.com/Community/Wiki/Extending_QStackedWidget_for_sliding_page_animations_in_Qt
 */
class SlidingStackedWidget : public QStackedWidget
{
    Q_OBJECT

public:
    enum class direction
    {
        LEFT2RIGHT,
        RIGHT2LEFT,
        TOP2BOTTOM,
        BOTTOM2TOP,
        AUTOMATIC
    };

    explicit SlidingStackedWidget(QWidget* parent);
    ~SlidingStackedWidget() override = default;
    bool isExpanded() const;

public slots:
    void setSpeed(int speed);
    void setAnimation(enum QEasingCurve::Type animationType);
    void setVerticalMode(bool vertical = true);
    void setWrap(bool wrap);
    void slideInNext();
    void slideInPrev();
    void slideInIdx(int idx, SlidingStackedWidget::direction _direction = direction::AUTOMATIC);
    void expandToOne();
    void collapse();

signals:
    void animationFinished();

protected slots:
    void animationDoneSlot();

protected:
    void slideInWgt(QWidget* widget, SlidingStackedWidget::direction _direction = direction::AUTOMATIC);
    QWidget* m_mainWindow;
    int m_speed;
    enum QEasingCurve::Type m_animationType;
    bool m_vertical;
    int m_now;
    int m_next;
    bool m_wrap;
    QPoint m_pNow;
    bool m_active;
    QVector<QWidget*> m_widgets;
    bool m_expanded;
};
