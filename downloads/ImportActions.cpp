#include "ImportActions.h"
#include "ui_ImportActions.h"

#include <QDebug>

ImportActions::ImportActions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImportActions)
{
    ui->setupUi(this);
    ui->btnImport->setButtonStyle(StyledPushButton::StyleGreen);
    connect(ui->btnImport, SIGNAL(clicked()), this, SLOT(onImport()));
    m_tvShow = 0;
    m_importDialog = new ImportDialog(this);
}

ImportActions::~ImportActions()
{
    delete ui;
}

void ImportActions::setButtonEnabled(bool enabled)
{
    ui->btnImport->setEnabled(enabled);
}

void ImportActions::setBaseName(QString baseName)
{
    m_baseName = baseName;
}

QString ImportActions::baseName()
{
    return m_baseName;
}

void ImportActions::setType(QString type)
{
    m_type = type;
}

QString ImportActions::type()
{
    return m_type;
}

void ImportActions::setTvShow(TvShow *show)
{
    m_importDir.clear();
    m_tvShow = show;
}

TvShow *ImportActions::tvShow()
{
    return m_tvShow;
}

void ImportActions::setImportDir(QString dir)
{
    m_importDir = dir;
}

QString ImportActions::importDir()
{
    m_tvShow = 0;
    return m_importDir;
}

void ImportActions::setFiles(QStringList files)
{
    m_files = files;
}

QStringList ImportActions::files()
{
    return m_files;
}

void ImportActions::setExtraFiles(QStringList extraFiles)
{
    m_extraFiles = extraFiles;
}

QStringList ImportActions::extraFiles()
{
    return m_extraFiles;
}

void ImportActions::onImport()
{
    m_importDialog->setFiles(files());
    m_importDialog->setExtraFiles(extraFiles());

    if (type() == "movie") {
        m_importDialog->setImportDir(importDir());
        m_importDialog->execMovie(baseName());
    } else if (type() == "tvshow") {
        m_importDialog->execTvShow(baseName(), tvShow());
    } else if (type() == "concert") {
        m_importDialog->setImportDir(importDir());
        m_importDialog->execConcert(baseName());
    }
}
