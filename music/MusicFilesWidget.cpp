#include "MusicFilesWidget.h"
#include "ui_MusicFilesWidget.h"

#include "../globals/Manager.h"

MusicFilesWidget *MusicFilesWidget::m_instance;

MusicFilesWidget::MusicFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicFilesWidget)
{
    m_instance = this;
    Manager::instance()->setMusicFilesWidget(this);
    ui->setupUi(this);

    ui->music->setModel(Manager::instance()->musicModel());
    ui->music->sortByColumn(0);
    ui->music->setAttribute(Qt::WA_MacShowFocusRect, false);
}

MusicFilesWidget::~MusicFilesWidget()
{
    delete ui;
}

MusicFilesWidget *MusicFilesWidget::instance()
{
    return m_instance;
}

void MusicFilesWidget::setFilter(QList<Filter *> filters, QString text)
{

}

void MusicFilesWidget::renewModel()
{

}
