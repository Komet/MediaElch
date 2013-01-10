#include "Badge.h"

Badge::Badge(QWidget *parent) :
    QLabel(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    setBadgeType(Badge::LabelDefault);
}

void Badge::setBadgeType(Badge::BadgeType type)
{
    m_badgeType = type;
    QString style = "QLabel { color: #ffffff; border-radius: 3px; font-weight: bold; padding: 4px 0; font-size: 10px; ";

    switch (type)
    {
    case Badge::LabelDefault:
        style.append("background-color: #999999;");
        break;
    case Badge::LabelSuccess:
        style.append("background-color: #468847;");
        break;
    case Badge::LabelWarning:
        style.append("background-color: #F89406;");
        break;
    case Badge::LabelImportant:
        style.append("background-color: #B94A48;");
        break;
    case Badge::LabelInfo:
        style.append("background-color: #3A87AD;");
        break;
    case Badge::LabelInverse:
        style.append("background-color: #333333;");
        break;
    }
    style.append("}");
    setStyleSheet(style);
}
