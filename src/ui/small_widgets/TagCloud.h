#pragma once

#include <QCompleter>
#include <QMouseEvent>
#include <QPointer>
#include <QWidget>

#include "ui/small_widgets/Badge.h"

namespace Ui {
class TagCloud;
}

class TagCloud : public QWidget
{
    Q_OBJECT

public:
    enum class BadgeType
    {
        SimpleLabel,
        Badge
    };

    explicit TagCloud(QWidget* parent = nullptr);
    ~TagCloud() override;
    void setTags(const QStringList& tags, const QStringList& activeTags);
    QStringList activeTags() const;
    void setText(const QString& text);
    void setPlaceholder(const QString& placeholder);
    void setBadgeType(BadgeType type);
    void clear();
    void setCompleter(QCompleter* completer);
    /// \brief Hides the "Tags" label.
    /// \todo Remove the label per default
    void hideLabel();

signals:
    void activated(QString);
    void deactivated(QString);

protected:
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void repositionTags();
    void addTag();

private:
    Ui::TagCloud* ui;
    BadgeType m_badgeType;
    int m_verticalSpace;
    int m_horizontalSpace;
    QStringList m_tags;
    QStringList m_activeTags;
    QVector<Badge*> m_badges;
    QPointer<QCompleter> m_completer;
    void drawTags(bool printAll = false);
};
