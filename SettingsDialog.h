#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    static SettingsDialog* instance(QWidget *parent = 0);
    void accept();

    QSize mainWindowSize();
    QPoint mainWindowPosition();
    QStringList movieDirectories();
    bool firstTime();

    void setMainWindowSize(QSize mainWindowSize);
    void setMainWindowPosition(QPoint mainWindowPosition);
    
public slots:
    int exec();

private slots:
    void addMovieDir();
    void removeMovieDir();
    void movieListRowChanged(int currentRow);

private:
    Ui::SettingsDialog *ui;

    QSettings m_settings;
    QStringList m_movieDirectories;
    QSize m_mainWindowSize;
    QPoint m_mainWindowPosition;
    bool m_firstTime;

    void loadSettings();
};

#endif // SETTINGSDIALOG_H
