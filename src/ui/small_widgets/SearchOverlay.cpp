#include "SearchOverlay.h"

#include <QPainter>
#include <QStyleOption>

SearchOverlay::SearchOverlay(QWidget* parent) : QWidget(parent)
{
    QSize size(100, 100);

    QString style =
        "QWidget { background-color: rgba(0, 0, 0, 80); border: 1px solid rgba(0, 0, 0, 20); border-radius: 7px; }";
    setStyleSheet(style);
    setFixedSize(size);

    m_label = new QLabel(this);
    m_label->setFixedSize(size);
    m_label->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
    m_label->setStyleSheet("color: #ffffff; font-weight: bold; font-size: 16px;");

    hide();
}

void SearchOverlay::resizeEvent(QResizeEvent* event)
{
    m_label->setFixedSize(event->size());
}

void SearchOverlay::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SearchOverlay::setText(QString text)
{
    m_label->setText(text);
}

void SearchOverlay::fadeIn()
{
    QWidget::show();
}

void SearchOverlay::fadeOut()
{
    QWidget::hide();
}
