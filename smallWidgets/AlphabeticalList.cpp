#include "AlphabeticalList.h"

#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOption>
#include <QToolButton>

AlphabeticalList::AlphabeticalList(QWidget *parent) :
    QWidget(parent)
{
    m_animDuration = 100;
    m_topSpace = 10;
    m_bottomSpace = 10;
    m_rightSpace = 10;
    m_layout = new QVBoxLayout(this);
    m_layout->setMargin(0);
    setContentsMargins(0, 0, 0, 0);
    QString style = "QWidget { background-color: rgba(0, 0, 0, 80); border: 1px solid rgba(0, 0, 0, 20); border-radius: 7px; }";
    style.append("QToolButton { background-color: transparent; border: none; color: rgb(255, 255, 255); font-weight: bold; } ");
    setStyleSheet(style);
    setFixedWidth(20);
}

void AlphabeticalList::adjustSize()
{
    int parentWidth = static_cast<QWidget*>(parent())->size().width();
    int parentHeight = static_cast<QWidget*>(parent())->size().height();
    move(parentWidth, m_topSpace);
    setFixedHeight(parentHeight-m_topSpace-m_bottomSpace);
}

void AlphabeticalList::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void AlphabeticalList::show()
{
    if (outAnim)
        outAnim->stop();

    int parentWidth = static_cast<QWidget*>(parent())->size().width();
    int duration = m_animDuration;
    if (size().width()+m_rightSpace != 0)
        duration *= (1-(qreal)(parentWidth-pos().x())/(size().width()+m_rightSpace));

    inAnim = new QPropertyAnimation(this, "pos");
    inAnim->setDuration(duration);
    inAnim->setStartValue(QPoint(pos().x(), m_topSpace));
    inAnim->setEndValue(QPoint(parentWidth-size().width()-m_rightSpace, m_topSpace));
    inAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AlphabeticalList::hide()
{
    if (inAnim)
        inAnim->stop();

    int parentWidth = static_cast<QWidget*>(parent())->size().width();
    int duration = m_animDuration;
    if (size().width()+m_rightSpace != 0)
        duration *= (qreal)(parentWidth-pos().x())/(size().width()+m_rightSpace);

    outAnim = new QPropertyAnimation(this, "pos");
    outAnim->setDuration(duration);
    outAnim->setStartValue(QPoint(pos().x(), m_topSpace));
    outAnim->setEndValue(QPoint(parentWidth, m_topSpace));
    outAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void AlphabeticalList::setTopSpace(const int space)
{
    m_topSpace = space;
}

void AlphabeticalList::setBottomSpace(const int space)
{
    m_bottomSpace = space;
}

void AlphabeticalList::setRightSpace(const int space)
{
    m_rightSpace = space;
}

void AlphabeticalList::setAlphas(QStringList alphas)
{
    foreach (QToolButton *button, findChildren<QToolButton*>()) {
        m_layout->removeWidget(button);
        button->deleteLater();
    }

    foreach (const QString &alpha, alphas) {
        QToolButton *button = new QToolButton(this);
        button->setText(alpha);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
        m_layout->addWidget(button);
        connect(button, SIGNAL(clicked()), this, SLOT(onAlphaClicked()));
    }
}

void AlphabeticalList::onAlphaClicked()
{
    QToolButton *button = static_cast<QToolButton*>(sender());
    if (!button)
        return;
    emit sigAlphaClicked(button->text());
}
