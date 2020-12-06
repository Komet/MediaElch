#include "MyLineEdit.h"

#include <QDebug>
#include <QMovie>
#include <QPainter>
#include <QStyle>
#include <QToolButton>

#include "globals/Globals.h"
#include "globals/Helper.h"

/**
 * \brief MyLineEdit::MyLineEdit
 */
MyLineEdit::MyLineEdit(QWidget* parent) : QLineEdit(parent), m_loadingLabel{new QLabel(nullptr)}
{
    m_moreLabel = new QLabel(this);
    m_moreLabel->setText("...");
    m_moreLabel->setStyleSheet("font-size: 10px; color: #a0a0a0;");
    m_moreLabel->hide();
    m_loadingLabel->hide();
    connect(this, &QLineEdit::textChanged, this, &MyLineEdit::myTextChanged);
}

/**
 * \brief Moves the icons to their positions
 */
void MyLineEdit::resizeEvent(QResizeEvent* /*event*/)
{
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);

    if (m_type == TypeLoading) {
        const QSize size = m_loadingLabel->sizeHint();
        m_loadingLabel->move(rect().right() - frameWidth - size.width(), (rect().bottom() + 1 - size.height()) / 2);

    } else if (m_type == TypeClear) {
        const QSize size = m_clearButton->sizeHint();
        m_clearButton->move(
            rect().right() - frameWidth - size.width() + 2, (rect().bottom() + 1 - size.height() + 6) / 2);
    }

    if (m_showMagnifier) {
        const QSize magnifierSize = m_magnifierLabel->sizeHint();
        m_magnifierLabel->move(6, (rect().bottom() + 1 - magnifierSize.height()) / 2);
    }
}

/**
 * \brief Captures key events and emits signals based on the key
 */
void MyLineEdit::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Down) {
        emit keyDown();
        return;
    }
    if (event->key() == Qt::Key_Up) {
        emit keyUp();
        return;
    }
    if (event->key() == Qt::Key_Backspace && cursorPosition() == 0) {
        emit backspaceInFront();
    }
    QLineEdit::keyPressEvent(event);
}

/**
 * \brief Emits custom focusOut signal
 */
void MyLineEdit::focusOutEvent(QFocusEvent* event)
{
    emit focusOut();
    QLineEdit::focusOutEvent(event);
}

/**
 * \brief Emits custom focusIn signal
 */
void MyLineEdit::focusInEvent(QFocusEvent* event)
{
    emit focusIn();
    QLineEdit::focusInEvent(event);
}

/**
 * \brief Shows/hides the loading movie and disabled/enables the line edit
 * \param loading Is loading
 */
void MyLineEdit::setLoading(bool loading)
{
    if (m_type != TypeLoading) {
        return;
    }
    m_loadingLabel->setVisible(loading);
    QLineEdit::setDisabled(loading);
}

/**
 * \brief Sets the type of the line edit
 * \param type Type of the line edit
 */
void MyLineEdit::setType(LineEditType type)
{
    m_type = type;

    if (type == TypeLoading) {
        m_loadingLabel->deleteLater();
        m_loadingLabel = new QLabel(this);
        auto* movie = new QMovie(":/img/spinner.gif", QByteArray(), this);
        movie->start();
        m_loadingLabel->setMovie(movie);
        QSize minimumSize = minimumSizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_styleSheets.append(
            QString("QLineEdit { padding-right: %2px; }").arg(m_loadingLabel->sizeHint().width() + frameWidth + 1));
        setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(m_paddingLeft));
        setMinimumSize(qMax(minimumSize.width(), m_loadingLabel->sizeHint().width() + frameWidth * 2 + 2),
            qMax(minimumSize.height(), m_loadingLabel->sizeHint().height() + frameWidth * 2 + 2));
        m_loadingLabel->setHidden(true);
    }

    if (type == TypeClear) {
        if (m_clearButton != nullptr) {
            m_clearButton->deleteLater();
        }
        m_clearButton = new QToolButton(this);
        m_clearButton->setIconSize({14, 14});
        // TODO: Padding (left/right) is quite big. But setting the size using
        //       setFixedWidth does not really work. Probably due to padding.

        m_clearButton->setCursor(Qt::PointingHandCursor);
        m_clearButton->setIcon(QIcon(":/img/stop.png"));
        m_clearButton->setStyleSheet("background-color: transparent; border: none;");

        QSize minimumSize = minimumSizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_styleSheets.append(
            QString("QLineEdit { padding-right: %2px; } ").arg(m_clearButton->sizeHint().width() + frameWidth + 1));
        setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(m_paddingLeft));
        setMinimumSize(qMax(minimumSize.width(), m_clearButton->sizeHint().width() + frameWidth * 2 + 2),
            (12 + frameWidth * 2 + 2));

        m_clearButton->setVisible(!text().isEmpty() || hasFilters());
        connect(m_clearButton, &QToolButton::clicked, this, &MyLineEdit::myClear, Qt::UniqueConnection);
    }
}

/**
 * \brief Returns the type of the line edit
 * \return Type of the line edit
 */
MyLineEdit::LineEditType MyLineEdit::type()
{
    return m_type;
}

/**
 * \brief Shows/hides the clear button
 * \param text Current text
 */
void MyLineEdit::myTextChanged(QString text)
{
    if (m_type != TypeClear) {
        return;
    }

    m_clearButton->setVisible(!text.isEmpty() || hasFilters());
}

void MyLineEdit::myClear()
{
    setText("");
    emit clearClicked();
}

/**
 * \brief Sets and additional style sheet
 * \param style Stylesheet
 */
void MyLineEdit::addAdditionalStyleSheet(QString style)
{
    m_styleSheets.append(style);
}

/**
 * \brief Show/hide the magnifier
 */
void MyLineEdit::setShowMagnifier(bool show)
{
    if (m_showMagnifier && !show) {
        m_paddingLeft -= 24;
    } else if (!m_showMagnifier && show) {
        m_paddingLeft += 24;
    }

    m_showMagnifier = show;

    if (show) {
        delete m_magnifierLabel;

        QPixmap magn(":/img/magnifier.png");
        magn =
            magn.scaled(QSize(14, 14) * helper::devicePixelRatio(this), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        helper::setDevicePixelRatio(magn, helper::devicePixelRatio(this));
        QPainter p;
        p.begin(&magn);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(magn.rect(), QColor(0, 0, 0, 150));
        p.end();

        m_magnifierLabel = new QLabel(this);
        m_magnifierLabel->setPixmap(magn);
        m_magnifierLabel->setStyleSheet("border: none;");
        setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(m_paddingLeft));
    }
}

/**
 * \brief Adds a filter and clears text
 */
void MyLineEdit::addFilter(Filter* filter)
{
    auto* label = new QLabel(this);
    if (filter->isInfo(MovieFilters::Title) || filter->isInfo(MovieFilters::OriginalTitle)
        || filter->isInfo(MovieFilters::Path) || filter->isInfo(ConcertFilters::Title)
        || filter->isInfo(TvShowFilters::Title) || filter->isInfo(MusicFilters::Title)) {
        label->setStyleSheet("background-color: #999999; border: 1px solid #999999; border-radius: 2px; font-size: "
                             "10px; color: #ffffff;");
    } else if (filter->isInfo(MovieFilters::ImdbId) || filter->isInfo(MovieFilters::TmdbId)) {
        label->setStyleSheet("background-color: #F0AD4E; border: 1px solid #F0AD4E; border-radius: 2px; font-size: "
                             "10px; color: #ffffff;");
    } else {
        label->setStyleSheet("background-color: #5BC0DE; border: 1px solid #5BC0DE; border-radius: 2px; font-size: "
                             "10px; color: #ffffff;");
    }
    label->setText(filter->shortText());
    label->show();
    m_filterLabels.append(label);
    drawFilters();
    setText("");
}

/**
 * \brief Removes the last filter
 */
void MyLineEdit::removeLastFilter()
{
    if (m_filterLabels.count() == 0) {
        return;
    }
    m_filterLabels.takeLast()->deleteLater();
    drawFilters();
}

/**
 * \brief Clears all filters
 */
void MyLineEdit::clearFilters()
{
    for (QLabel* label : m_filterLabels) {
        label->deleteLater();
    }
    m_filterLabels.clear();
    drawFilters();
}

bool MyLineEdit::hasFilters() const
{
    return !m_filterLabels.isEmpty();
}

/**
 * \brief Draws the filter labels
 *        If necessary this also show the "..." label in front of the filter labels
 */
void MyLineEdit::drawFilters()
{
    int paddingLeft = m_paddingLeft;
    int labelWidth = 0;
    int hidden = 0;
    for (QLabel* l : m_filterLabels) {
        labelWidth += l->width() + 2;
        l->show();
    }
    while (labelWidth + 50 > width() && hidden < m_filterLabels.count()) {
        m_filterLabels.at(hidden++)->hide();
        labelWidth = 0;
        for (QLabel* l : m_filterLabels) {
            if (l->isVisible()) {
                labelWidth += l->width() + 2;
            }
        }
    }

    if (hidden > 0) {
        m_moreLabel->move(m_paddingLeft, 1);
        m_moreLabel->show();
        paddingLeft += m_moreLabel->width();
    } else {
        m_moreLabel->hide();
    }

    for (QLabel* l : m_filterLabels) {
        if (l->isVisible() || l == m_filterLabels.last()) {
            l->move(paddingLeft, 1);
            paddingLeft += l->width() + 2;
        }
    }

    setStyleSheet(m_styleSheets.join(" ") + QString(" QLineEdit { padding-left: %1px; }").arg(paddingLeft));
}

/**
 * \brief Returns the current left padding. The padding is calculated by
 *        summing up all visible filter labels
 * \return Current offset from left
 */
int MyLineEdit::paddingLeft()
{
    int paddingLeft = m_paddingLeft;
    if (m_moreLabel->isVisible()) {
        paddingLeft += m_moreLabel->width();
    }
    for (QLabel* l : m_filterLabels) {
        if (l->isVisible()) {
            paddingLeft += l->width() + 2;
        }
    }
    return paddingLeft;
}
