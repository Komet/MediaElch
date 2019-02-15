#pragma once

#include "globals/Filter.h"
#include "globals/Globals.h"

#include <QWidget>

namespace Ui {
class Navbar;
}

class Navbar : public QWidget
{
    Q_OBJECT

public:
    explicit Navbar(QWidget* parent = nullptr);
    ~Navbar() override;
    void setActionSearchEnabled(bool enabled);
    void setActionSaveEnabled(bool enabled);
    void setActionSaveAllEnabled(bool enabled);
    void setActionReloadEnabled(bool enabled);
    void setActionRenameEnabled(bool enabled);
    void setReloadToolTip(QString toolTip);

public slots:
    void setActiveWidget(MainWidgets widget);
    void setFilterWidgetEnabled(bool enabled);

signals:
    void sigSearch();
    void sigSave();
    void sigSaveAll();
    void sigReload();
    void sigRename();
    void sigSettings();
    void sigSync();
    void sigExport();
    void sigAbout();
    void sigLike();
    void sigFilterChanged(QVector<Filter*>, QString);

private slots:
    void onDonated(bool donated);

private:
    Ui::Navbar* ui;
};
