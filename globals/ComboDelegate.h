#ifndef COMBODELEGATE_H
#define COMBODELEGATE_H

#include <QItemDelegate>

#include "globals/Globals.h"

/**
 * @brief The ComboDelegate class
 * This class provides a combo box with items based on widget and type
 */
class ComboDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit ComboDelegate(QObject *parent, MainWidgets widget, ComboDelegateType type);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    MainWidgets m_widget;
    ComboDelegateType m_type;
};

#endif // COMBODELEGATE_H
