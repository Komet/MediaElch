#ifndef PLUGINSWIDGET_H
#define PLUGINSWIDGET_H

#include <QWidget>

#include "plugins/PluginManager.h"

namespace Ui {
class PluginsWidget;
}

class PluginsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PluginsWidget(QWidget *parent = nullptr);
    ~PluginsWidget();

    void setPlugin(PluginManager::Plugin plugin);

private:
    Ui::PluginsWidget *ui;
};

#endif // PLUGINSWIDGET_H
