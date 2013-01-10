#ifndef BADGE_H
#define BADGE_H

#include <QLabel>

class Badge : public QLabel
{
    Q_OBJECT
public:
    enum BadgeType {
        LabelSuccess, LabelDefault, LabelWarning, LabelImportant, LabelInfo, LabelInverse
    };

    explicit Badge(QWidget *parent = 0);
    void setBadgeType(Badge::BadgeType type);
private:
    Badge::BadgeType m_badgeType;
};

#endif // BADGE_H
