#include "Badge.h"

#include <QPainter>

Badge::Badge(QWidget *parent) :
    QLabel(parent),
    m_active{false},
    m_closable{false},
    m_fontBold{true},
    m_showActiveMark{true}
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setBadgeType(Badge::LabelDefault);
}

Badge::Badge(const QString &text, QWidget *parent) :
    QLabel(text, parent),
    m_active{false},
    m_closable{false},
    m_fontBold{true},
    m_showActiveMark{true}
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setBadgeType(Badge::LabelDefault);
}

void Badge::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);

    if (m_closable) {
        QRect rect;
        rect.setX(width() - 20);
        rect.setY(0);
        rect.setWidth(19);
        rect.setHeight(height() - 1);

        QPainter p(this);
        p.save();
        QColor circColor(200, 200, 200, 150);
        int radius = height() - 6;
        p.setPen(circColor);
        p.setBrush(QBrush(circColor));
        p.drawEllipse(width() - radius - 5, 3, radius, radius);

        p.setPen(QColor(255, 255, 255));
        QFont font;
        font.setPixelSize(8);
        p.setFont(font);
        p.drawText(width() - 12, 0, 12, height() - 1, Qt::AlignVCenter, "x");
        p.restore();
    }

    if (m_active && m_showActiveMark) {
        QImage mark = QImage(":img/checkmark_white_64.png").scaledToWidth(12, Qt::SmoothTransformation);
        QPainter p(this);
        p.save();
        p.drawImage(7, (height() - mark.height()) / 2, mark);
        p.restore();
    }
}

void Badge::setFontBold(const bool &bold)
{
    m_fontBold = bold;
    applyStyleSheet();
}

void Badge::setBadgeType(Badge::BadgeType type)
{
    m_badgeType = type;
    applyStyleSheet();
}

void Badge::setActive(const bool &active)
{
    m_active = active;
    switch (m_badgeType) {
    case Badge::LabelDefault:
    case Badge::LabelSuccess:
    case Badge::LabelWarning:
    case Badge::LabelImportant:
    case Badge::LabelInfo:
    case Badge::LabelInverse: m_badgeType = active ? Badge::LabelWarning : Badge::LabelDefault; break;
    case Badge::BadgeDefault:
    case Badge::BadgeSuccess:
    case Badge::BadgeWarning:
    case Badge::BadgeImportant:
    case Badge::BadgeInfo:
    case Badge::BadgeInverse: m_badgeType = active ? Badge::BadgeInfo : Badge::BadgeDefault; break;
    }

    if (active && !property("activeText").toString().isEmpty())
        setText(property("activeText").toString());
    else if (!active && !property("inactiveText").toString().isEmpty())
        setText(property("inactiveText").toString());

    applyStyleSheet();
}

void Badge::setClosable(const bool &closable)
{
    m_closable = closable;
    applyStyleSheet();
}

void Badge::setShowActiveMark(const bool &showActiveMark)
{
    m_showActiveMark = showActiveMark;
    applyStyleSheet();
}

bool Badge::isActive() const
{
    return m_active;
}

bool Badge::isClosable() const
{
    return m_closable;
}

void Badge::applyStyleSheet()
{
    QString style = "QLabel { color: #ffffff; font-size: 10px; ";

    switch (m_badgeType) {
    case Badge::LabelDefault:
    case Badge::BadgeDefault: style.append("background-color: #999999;"); break;
    case Badge::LabelSuccess:
    case Badge::BadgeSuccess: style.append("background-color: #468847;"); break;
    case Badge::LabelWarning:
    case Badge::BadgeWarning: style.append("background-color: #F89406;"); break;
    case Badge::LabelImportant:
    case Badge::BadgeImportant: style.append("background-color: #B94A48;"); break;
    case Badge::LabelInfo:
    case Badge::BadgeInfo: style.append("background-color: #3A87AD;"); break;
    case Badge::LabelInverse:
    case Badge::BadgeInverse: style.append("background-color: #333333;"); break;
    }

    switch (m_badgeType) {
    case Badge::LabelDefault:
    case Badge::LabelSuccess:
    case Badge::LabelWarning:
    case Badge::LabelImportant:
    case Badge::LabelInfo:
    case Badge::LabelInverse:
        style.append("border-radius: 3px;");
        style.append("padding: 4px 0;");
        break;
    case Badge::BadgeDefault:
    case Badge::BadgeSuccess:
    case Badge::BadgeWarning:
    case Badge::BadgeImportant:
    case Badge::BadgeInfo:
    case Badge::BadgeInverse:
        style.append("border-radius: 8px;");
        style.append("padding-left: 5px; padding-right: 5px; padding-top: 2px; padding-bottom: 2px;");
        break;
    }

    if (m_fontBold)
        style.append("font-weight: bold;");
    if (m_active && m_showActiveMark)
        style.append("padding-left: 20px;");
    if (m_closable)
        style.append("padding-right: 20px;");

    style.append("}");
    setStyleSheet(style);
    resize(sizeHint().width(), sizeHint().height());
}

void Badge::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
        emit clicked();
    QLabel::mousePressEvent(ev);
}
