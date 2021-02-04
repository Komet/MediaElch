#include "ui/main/QuickOpen.h"

#include "third_party/kfts/kfts_fuzzy_match.h"

#include <QAbstractTextDocumentLayout>
#include <QCoreApplication>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QTextDocument>
#include <QVBoxLayout>

namespace mediaelch {

/// \brief Filters and sorts the source model according to a fuzzy filter match.
/// \note Based on https://invent.kde.org/utilities/kate/-/merge_requests/179
/// \details This proxy only prints the DisplayRole of the first column and ignores all styles.
///          It uses an internal score which depends on the fact that the model does not change
///          its size after "setSourceModel" is called.
class QuickOpenFilterModel : public QSortFilterProxyModel
{
public:
    QuickOpenFilterModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {}
    ~QuickOpenFilterModel() override = default;

public slots:
    void setFilterString(const QString& string)
    {
        beginResetModel();
        m_pattern = string;
        endResetModel();
    }

    void setSourceModel(QAbstractItemModel* sourceModel) override
    {
        // also default-initializes socres, i.e. 0
        m_scores.resize(sourceModel->rowCount());
        QSortFilterProxyModel::setSourceModel(sourceModel);
    }

public:
    int columnCount(const QModelIndex& parent = QModelIndex()) const override
    {
        if (parent.isValid()) {
            // non-root node
            return 0;
        }
        return 1;
    }

    /// \brief Proxy function that only uses the DisplayRole of the first column.
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
    {
        if (index.column() != 0 || role != Qt::DisplayRole) {
            return QVariant();
        }
        return sourceModel()->data(mapToSource(index), role);
    }

protected:
    bool lessThan(const QModelIndex& sourceLeft, const QModelIndex& sourceRight) const override
    {
        const int l = m_scores[sourceLeft.row()];
        const int r = m_scores[sourceRight.row()];
        return l < r;
    }

    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override
    {
        if (m_pattern.isEmpty())
            return true;

        int score = 0;
        const QModelIndex idx = sourceModel()->index(sourceRow, 0, sourceParent);
        QString displayRole = idx.data(Qt::DisplayRole).toString();
        const bool res = kfts::fuzzy_match_sequential(m_pattern, displayRole, score);
        m_scores[sourceRow] = score;
        return res;
    }

private:
    QString m_pattern;
    // TODO: No mutable
    mutable QVector<int> m_scores;
};

/// \brief Paints the model's data using fuzzy highlighting like SublimeText.
/// \note Based on https://invent.kde.org/utilities/kate/-/merge_requests/179
class QuickOpenStyleDelegate : public QStyledItemDelegate
{
public:
    QuickOpenStyleDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    ~QuickOpenStyleDelegate() override = default;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QStyleOptionViewItem options = option;
        initStyleOption(&options, index);

        QTextDocument doc;

        const QString original = index.data().toString();

        auto str = index.data().toString();
        const QString nameColor = option.palette.color(QPalette::Link).name();
        kfts::to_fuzzy_matched_display_string(
            m_filterString, str, QStringLiteral("<b style=\"color:%1;\">").arg(nameColor), QStringLiteral("</b>"));

        doc.setHtml(str);
        doc.setDocumentMargin(2);

        painter->save();

        // paint background
        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        } else {
            painter->fillRect(option.rect, option.palette.base());
        }

        options.text = QString(); // clear old text
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

        // draw text
        painter->translate(option.rect.x(), option.rect.y());

        QAbstractTextDocumentLayout::PaintContext ctx;
        ctx.palette.setColor(QPalette::Text, options.palette.text().color());
        doc.documentLayout()->draw(painter, ctx);

        painter->restore();
    }

public slots:
    void setFilterString(const QString& text) { m_filterString = text; }

private:
    QString m_filterString;
};


QuickOpen::QuickOpen(QWidget* parent) : QMenu(parent)
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(4, 4, 4, 4);
    setLayout(layout);

    m_lineEdit = new QLineEdit(this);
    setFocusProxy(m_lineEdit);

    layout->addWidget(m_lineEdit);

    m_treeView = new QTreeView();
    layout->addWidget(m_treeView, 1);
    m_treeView->setTextElideMode(Qt::ElideLeft);
    m_treeView->setUniformRowHeights(true);

    auto* delegate = new QuickOpenStyleDelegate(this);
    m_treeView->setItemDelegateForColumn(0, delegate);

    m_proxyModel = new QuickOpenFilterModel(this);
    m_proxyModel->setFilterRole(Qt::DisplayRole);
    m_proxyModel->setFilterKeyColumn(0);

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &QuickOpen::slotReturnPressed);
    connect(m_lineEdit, &QLineEdit::textChanged, m_proxyModel, &QuickOpenFilterModel::setFilterString);
    connect(m_lineEdit, &QLineEdit::textChanged, delegate, &QuickOpenStyleDelegate::setFilterString);
    connect(m_lineEdit, &QLineEdit::textChanged, this, [this]() {
        m_treeView->viewport()->update();
        reselectFirst();
    });
    connect(m_treeView, &QTreeView::clicked, this, &QuickOpen::slotReturnPressed);

    m_treeView->setSortingEnabled(true);
    m_treeView->setModel(m_proxyModel);

    m_treeView->installEventFilter(this);
    m_lineEdit->installEventFilter(this);

    m_treeView->setHeaderHidden(true);
    m_treeView->setRootIsDecorated(false);
    m_treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->setSelectionMode(QTreeView::SingleSelection);

    setHidden(true);
}

void QuickOpen::updateViewGeometry()
{
    m_treeView->resizeColumnToContents(0);
    m_treeView->resizeColumnToContents(1);

    const QSize centralSize = parentWidget()->size();

    // width: 2.4 of editor, height: 1/2 of editor
    const QSize viewMaxSize(centralSize.width() / 2.4, centralSize.height() / 2);

    // Position should be central over window
    const int xPos = std::max(0, (centralSize.width() - viewMaxSize.width()) / 2);
    const int yPos = std::max(0, (centralSize.height() - viewMaxSize.height()) * 1 / 4);

    const QPoint p(xPos, yPos);
    move(p + parentWidget()->pos());

    this->setFixedSize(viewMaxSize);
}

void QuickOpen::setModel(QAbstractItemModel* model)
{
    m_proxyModel->setSourceModel(model);
    updateViewGeometry();
    show();
    setFocus();
}

bool QuickOpen::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::ShortcutOverride) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (obj == m_lineEdit) {
            const bool forward2list = (keyEvent->key() == Qt::Key_Up) || (keyEvent->key() == Qt::Key_Down)
                                      || (keyEvent->key() == Qt::Key_PageUp) || (keyEvent->key() == Qt::Key_PageDown);
            if (forward2list) {
                QCoreApplication::sendEvent(m_treeView, event);
                return true;
            }

            if (keyEvent->key() == Qt::Key_Escape) {
                m_lineEdit->clear();
                keyEvent->accept();
                hide();
                emit closed();
                return true;
            }
        } else {
            const bool forward2input = (keyEvent->key() != Qt::Key_Up) && (keyEvent->key() != Qt::Key_Down)
                                       && (keyEvent->key() != Qt::Key_PageUp) && (keyEvent->key() != Qt::Key_PageDown)
                                       && (keyEvent->key() != Qt::Key_Tab) && (keyEvent->key() != Qt::Key_Backtab);
            if (forward2input) {
                QCoreApplication::sendEvent(m_lineEdit, event);
                return true;
            }
        }
    }
    // hide on focus out, if neither input field nor list have focus!
    else if (event->type() == QEvent::FocusOut && !(m_lineEdit->hasFocus() || m_treeView->hasFocus())) {
        m_lineEdit->clear();
        hide();
        emit closed();
        return true;
    }

    return QWidget::eventFilter(obj, event);
}

void QuickOpen::slotReturnPressed()
{
    QModelIndex index = m_proxyModel->mapToSource(m_treeView->currentIndex());
    emit itemSelected(index);

    m_lineEdit->clear();
    hide();
    emit closed();
}

void QuickOpen::reselectFirst()
{
    QModelIndex index = m_proxyModel->index(0, 0);
    m_treeView->setCurrentIndex(index);
}

} // namespace mediaelch
