#pragma once

#include "globals/Globals.h"

#include <QItemDelegate>
#include <QModelIndex>
#include <QWidget>

/**
 * @brief The ComboDelegate class
 * This class provides a combo box with items based on widget and type
 */
class ComboDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ComboDelegate(QObject* parent, MainWidgets widget, ComboDelegateType type);
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void
    updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;

private:
    MainWidgets m_widget;
    ComboDelegateType m_type;
};
