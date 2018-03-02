#include "PluginsWidget.h"
#include "ui_PluginsWidget.h"

PluginsWidget::PluginsWidget(QWidget *parent) : QWidget(parent), ui(new Ui::PluginsWidget)
{
    ui->setupUi(this);
    ui->installed->setBadgeType(Badge::LabelSuccess);
    ui->updateAvailable->setBadgeType(Badge::LabelInfo);
}

PluginsWidget::~PluginsWidget()
{
    delete ui;
}

void PluginsWidget::setPlugin(PluginManager::Plugin plugin)
{
    ui->name->setText(plugin.name);
    ui->version->setText(plugin.installed ? plugin.installedVersion : plugin.version);
    ui->installed->setVisible(plugin.installed);
    ui->updateAvailable->setVisible(plugin.updateAvailable);
}
