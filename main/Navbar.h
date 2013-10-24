#ifndef NAVBAR_H
#define NAVBAR_H

#include <QWidget>

namespace Ui {
class Navbar;
}

class Navbar : public QWidget
{
    Q_OBJECT

public:
    explicit Navbar(QWidget *parent = 0);
    ~Navbar();
    void setActionSearchEnabled(bool enabled);
    void setActionSaveEnabled(bool enabled);
    void setActionSaveAllEnabled(bool enabled);
    void setActionReloadEnabled(bool enabled);
    void setActionRenameEnabled(bool enabled);
    void setReloadToolTip(QString toolTip);

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

private:
    Ui::Navbar *ui;
};

#endif // NAVBAR_H
