#include "ui/small_widgets/RatingSourceDelegate.h"

#include "data/Rating.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QEvent>
#include <QLineEdit>

QWidget*
RatingSourceDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QLineEdit* editor = new QLineEdit(parent);

    QCompleter* completer = new QCompleter(Rating::commonSources(), parent);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    completer->setMaxVisibleItems(12);
    editor->setCompleter(completer);

    return editor;
}

void RatingSourceDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const QString value = index.model()->data(index, Qt::EditRole).toString();

    auto* lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(value);

    if (value.isEmpty()) {
        lineEdit->completer()->complete();
    }
}

void RatingSourceDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* lineEdit = static_cast<QLineEdit*>(editor);
    const QString value = lineEdit->text().trimmed();
    model->setData(index, value, Qt::EditRole);
}

void RatingSourceDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
