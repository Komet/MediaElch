#include "MyTableView.h"

#include <QDebug>

MyTableView::MyTableView(QWidget* parent) : QTableView(parent), m_searchOverlay{new SearchOverlay(this)}
{
    connect(&m_searchOverlayTimer, &QTimer::timeout, this, &MyTableView::onSearchOverlayTimeout);
}

void MyTableView::resizeEvent(QResizeEvent* event)
{
    m_searchOverlay->setFixedWidth(event->size().width() - 80);
    m_searchOverlay->move((event->size().width() - m_searchOverlay->width()) / 2,
        (event->size().height() - m_searchOverlay->height()) / 2);
    QTableView::resizeEvent(event);
}

void MyTableView::setLastColumnWidth(int& width)
{
    setColumnWidth(model()->columnCount() - 1, width);
}

int MyTableView::lastColumnWidth() const
{
    return columnWidth(model()->columnCount() - 1);
}

void MyTableView::setFirstColumnWidth(int& width)
{
    setColumnWidth(0, width);
}

int MyTableView::firstColumnWidth() const
{
    return columnWidth(0);
}

void MyTableView::setUseSearchOverlay(const bool& useSearchOverlay)
{
    m_useSearchOverlay = useSearchOverlay;
}

void MyTableView::mouseMoveEvent(QMouseEvent* event)
{
    if (event->pos().x() > 0 && event->pos().x() < 30) {
        if (!m_mouseInLeftEdge) {
            m_mouseInLeftEdge = true;
            emit sigLeftEdge(true);
        }
    } else {
        if (m_mouseInLeftEdge) {
            m_mouseInLeftEdge = false;
            emit sigLeftEdge(false);
        }
    }
}

void MyTableView::keyPressEvent(QKeyEvent* keyEvent)
{
    const int key = keyEvent->key();
    const QString value = keyEvent->text();

    if (!m_useSearchOverlay || key == Qt::Key_Escape || (value.isEmpty() && key != Qt::Key_Backspace)) {
        QTableView::keyPressEvent(keyEvent);
        return;
    }

    // Character may be Ctrl+A, etc.
    if (value.size() != 1 || value[0] < 'a' || value[0] > 'z') {
        QTableView::keyPressEvent(keyEvent);
        return;
    }

    if (key == Qt::Key_Backspace) {
        if (!m_currentSearchText.isEmpty()) {
            m_currentSearchText.remove(m_currentSearchText.length() - 1, 1);
        }
        QTableView::keyPressEvent(keyEvent);
        return;
    }

    if (!value.isEmpty()) {
        m_currentSearchText.append(value);
    }

    m_searchOverlay->fadeIn();
    m_searchOverlay->setText(m_currentSearchText);
    m_searchOverlayTimer.start(1000);

    int matchingRow = -1;
    for (int i = 0, n = model()->rowCount(); i < n; ++i) {
        QModelIndex index = model()->index(i, 0);
        QString title = model()->data(index).toString();
        if (title.startsWith(m_currentSearchText, Qt::CaseInsensitive)) {
            scrollTo(index, QAbstractItemView::PositionAtCenter);
            selectRow(i);
            return;
        }
        if (title.contains(m_currentSearchText, Qt::CaseInsensitive)) {
            matchingRow = i;
        }
    }
    if (matchingRow > -1) {
        scrollTo(model()->index(matchingRow, 0), QAbstractItemView::PositionAtCenter);
        selectRow(matchingRow);
    }
}

bool MyTableView::useSearchOverlay() const
{
    return m_useSearchOverlay;
}

void MyTableView::onSearchOverlayTimeout()
{
    m_currentSearchText.clear();
    m_searchOverlay->fadeOut();
}
