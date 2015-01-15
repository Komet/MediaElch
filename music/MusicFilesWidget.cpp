#include "MusicFilesWidget.h"
#include "ui_MusicFilesWidget.h"

#include <QDebug>
#include "../globals/Manager.h"

MusicFilesWidget *MusicFilesWidget::m_instance;

MusicFilesWidget::MusicFilesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MusicFilesWidget)
{
    m_instance = this;
    Manager::instance()->setMusicFilesWidget(this);
    ui->setupUi(this);

    m_proxyModel = new MusicProxyModel(this);
    m_proxyModel->setSourceModel(Manager::instance()->musicModel());
    ui->music->setModel(m_proxyModel);
    ui->music->sortByColumn(0, Qt::AscendingOrder);
    ui->music->setAttribute(Qt::WA_MacShowFocusRect, false);

    connect(ui->music->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onItemSelected(QModelIndex)), Qt::QueuedConnection);
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
    Q_UNUSED(filters);
    Q_UNUSED(text);
}

void MusicFilesWidget::onItemSelected(QModelIndex index)
{
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    if (Manager::instance()->musicModel()->getItem(sourceIndex)->type() == TypeArtist)
        emit sigArtistSelected(Manager::instance()->musicModel()->getItem(sourceIndex)->artist());
    else if (Manager::instance()->musicModel()->getItem(sourceIndex)->type() == TypeAlbum)
        emit sigAlbumSelected(Manager::instance()->musicModel()->getItem(sourceIndex)->album());
}
