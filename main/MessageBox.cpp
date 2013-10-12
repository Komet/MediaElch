#include "main/MessageBox.h"
#include "ui_MessageBox.h"

#include <QDebug>
#include <QLabel>

/**
 * @brief MessageBox::MessageBox
 * @param parent
 */
MessageBox::MessageBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    m_msgCounter = 0;
    hide();
}

/**
 * @brief MessageBox::~MessageBox
 */
MessageBox::~MessageBox()
{
    delete ui;
}

/**
 * @brief Returns an instance of the message box
 * @param parent Parent widget (used when called the first time)
 * @return Instance of message box
 */
MessageBox *MessageBox::instance(QWidget *parent)
{
    static MessageBox *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MessageBox(parent);
    }
    return m_instance;
}

/**
 * @brief Repositions the message box in the upper right corner
 * @param size Size of the parent widget (MainWidget)
 */
void MessageBox::reposition(QSize size)
{
    this->move(size.width()-this->size().width(), 0);
    m_parentSize = size;
}

/**
 * @brief Adjusts the size to hold all the messages
 */
void MessageBox::adjustSize()
{
    qDebug() << "Entered";
    int height = 48;
    foreach (const Message *msg, m_messages)
        height += msg->sizeHint().height() + ui->layoutMessages->spacing();
    height -= ui->layoutMessages->spacing();
    qDebug() << "Setting size to" << size().width() << "x" << height;
    resize(this->size().width(), height);
}

/**
 * @brief Shows a message
 * @param message Message to show
 * @param timeout How long should it be visible
 * @return Id of the message
 */
int MessageBox::showMessage(QString message, int timeout)
{
    qDebug() << "Entered, message=" << message << "timeout=" << timeout;
    m_msgCounter++;
    Message *msg = new Message(this);
    msg->setMessage(message, timeout);
    msg->setId(m_msgCounter);
    m_messages.append(msg);
    adjustSize();
    ui->layoutMessages->addWidget(msg);
    show();
    connect(msg, SIGNAL(sigHideMessage(int)), this, SLOT(removeMessage(int)));
    //qApp->processEvents(QEventLoop::WaitForMoreEvents);
    return m_msgCounter;
}

/**
 * @brief Removes a message
 * @param id Id of the message to remove
 */
void MessageBox::removeMessage(int id)
{
    qDebug() << "Entered, id=" << id;
    foreach (Message *msg, m_messages) {
        if (msg->id() == id) {
            ui->layoutMessages->removeWidget(msg);
            msg->deleteLater();
            m_messages.removeOne(msg);
            adjustSize();
        }
    }
    if (m_messages.size() == 0)
        hide();
}

/**
 * @brief Shows a message with progress bar
 * @param message Message to display
 * @param id Id of the message
 */
void MessageBox::showProgressBar(QString message, int id, bool unique)
{
    qDebug() << "Entered, message=" << message << "id=" << id;
    if (unique) {
        foreach (Message *msg, m_messages) {
            if (msg->id() == id)
                return;
        }
    }
    m_msgCounter++;
    Message *msg = new Message(this);
    msg->setMessage(message);
    msg->setId(id);
    msg->showProgressBar(true);
    m_messages.append(msg);
    adjustSize();
    ui->layoutMessages->addWidget(msg);
    show();
    connect(msg, SIGNAL(sigHideMessage(int)), this, SLOT(removeMessage(int)));
}

/**
 * @brief Sets the value of a message with progress bar
 * @param current Current value
 * @param max Maximum value
 * @param id Id of the message
 */
void MessageBox::progressBarProgress(int current, int max, int id)
{
    foreach (Message *msg, m_messages) {
        if (msg->id() == id)
            msg->setProgress(current, max);
    }
}

/**
 * @brief Hides a message with progress bar
 * @param id Id of message to hide
 */
void MessageBox::hideProgressBar(int id)
{
    qDebug() << "Entered, id=" << id;
    removeMessage(id);
}

int MessageBox::maxValue(int id)
{
    foreach (Message *msg, m_messages) {
        if (msg->id() == id)
            return msg->maxValue();
    }

    return 0;
}

int MessageBox::value(int id)
{
    foreach (Message *msg, m_messages) {
        if (msg->id() == id)
            return msg->value();
    }

    return 0;
}
