#include "ui/small_widgets/SpinBoxDelegate.h"

#include <QSpinBox>

SpinBoxDelegate::SpinBoxDelegate(QObject* parent) : QStyledItemDelegate(parent)
{
}

QWidget*
SpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto* editor = new QSpinBox(parent);
    editor->setFrame(false);
    editor->setRange(0, 1 << 30);

    return editor;
}

void SpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const int value = index.model()->data(index, Qt::EditRole).toInt();

    auto* spinBox = dynamic_cast<QSpinBox*>(editor);
    spinBox->setValue(value);
}

void SpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* spinBox = dynamic_cast<QSpinBox*>(editor);
    spinBox->interpretText();
    const int value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void SpinBoxDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}


DoubleSpinBoxDelegate::DoubleSpinBoxDelegate(double stepSize, QObject* parent) :
    QStyledItemDelegate(parent), m_stepSize{stepSize}
{
}

QWidget*
DoubleSpinBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)

    auto* editor = new QDoubleSpinBox(parent);
    editor->setFrame(false);
    editor->setSingleStep(m_stepSize);
    editor->setRange(0, 1 << 30);

    return editor;
}

void DoubleSpinBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    const double value = index.model()->data(index, Qt::EditRole).toDouble();

    auto* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);
    spinBox->setValue(value);
}

void DoubleSpinBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* spinBox = dynamic_cast<QDoubleSpinBox*>(editor);
    spinBox->interpretText();
    const double value = spinBox->value();

    model->setData(index, value, Qt::EditRole);
}

void DoubleSpinBoxDelegate::updateEditorGeometry(QWidget* editor,
    const QStyleOptionViewItem& option,
    const QModelIndex& index) const
{
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}
