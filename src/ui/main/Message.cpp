#include "Message.h"
#include "ui_Message.h"

#include <QDebug>
#include <QGraphicsDropShadowEffect>

#include "globals/Globals.h"
#include "globals/Helper.h"
#include "ui/notifications/NotificationBox.h"

Message::Message(QWidget* parent) : QWidget(parent), ui(new Ui::Message)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(0);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &Message::timeout);

    if (helper::devicePixelRatio(this) >= 0.95 && helper::devicePixelRatio(this) <= 1.05) {
        auto* effect = new QGraphicsDropShadowEffect(this);
        effect->setColor(QColor(0, 0, 0, 30));
        effect->setOffset(4);
        effect->setBlurRadius(8);
        setGraphicsEffect(effect);
    }

    setType(NotificationType::NotificationInfo);
}

/**
 * \brief Message::~Message
 */
Message::~Message()
{
    delete ui;
}

void Message::setType(NotificationType type)
{
    switch (type) {
    case NotificationType::NotificationInfo:
        setStyleSheet("#widget { border-left: 5px solid #5BC0DE; background-color: #F4F8FA; }");
        break;
    case NotificationType::NotificationSuccess:
        setStyleSheet("#widget { border-left: 5px solid #50d850; background-color: #f4faf6; }");
        break;
    case NotificationType::NotificationError:
        setStyleSheet("#widget { border-left: 5px solid #D9534F; background-color: #FDF7F7; }");
        break;
    case NotificationType::NotificationWarning:
        setStyleSheet("#widget { border-left: 5px solid #F0AD4E; background-color: #FCF8F2; }");
        break;
    }
}

/**
 * \brief Sets the (unique) id of the message
 * \param id Id
 * \see Message::id
 */
void Message::setId(int id)
{
    m_id = id;
}

/**
 * \property Message::id
 * \brief Holds the (unique) id of the message
 * \return Id of the message
 * \see Message::setId
 */
int Message::id() const
{
    return m_id;
}

/**
 * \brief Shows or hides the progress bar
 * \param show Show the progress bar (or not)
 */
void Message::showProgressBar(bool show)
{
    ui->progressBar->setVisible(show);
    m_timer->start(10 * 60 * 1000);
}

/**
 * \brief Sets the value of the progress bar
 * \param current Current value
 * \param max Maximum value
 */
void Message::setProgress(int current, int max)
{
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(current);
    m_timer->start(10 * 60 * 1000);
}

/**
 * \brief Sets the message to be displayed
 * \param message Message
 * \param timeout How long should it be visible
 */
void Message::setMessage(QString message, int timeout)
{
    ui->label->setText(message);
    m_timer->start(timeout);
}

/**
 * \brief Message::timeout
 */
void Message::timeout()
{
    qDebug() << "Entered, m_id=" << m_id;
    emit sigHideMessage(m_id);
}

int Message::maxValue() const
{
    return ui->progressBar->maximum();
}

int Message::value() const
{
    return ui->progressBar->value();
}
