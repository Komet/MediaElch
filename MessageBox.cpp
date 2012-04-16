#include "MessageBox.h"
#include "ui_MessageBox.h"

#include <QDebug>
#include <QLabel>

MessageBox::MessageBox(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MessageBox)
{
    ui->setupUi(this);
    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(hide()));
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

void MessageBox::showMessage(QString message)
{
    ui->progressBar->hide();
    ui->label->setText(message);
    resize(this->size().width(), 48+ui->label->sizeHint().height());
    show();
    m_timer->start(3000);
}

void MessageBox::showProgressBar(QString message)
{
    m_timer->stop();
    ui->progressBar->show();
    ui->progressBar->setValue(0);
    ui->label->setText(message);
    resize(this->size().width(), 48+ui->label->sizeHint().height()+ui->progressBar->sizeHint().height()+4);
    show();
}

void MessageBox::progressBarProgress(int current, int max)
{
    ui->progressBar->setRange(0, max);
    ui->progressBar->setValue(current);
}

void MessageBox::hideProgressBar()
{
    ui->progressBar->hide();
    hide();
}
