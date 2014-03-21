#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QIcon>
#include <QSettings>
#include <QString>
#include <QWidget>

class PluginInterface
{

public:
    enum Actions {
        ActionSearch, ActionSave, ActionSaveAll, ActionReload, ActionRename
    };

    enum Section {
        SectionMovies, SectionTvShows, SectionConcerts, SectionImport, SectionDefault
    };

    virtual ~PluginInterface() {}

    virtual void init(void *movieModel, void *tvShowModel, void *concertModel, void *notificationBox, void *helper) = 0;

    virtual PluginInterface::Section section() = 0;
    virtual QString name() const = 0;
    virtual QString identifier() const = 0;
    virtual QString version() const = 0;
    virtual QString minimumVersion() const = 0;

    virtual QIcon menuIcon() = 0;

    virtual QList<PluginInterface::Actions> enabledActions() = 0;
    virtual void doAction(PluginInterface::Actions action) = 0;

    virtual bool hasSettings() = 0;
    virtual QWidget *settingsWidget() = 0;
    virtual QWidget *widget() = 0;

public slots:
    virtual void saveSettings(QSettings *settings) = 0;
    virtual void loadSettings(QSettings *settings) = 0;
};

#define PluginInterface_iid "com.kvibes.MediaElch.PluginInterface/1.0"
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H
