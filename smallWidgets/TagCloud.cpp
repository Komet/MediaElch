#include "TagCloud.h"
#include "ui_TagCloud.h"

#include <QDebug>

#include "Badge.h"
#include "globals/LocaleStringCompare.h"

TagCloud::TagCloud(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TagCloud),
    m_badgeType{TagCloud::TypeBadge},
    m_verticalSpace{4},
    m_horizontalSpace{4}
{
    ui->setupUi(this);
    connect(ui->scrollAreaWidgetContents, SIGNAL(resized()), this, SLOT(repositionTags()));
    connect(ui->lineEdit, SIGNAL(returnPressed()), this, SLOT(addTag()));
}

TagCloud::~TagCloud()
{
    delete ui;
}

void TagCloud::clear()
{
    ui->lineEdit->clear();
    m_tags.clear();
    m_activeTags.clear();
    drawTags();
}

void TagCloud::setTags(const QStringList &tags, const QStringList &activeTags)
{
    m_tags.clear();
    m_activeTags.clear();

    foreach (const QString &tag, tags) {
        if (!m_tags.contains(tag))
            m_tags.append(tag);
    }

    foreach (const QString &tag, activeTags) {
        if (!m_tags.contains(tag))
            m_tags.append(tag);
        if (!m_activeTags.contains(tag))
            m_activeTags.append(tag);
    }

    qSort(m_tags.begin(), m_tags.end(), LocaleStringCompare());
    qSort(m_activeTags.begin(), m_activeTags.end(), LocaleStringCompare());
    drawTags();
}

void TagCloud::drawTags()
{
    foreach (Badge *badge, m_badges)
        delete badge;
    m_badges.clear();

    int x = 0;
    int y = 0;
    int width = ui->scrollAreaWidgetContents->width();
    int heightToAdd = 0;

    QStringList tags = m_activeTags;
    foreach (const QString &tag, m_tags) {
        if (!tags.contains(tag))
            tags.append(tag);
    }

    foreach (const QString &word, tags) {
        Badge *badge = new Badge(word, ui->scrollAreaWidgetContents);
        if (m_badgeType == TagCloud::TypeSimpleLabel) {
            badge->setBadgeType(Badge::LabelWarning);
            badge->setShowActiveMark(false);
        } else {
            badge->setBadgeType(Badge::BadgeDefault);
            badge->setShowActiveMark(false);
        }
        if (m_activeTags.contains(word))
            badge->setActive(true);
        badge->show();
        if (x+badge->width()+m_horizontalSpace > width) {
            x = 0;
            y += badge->height()+m_verticalSpace;
        }
        if (badge->height() > heightToAdd)
            heightToAdd = badge->height();
        badge->move(x, y);
        m_badges.append(badge);
        x += badge->width() + m_horizontalSpace;
    }
    ui->scrollAreaWidgetContents->setFixedHeight(y+heightToAdd);
    setMaximumHeight(qMax(80, y+heightToAdd+qMax(30, ui->lineEdit->height())+25));
}

void TagCloud::repositionTags()
{
    int x = 0;
    int y = 0;
    int width = ui->scrollAreaWidgetContents->width();
    int heightToAdd = 0;

    foreach (Badge *badge, m_badges) {
        if (x+badge->width()+2 > width) {
            x = 0;
            y += badge->height()+m_verticalSpace;
        }
        if (badge->height() > heightToAdd)
            heightToAdd = badge->height();
        badge->move(x, y);
        x += badge->width() + 2;
    }
    ui->scrollAreaWidgetContents->setFixedHeight(y+heightToAdd);
    setMaximumHeight(qMax(80, y+heightToAdd+qMax(30, ui->lineEdit->height())+25));
}

void TagCloud::mousePressEvent(QMouseEvent *event)
{
    Badge *child = static_cast<Badge*>(childAt(event->pos()));
    if (!child || !child->inherits("Badge"))
        return;

    child->setActive(!child->isActive());
    if (child->isActive())
        emit activated(child->text());
    else
        emit deactivated(child->text());
    if (child->isActive()) {
        if (!m_activeTags.contains(child->text()))
            m_activeTags.append(child->text());
    } else {
        if (m_activeTags.contains(child->text()))
            m_activeTags.removeOne(child->text());
    }
    repositionTags();
}

void TagCloud::addTag()
{
    QString word = ui->lineEdit->text();
    ui->lineEdit->clear();
    if (word.isEmpty() || m_activeTags.contains(word))
        return;

    if (m_tags.contains(word)) {
        m_activeTags.append(word);
        foreach (Badge *badge, m_badges) {
            if (badge->text() == word) {
                badge->setActive(true);
                repositionTags();
                break;
            }
        }
    } else {
        m_tags.append(word);
        m_activeTags.append(word);
        Badge *badge = new Badge(word, ui->scrollAreaWidgetContents);
        if (m_badgeType == TagCloud::TypeSimpleLabel) {
            badge->setBadgeType(Badge::LabelWarning);
            badge->setShowActiveMark(false);
        } else {
            badge->setBadgeType(Badge::BadgeDefault);
            badge->setShowActiveMark(false);
        }
        badge->show();
        badge->setActive(true);
        m_badges.append(badge);
        repositionTags();
    }
    emit activated(word);
}

QStringList TagCloud::activeTags() const
{
    return m_activeTags;
}

void TagCloud::setText(const QString &text)
{
    ui->label->setText(text);
}

void TagCloud::setPlaceholder(const QString &placeholder)
{
    ui->lineEdit->setPlaceholderText(placeholder);
}

void TagCloud::setBadgeType(TagCloud::CloudBadgeType type)
{
    m_badgeType = type;
}

void TagCloud::setCompleter(QCompleter *completer)
{
    if (m_completer)
        m_completer->deleteLater();
    m_completer = completer;
    ui->lineEdit->setCompleter(m_completer);
    connect(m_completer, SIGNAL(activated(QString)), this, SLOT(addTag()));
    connect(m_completer, SIGNAL(activated(QString)), ui->lineEdit, SLOT(clear()), Qt::QueuedConnection);
}
