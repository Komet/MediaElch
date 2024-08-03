#pragma once

#include "settings/Settings.h"

#include <QObject>
#include <QString>

class ImportSettings : public QObject
{
    Q_OBJECT
public:
    explicit ImportSettings(Settings& settings, QObject* parent = nullptr);
    ~ImportSettings() override;

    void init();

    ELCH_NODISCARD QString unrar() const;
    void setUnrar(QString unrar);

    ELCH_NODISCARD QString makeMkvCon() const;
    void setMakeMkvCon(QString makeMkvCon);

    ELCH_NODISCARD bool deleteArchives() const;
    void setDeleteArchives(bool deleteArchives);

private:
    Settings& m_settings;
};
