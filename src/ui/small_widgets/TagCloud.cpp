#include "TagCloud.h"
#include "ui_TagCloud.h"

#include <QDebug>

#include "Badge.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Meta.h"

TagCloud::TagCloud(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::TagCloud),
    m_badgeType{TagCloud::BadgeType::Badge},
    m_verticalSpace{4},
    m_horizontalSpace{4}
{
    ui->setupUi(this);
    connect(ui->scrollAreaWidgetContents, &MyWidget::resized, this, &TagCloud::repositionTags);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &TagCloud::addTag);
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

/**
 * \brief Adds tags to the tagcloud. All `activeTags` must also be included in `tags`.
 * \param tags        Non-active and active tags. Each tag must be distinct.
 * \param activeTags  Active tags. Each tag must be distinct.
 */
void TagCloud::setTags(const QStringList& tags, const QStringList& activeTags)
{
    m_tags = tags;
    m_activeTags = activeTags;

    std::sort(m_tags.begin(), m_tags.end(), LocaleStringCompare());
    std::sort(m_activeTags.begin(), m_activeTags.end(), LocaleStringCompare());
    drawTags();
}

void TagCloud::drawTags(bool printAll)
{
    for (Badge* badge : m_badges) {
        delete badge;
    }
    m_badges.clear();

    int x = 0;
    int y = 0;
    int width = ui->scrollAreaWidgetContents->width();
    int heightToAdd = 0;

    QStringList tags = m_activeTags;
    tags.reserve(m_tags.size());

    // \todo(bugwelle) Refactor. This is currently an ugly solution.
    // This ensures that we have at most 150 tags.
    const int maxNonActiveTagCount = (m_activeTags.size() > 150) ? 3 : 150;
    int nonActiveTagCount = 0;

    for (const QString& tag : m_tags) {
        if (!tags.contains(tag)) {
            tags.append(tag);
            ++nonActiveTagCount;
        }
        // We only want to print 3 non-active tags if there are too many.
        if (!printAll && nonActiveTagCount > maxNonActiveTagCount) {
            break;
        }
    }

    for (const QString& word : tags) {
        auto* badge = new Badge(word, ui->scrollAreaWidgetContents);
        if (m_badgeType == TagCloud::BadgeType::SimpleLabel) {
            badge->setBadgeType(Badge::Type::LabelWarning);
        } else {
            badge->setBadgeType(Badge::Type::BadgeDefault);
        }
        badge->setShowActiveMark(false);

        if (m_activeTags.contains(word)) {
            badge->setActive(true);
        }
        badge->show();
        if (x + badge->width() + m_horizontalSpace > width) {
            x = 0;
            y += badge->height() + m_verticalSpace;
        }
        if (badge->height() > heightToAdd) {
            heightToAdd = badge->height();
        }
        badge->move(x, y);
        m_badges.append(badge);
        x += badge->width() + m_horizontalSpace;
    }


    if (!printAll && nonActiveTagCount > maxNonActiveTagCount) {
        auto* badge = new Badge("[...] Click to see all", ui->scrollAreaWidgetContents);
        badge->setBadgeType(Badge::Type::BadgeDefault);
        badge->setShowActiveMark(false);
        badge->show();
        if (x + badge->width() + m_horizontalSpace > width) {
            x = 0;
            y += badge->height() + m_verticalSpace;
        }
        if (badge->height() > heightToAdd) {
            heightToAdd = badge->height();
        }
        badge->move(x, y);
        m_badges.append(badge);
    }

    ui->scrollAreaWidgetContents->setFixedHeight(y + heightToAdd);
    setMaximumHeight(qMax(80, y + heightToAdd + qMax(30, ui->lineEdit->height()) + 25));
}

void TagCloud::repositionTags()
{
    int x = 0;
    int y = 0;
    int width = ui->scrollAreaWidgetContents->width();
    int heightToAdd = 0;

    for (Badge* badge : m_badges) {
        if (x + badge->width() + 2 > width) {
            x = 0;
            y += badge->height() + m_verticalSpace;
        }
        if (badge->height() > heightToAdd) {
            heightToAdd = badge->height();
        }
        badge->move(x, y);
        x += badge->width() + 2;
    }
    ui->scrollAreaWidgetContents->setFixedHeight(y + heightToAdd);
    setMaximumHeight(qMax(80, y + heightToAdd + qMax(30, ui->lineEdit->height()) + 25));
}

void TagCloud::mousePressEvent(QMouseEvent* event)
{
    Badge* child = dynamic_cast<Badge*>(childAt(event->pos()));
    if ((child == nullptr) || !child->inherits("Badge")) {
        return;
    }

    // todo(bugwelle): Ugly. We just check if "..." is a tag.
    //                 It is added if too many tags are shown.
    if (child->text() == "[...] Click to see all") {
        drawTags(true);
        return;
    }

    child->setActive(!child->isActive());
    if (child->isActive()) {
        emit activated(child->text());
    } else {
        emit deactivated(child->text());
    }
    if (child->isActive()) {
        if (!m_activeTags.contains(child->text())) {
            m_activeTags.append(child->text());
        }
    } else {
        if (m_activeTags.contains(child->text())) {
            m_activeTags.removeOne(child->text());
        }
    }
    repositionTags();
}

void TagCloud::addTag()
{
    QString word = ui->lineEdit->text();
    ui->lineEdit->clear();
    if (word.isEmpty() || m_activeTags.contains(word)) {
        return;
    }

    if (m_tags.contains(word)) {
        m_activeTags.append(word);
        for (Badge* badge : m_badges) {
            if (badge->text() == word) {
                badge->setActive(true);
                repositionTags();
                break;
            }
        }
    } else {
        m_tags.append(word);
        m_activeTags.append(word);
        auto* badge = new Badge(word, ui->scrollAreaWidgetContents);
        if (m_badgeType == TagCloud::BadgeType::SimpleLabel) {
            badge->setBadgeType(Badge::Type::LabelWarning);
            badge->setShowActiveMark(false);
        } else {
            badge->setBadgeType(Badge::Type::BadgeDefault);
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

void TagCloud::setText(const QString& text)
{
    ui->lblTag->setText(text);
}

void TagCloud::setPlaceholder(const QString& placeholder)
{
    ui->lineEdit->setPlaceholderText(placeholder);
}

void TagCloud::setBadgeType(TagCloud::BadgeType type)
{
    m_badgeType = type;
}

void TagCloud::setCompleter(QCompleter* completer)
{
    if (m_completer != nullptr) {
        m_completer->deleteLater();
    }
    m_completer = completer;
    ui->lineEdit->setCompleter(m_completer);

    connect(m_completer,
        elchOverload<const QString&>(&QCompleter::activated), //
        this,
        [this](const QString& /*unused*/) { addTag(); });

    connect(
        m_completer,
        elchOverload<const QString&>(&QCompleter::activated),
        ui->lineEdit,
        [this](const QString& /*unused*/) { ui->lineEdit->clear(); },
        Qt::QueuedConnection);
}

void TagCloud::hideLabel()
{
    ui->lblTag->hide();
}
