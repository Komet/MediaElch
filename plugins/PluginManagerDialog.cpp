#include "PluginManagerDialog.h"
#include "ui_PluginManagerDialog.h"

#include "settings/Settings.h"

PluginManagerDialog::PluginManagerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginManagerDialog)
{
    ui->setupUi(this);

    connect(ui->btnInstallPlugin, SIGNAL(clicked()), this, SLOT(onInstallPlugin()));
    connect(ui->btnUpdatePlugin, SIGNAL(clicked()), this, SLOT(onUpdatePlugin()));
    connect(PluginManager::instance(), SIGNAL(sigLicenseInvalid(PluginManager::Plugin)), this, SLOT(onInvalidLicense()));
    connect(PluginManager::instance(), SIGNAL(sigPluginInstalled(PluginManager::Plugin)), this, SLOT(onPluginInstalled()));
    connect(PluginManager::instance(), SIGNAL(sigPluginUpdated(PluginManager::Plugin)), this, SLOT(onPluginUpdated()));
    connect(PluginManager::instance(), SIGNAL(sigPluginInstallFailure(PluginManager::Plugin)), this, SLOT(onPluginFailure()));
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
    ui->licenseKey->setText("");
    ui->btnInstallPlugin->setVisible(true);
    ui->btnUpdatePlugin->setVisible(false);
    exec();
}

void PluginManagerDialog::updatePlugin(PluginManager::Plugin plugin)
{
    m_plugin = plugin;
    ui->btnInstallPlugin->setVisible(false);
    ui->btnUpdatePlugin->setVisible(true);
    ui->licenseKey->setText(Settings::instance()->licenseKey(plugin));
    exec();
}

void PluginManagerDialog::onInstallPlugin()
{
    ui->btnInstallPlugin->setEnabled(false);
    ui->msgLabel->setText(tr("Plugin is being installed..."));
    Settings::instance()->setLicenseKey(m_plugin, ui->licenseKey->text());
    PluginManager::instance()->installPlugin(m_plugin, ui->licenseKey->text());
}

void PluginManagerDialog::onUpdatePlugin()
{
    ui->btnUpdatePlugin->setEnabled(false);
    ui->msgLabel->setText(tr("Plugin is being updated..."));
    Settings::instance()->setLicenseKey(m_plugin, ui->licenseKey->text());
    PluginManager::instance()->updatePlugin(m_plugin, ui->licenseKey->text());
}

void PluginManagerDialog::onInvalidLicense()
{
    ui->btnInstallPlugin->setEnabled(true);
    ui->btnUpdatePlugin->setEnabled(true);
    ui->msgLabel->setText(tr("Your license key is invalid."));
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
