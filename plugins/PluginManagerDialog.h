#ifndef PLUGINMANAGERDIALOG_H
#define PLUGINMANAGERDIALOG_H

#include <QDialog>
#include "plugins/PluginManager.h"

namespace Ui {
class PluginManagerDialog;
}

class PluginManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PluginManagerDialog(QWidget *parent = nullptr);
    ~PluginManagerDialog();

    void installPlugin(PluginManager::Plugin plugin);
    void updatePlugin(PluginManager::Plugin plugin);

public slots:
    int exec();

private slots:
    void onInstallPlugin();
    void onUpdatePlugin();
    void onPluginInstalled();
    void onPluginUpdated();
    void onPluginFailure();

private:
    Ui::PluginManagerDialog *ui;
    PluginManager::Plugin m_plugin;
};

#endif // PLUGINMANAGERDIALOG_H
