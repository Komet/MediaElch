#include "MyLineEdit.h"

#include <QDebug>
#include <QMovie>
#include <QPainter>
#include <QStyle>
#include <QToolButton>

#include "globals/Globals.h"

/**
 * @brief MyLineEdit::MyLineEdit
 * @param parent
 */
MyLineEdit::MyLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    m_showMagnifier = false;
    m_magnifierLabel = 0;
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(myTextChanged(QString)));
}

/**
 * @brief Moves the icons to their positions
 */
void MyLineEdit::resizeEvent(QResizeEvent *)
{
    if (m_type == TypeLoading) {
        QSize size = m_loadingLabel->sizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_loadingLabel->move(rect().right()-frameWidth-size.width(), (rect().bottom()+1-size.height())/2);
    } else if (m_type == TypeClear) {
        QSize size = m_clearButton->sizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        m_clearButton->move(rect().right()-frameWidth-size.width()+2, (rect().bottom()+1-size.height()+6)/2);
    }

    if (m_showMagnifier) {
        QSize size = m_magnifierLabel->sizeHint();
        m_magnifierLabel->move(6, (rect().bottom()+1-size.height())/2);
    }
}

/**
 * @brief Shows/hides the loading movie and disabled/enables the line edit
 * @param loading Is loading
 */
void MyLineEdit::setLoading(bool loading)
{
    if (m_type != TypeLoading)
        return;
    m_loadingLabel->setVisible(loading);
    QLineEdit::setDisabled(loading);
}

/**
 * @brief Sets the type of the line edit
 * @param type Type of the line edit
 */
void MyLineEdit::setType(LineEditType type)
{
    m_type = type;

    if (type == TypeLoading) {
        m_loadingLabel = new QLabel(this);
        QMovie *movie = new QMovie(":/img/spinner.gif");
        movie->start();
        m_loadingLabel->setMovie(movie);
        QSize minimumSize = minimumSizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        setStyleSheet(styleSheet() + QString("%1 QLineEdit { padding-right: %2px; } ").arg(m_initialStyleSheet).arg(m_loadingLabel->sizeHint().width() + frameWidth + 1));
        setMinimumSize(qMax(minimumSize.width(), m_loadingLabel->sizeHint().width() + frameWidth * 2 + 2),
                             qMax(minimumSize.height(), m_loadingLabel->sizeHint().height() + frameWidth * 2 + 2));
        m_loadingLabel->setHidden(true);
    }

    if (type == TypeClear) {
        m_clearButton = new QToolButton(this);
        m_clearButton->setFixedSize(14, 14);
        m_clearButton->setCursor(Qt::ArrowCursor);
        m_clearButton->setIcon(QIcon(":/img/stop.png"));
        m_clearButton->setStyleSheet("background-color: transparent; border: none;");
        QSize minimumSize = minimumSizeHint();
        int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
        setStyleSheet(styleSheet() + QString("%1 QLineEdit { padding-right: %2px; } ").arg(m_initialStyleSheet).arg(m_clearButton->sizeHint().width() + frameWidth + 1));
        setMinimumSize(qMax(minimumSize.width(), m_clearButton->sizeHint().width() + frameWidth * 2 + 2),
                             (12 + frameWidth * 2 + 2));
        m_clearButton->setVisible(!text().isEmpty());
        connect(m_clearButton, SIGNAL(clicked()), this, SLOT(myClear()), Qt::UniqueConnection);
    }
}

/**
 * @brief Returns the type of the line edit
 * @return Type of the line edit
 */
MyLineEdit::LineEditType MyLineEdit::type()
{
    return m_type;
}

/**
 * @brief Shows/hides the clear button
 * @param text Current text
 */
void MyLineEdit::myTextChanged(QString text)
{
    if (m_type != TypeClear)
        return;

    m_clearButton->setVisible(!text.isEmpty());
}

/**
 * @brief Clears the text
 */
void MyLineEdit::myClear()
{
    setText("");
}

/**
 * @brief Sets and additional style sheet
 * @param style Stylesheet
 */
void MyLineEdit::setAdditionalStyleSheet(QString style)
{
    m_initialStyleSheet = style;
}

/**
 * @brief Show/hide the magnifier
 * @param show
 */
void MyLineEdit::setShowMagnifier(bool show)
{
    m_showMagnifier = show;
    if (show) {
        if (m_magnifierLabel != 0)
            delete m_magnifierLabel;
        QPixmap magn(":/img/magnifier.png");
        magn = magn.scaled(14, 14, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QPainter p;
        p.begin(&magn);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(magn.rect(), QColor(0, 0, 0, 150));
        p.end();

        m_magnifierLabel = new QLabel(this);
        m_magnifierLabel->setPixmap(magn);
        m_magnifierLabel->setStyleSheet("border: none;");
        setStyleSheet(styleSheet() + " QLineEdit { padding-left: 20px; } ");
    }
}
