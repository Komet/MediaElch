#include "PluginManagerDialog.h"
#include "ui_PluginManagerDialog.h"

#include "settings/Settings.h"

PluginManagerDialog::PluginManagerDialog(QWidget *parent) : QDialog(parent), ui(new Ui::PluginManagerDialog)
{
    ui->setupUi(this);

    // clang-format off
    connect(ui->btnInstallPlugin,      &QAbstractButton::clicked,               this, &PluginManagerDialog::onInstallPlugin);
    connect(ui->btnUpdatePlugin,       &QAbstractButton::clicked,               this, &PluginManagerDialog::onUpdatePlugin);
    connect(PluginManager::instance(), &PluginManager::sigPluginInstalled,      this, &PluginManagerDialog::onPluginInstalled);
    connect(PluginManager::instance(), &PluginManager::sigPluginUpdated,        this, &PluginManagerDialog::onPluginUpdated);
    connect(PluginManager::instance(), &PluginManager::sigPluginInstallFailure, this, &PluginManagerDialog::onPluginFailure);
    // clang-format on
}

PluginManagerDialog::~PluginManagerDialog()
{
    delete ui;
}

int PluginManagerDialog::exec()
{
    ui->msgLabel->setText("");
    return QDialog::exec();
}

void PluginManagerDialog::installPlugin(PluginManager::Plugin plugin)
{
    m_plugin = plugin;
    ui->btnInstallPlugin->setVisible(true);
    ui->btnUpdatePlugin->setVisible(false);
    exec();
}

void PluginManagerDialog::updatePlugin(PluginManager::Plugin plugin)
{
    m_plugin = plugin;
    ui->btnInstallPlugin->setVisible(false);
    ui->btnUpdatePlugin->setVisible(true);
    exec();
}

void PluginManagerDialog::onInstallPlugin()
{
    ui->btnInstallPlugin->setEnabled(false);
    ui->msgLabel->setText(tr("Plugin is being installed..."));
    PluginManager::instance()->installPlugin(m_plugin);
}

void PluginManagerDialog::onUpdatePlugin()
{
    ui->btnUpdatePlugin->setEnabled(false);
    ui->msgLabel->setText(tr("Plugin is being updated..."));
    PluginManager::instance()->updatePlugin(m_plugin);
}

void PluginManagerDialog::onPluginInstalled()
{
    ui->btnInstallPlugin->setEnabled(true);
    ui->btnUpdatePlugin->setEnabled(true);
    ui->msgLabel->setText(tr("Plugin was installed successfully."));
    reject();
}

void PluginManagerDialog::onPluginUpdated()
{
    ui->btnInstallPlugin->setEnabled(true);
    ui->btnUpdatePlugin->setEnabled(true);
    ui->msgLabel->setText(tr("Plugin was updated successfully."));
    reject();
}

void PluginManagerDialog::onPluginFailure()
{
    ui->btnInstallPlugin->setEnabled(true);
    ui->btnUpdatePlugin->setEnabled(true);
    ui->msgLabel->setText(tr("An error occured while installing this plugin."));
}
