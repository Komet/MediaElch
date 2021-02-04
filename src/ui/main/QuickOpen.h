#pragma once

#include <QLineEdit>
#include <QMenu>
#include <QTreeView>

namespace mediaelch {

class QuickOpenFilterModel;

/// \brief A SublimeText-style quick open menu.
/// \note Based on https://invent.kde.org/utilities/kate/-/merge_requests/179
/// \details Use "setModel()" to set a list model. Only the DisplayRole of the first
///          column is used. Note that this class sets the model item's data for Role:Score.
class QuickOpen : public QMenu
{
    Q_OBJECT
public:
    QuickOpen(QWidget* parent = nullptr);
    ~QuickOpen() override = default;

    void updateViewGeometry();
    void setModel(QAbstractItemModel* model);

signals:
    void itemSelected(QModelIndex index);
    void closed();

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

public slots:
    void slotReturnPressed();
    void reselectFirst();

private:
    QTreeView* m_treeView;
    QLineEdit* m_lineEdit;
    QuickOpenFilterModel* m_proxyModel;
};

} // namespace mediaelch
