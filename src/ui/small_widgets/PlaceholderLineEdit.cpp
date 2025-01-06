#include "ui/small_widgets/PlaceholderLineEdit.h"

#include "log/Log.h"
#include "renamer/PlaceholderParser.h"
#include "utils/Meta.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QLineEdit>
#include <QMenu>
#include <QPainter>

PlaceholderLineEdit::PlaceholderLineEdit(QWidget* parent) : QComboBox(parent)
{
    setEditable(true);

    connect(this, &PlaceholderLineEdit::editTextChanged, this, &PlaceholderLineEdit::onTextChanged);

    m_defaultTextColor = palette().color(QPalette::Text);
}

void PlaceholderLineEdit::onTextChanged(QString text)
{
    const mediaelch::PlaceholderParser::Result result = mediaelch::PlaceholderParser::parse(text);
    emit onValidated(result);

    if (result.hasError()) {
        QPalette p = palette();
        p.setColor(QPalette::Text, Qt::red);
        setPalette(p);
        setToolTip(tr("Errors in naming pattern: %1").arg(result.errorMessageList().join(", ")));

    } else {
        QPalette p = palette();
        p.setColor(QPalette::Text, m_defaultTextColor);
        setPalette(p);
        setToolTip("");
    }
}

void PlaceholderLineEdit::contextMenuEvent(QContextMenuEvent* event)
{
    QLineEdit* field = lineEdit();

    QMenu* menu = field->createStandardContextMenu();

    if (!m_placeholders.isEmpty()) {
        menu->addSeparator();
        QMenu* placeholderMenu = menu->addMenu(tr("Insert placeholder"));

        for (const QString& placeholder : asConst(m_placeholders)) {
            placeholderMenu->addAction(addDelimiters(placeholder), this, [this, placeholder, field]() {
                field->insert(addDelimiters(placeholder));
            });
        }
    }

    menu->exec(event->globalPos());
    delete menu;
}

QString PlaceholderLineEdit::addDelimiters(const QString& placeholder)
{
    return QStringLiteral("<%1>").arg(placeholder);
}

QStringList PlaceholderLineEdit::placeholders() const
{
    return m_placeholders;
}

void PlaceholderLineEdit::setPlaceholders(const QStringList& placeholders)
{
    m_placeholders = placeholders;
}

void PlaceholderLineEdit::setText(const QString& text)
{
    setCurrentText(text);
}

QString PlaceholderLineEdit::text() const
{
    return currentText();
}

void PlaceholderLineEdit::setItems(const QStringList& items)
{
    clear();
    addItems(items);
}
