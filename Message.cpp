#include "Message.h"
#include "ui_Message.h"

#include <QDebug>

Message::Message(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Message)
{
    ui->setupUi(this);
    ui->progressBar->hide();
    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

Message::~Message()
{
    delete ui;
}

void Message::setId(int id)
{
    m_id = id;
}

int Message::id()
{
    return m_id;
}

void Message::showProgressBar(bool show)
{
    ui->progressBar->setVisible(show);
    m_timer->start(30000);
}

void Message::setProgress(int current, int max)
{
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(current);
    m_timer->start(30000);
}

void Message::setMessage(QString message, int timeout)
{
    ui->label->setText(message);
    m_timer->start(timeout);
}

void Message::timeout()
{
    emit sigHideMessage(m_id);
}
