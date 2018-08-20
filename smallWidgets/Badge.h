#ifndef BADGE_H
#define BADGE_H

#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>

class Badge : public QLabel
{
    Q_OBJECT
public:
    enum class Type
    {
        LabelSuccess,
        LabelDefault,
        LabelWarning,
        LabelImportant,
        LabelInfo,
        LabelInverse,
        BadgeSuccess,
        BadgeDefault,
        BadgeWarning,
        BadgeImportant,
        BadgeInfo,
        BadgeInverse
    };

    explicit Badge(QWidget *parent = nullptr);
    explicit Badge(const QString &text, QWidget *parent = nullptr);
    void setBadgeType(Badge::Type type);
    void setClosable(const bool &closable);
    void setActive(const bool &active);
    void setFontBold(const bool &bold);
    bool isActive() const;
    bool isClosable() const;
    void setShowActiveMark(const bool &showActiveMark);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) override;

signals:
    void clicked();

private:
    Type m_badgeType;
    bool m_active;
    bool m_closable;
    bool m_fontBold;
    bool m_showActiveMark;
    void applyStyleSheet();
};

#endif // BADGE_H
