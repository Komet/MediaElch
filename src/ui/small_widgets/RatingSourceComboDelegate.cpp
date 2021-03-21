#include "ui/small_widgets/RatingSourceComboDelegate.h"

#include "data/Rating.h"

#include <QComboBox>

QWidget* RatingSourceComboDelegate::createEditor(QWidget* parent,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    QComboBox* editor = new QComboBox(parent);
    editor->setFrame(false);
    // editor->setMinimum(0);
    // editor->setMaximum(100);

    const QStringList commonSources = Rating::commonSources();

    for (const QString& source : commonSources) {
        editor->addItem(Rating::sourceToName(source), source);
    }

    return editor;
}

void RatingSourceComboDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox* comboBox = static_cast<QComboBox*>(editor);

    int comboIndex = comboBox->findData(value);
    if (comboIndex < 0) {
        comboIndex = 0;
    }

    comboBox->setCurrentIndex(comboIndex);
}

void RatingSourceComboDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = static_cast<QComboBox*>(editor);

    const QString value = comboBox->currentData().toString();

    model->setData(index, value, Qt::EditRole);
}

void RatingSourceComboDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
