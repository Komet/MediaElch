#include "MessageBox.h"
#include "ui_MessageBox.h"

#include <QDebug>
#include <QLabel>

MessageBox::MessageBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    m_msgCounter = 0;
    hide();
}

MessageBox::~MessageBox()
{
    delete ui;
}

MessageBox *MessageBox::instance(QWidget *parent)
{
    static MessageBox *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MessageBox(parent);
    }
    return m_instance;
}

void MessageBox::reposition(QSize size)
{
    this->move(size.width()-this->size().width(), 0);
    m_parentSize = size;
}

void MessageBox::adjustSize()
{
    int height = 48;
    foreach (const Message *msg, m_messages)
        height += msg->sizeHint().height() + ui->layoutMessages->spacing();
    height -= ui->layoutMessages->spacing();
    resize(this->size().width(), height);
}

int MessageBox::showMessage(QString message, int timeout)
{
    m_msgCounter++;
    Message *msg = new Message(this);
    msg->setMessage(message, timeout);
    msg->setId(m_msgCounter);
    m_messages.append(msg);
    adjustSize();
    ui->layoutMessages->addWidget(msg);
    show();
    connect(msg, SIGNAL(sigHideMessage(int)), this, SLOT(removeMessage(int)));
    return m_msgCounter;
}

void MessageBox::removeMessage(int id)
{
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

void MessageBox::showProgressBar(QString message, int id)
{
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

void MessageBox::progressBarProgress(int current, int max, int id)
{
    foreach (Message *msg, m_messages) {
        if (msg->id() == id)
            msg->setProgress(current, max);
    }
}

void MessageBox::hideProgressBar(int id)
{
    removeMessage(id);
}
