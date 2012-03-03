#include "MyLineEdit.h"

#include <QDebug>
#include <QMovie>
#include <QStyle>
#include <QToolButton>

MyLineEdit::MyLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(myTextChanged(QString)));
}

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
}

void MyLineEdit::setLoading(bool loading)
{
    if (m_type != TypeLoading)
        return;
    m_loadingLabel->setVisible(loading);
    QLineEdit::setDisabled(loading);
}

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
        setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(m_loadingLabel->sizeHint().width() + frameWidth + 1));
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
        setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(m_clearButton->sizeHint().width() + frameWidth + 1));
        setMinimumSize(qMax(minimumSize.width(), m_clearButton->sizeHint().width() + frameWidth * 2 + 2),
                             (12 + frameWidth * 2 + 2));
        m_clearButton->setVisible(!text().isEmpty());
        connect(m_clearButton, SIGNAL(clicked()), this, SLOT(myClear()), Qt::UniqueConnection);
    }
}

MyLineEdit::LineEditType MyLineEdit::type()
{
    return m_type;
}

void MyLineEdit::myTextChanged(QString text)
{
    if (m_type != TypeClear)
        return;

    m_clearButton->setVisible(!text.isEmpty());
}

void MyLineEdit::myClear()
{
    setText("");
}
