#ifndef TAGCLOUD_H
#define TAGCLOUD_H

#include <QMouseEvent>
#include <QWidget>
#include "smallWidgets/Badge.h"

namespace Ui {
class TagCloud;
}

class TagCloud : public QWidget
{
    Q_OBJECT

public:
    explicit TagCloud(QWidget *parent = 0);
    ~TagCloud();
    void setTags(const QStringList &tags, const QStringList &activeTags);
    QStringList activeTags() const;
    void setText(const QString &text);
    void setPlaceholder(const QString &placeholder);

protected:
    void mousePressEvent(QMouseEvent *event);

private slots:
    void repositionTags();
    void addTag();

private:
    Ui::TagCloud *ui;
    int m_verticalSpace;
    int m_horizontalSpace;
    QStringList m_tags;
    QStringList m_activeTags;
    QList<Badge*> m_badges;
    void drawTags();

};

#endif // TAGCLOUD_H
