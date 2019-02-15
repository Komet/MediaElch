#pragma once

#include <QSettings>
#include <QString>

class ImportSettings
{
public:
    void loadSettings();
    void saveSettings();
    void setQSettings(QSettings* settings) { m_settings = settings; }

    QString unrar() const;
    QString makeMkvCon() const;

    void setUnrar(QString unrar);
    void setMakeMkvCon(QString makeMkvCon);

private:
    QSettings* m_settings = nullptr;

    QString m_unrar;
    QString m_makeMkvCon;
};
