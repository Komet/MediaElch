#include "ui/settings/ImportSettingsWidget.h"
#include "ui_ImportSettingsWidget.h"

#include "settings/Settings.h"

#include <QFileDialog>

ImportSettingsWidget::ImportSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ImportSettingsWidget)
{
    ui->setupUi(this);

    // clang-format off
    connect(ui->btnChooseUnrar,      &QAbstractButton::clicked, this, &ImportSettingsWidget::onChooseUnrar);
    connect(ui->btnChooseMakemkvcon, &QAbstractButton::clicked, this, &ImportSettingsWidget::onChooseMakeMkvCon);
    // clang-format on
}

ImportSettingsWidget::~ImportSettingsWidget()
{
    delete ui;
}

void ImportSettingsWidget::setSettings(Settings& settings)
{
    m_settings = std::make_unique<ImportSettings>(settings);
}

void ImportSettingsWidget::loadSettings()
{
    ui->chkDeleteArchives->setChecked(m_settings->deleteArchives());
    ui->unrarPath->setText(m_settings->unrar());
    ui->makemkvconPath->setText(m_settings->makeMkvCon());
}

void ImportSettingsWidget::saveSettings()
{
    // Downloads
    m_settings->setUnrar(ui->unrarPath->text());
    m_settings->setMakeMkvCon(ui->makemkvconPath->text());
    m_settings->setDeleteArchives(ui->chkDeleteArchives->isChecked());
}

void ImportSettingsWidget::onChooseUnrar()
{
    QString unrar = QFileDialog::getOpenFileName(this, tr("Choose unrar"), QDir::homePath());
    if (!unrar.isEmpty()) {
        ui->unrarPath->setText(unrar);
    }
}

void ImportSettingsWidget::onChooseMakeMkvCon()
{
    QString makeMkvCon = QFileDialog::getOpenFileName(this, tr("Choose makemkvcon"), QDir::homePath());
    if (!makeMkvCon.isEmpty()) {
        ui->makemkvconPath->setText(makeMkvCon);
    }
}
