#include "ui/small_widgets/RatingSourceDelegate.h"

#include "data/Rating.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCompleter>
#include <QEvent>

QWidget*
RatingSourceDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QComboBox* editor = new QComboBox(parent);

    editor->addItems(Rating::commonSources());
    editor->setEditable(true);

    QCompleter* completer = new QCompleter(Rating::commonSources(), parent);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    editor->setCompleter(completer);

    return editor;
}

void RatingSourceDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const QString value = index.model()->data(index, Qt::EditRole).toString();

    auto* combox = dynamic_cast<QComboBox*>(editor);
    combox->setCurrentText(value);
}

void RatingSourceDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* combox = dynamic_cast<QComboBox*>(editor);
    const QString value = combox->currentText().trimmed();
    model->setData(index, value, Qt::EditRole);
}

void RatingSourceDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
