#pragma once

#include "media_center/KodiVersion.h"
#include "settings/Settings.h"

#include <QObject>
#include <QString>

namespace mediaelch {

class KodiSettings : public QObject
{
    Q_OBJECT
public:
    explicit KodiSettings(Settings& settings, QObject* parent = nullptr);
    ~KodiSettings() override = default;

    void init();

    ELCH_NODISCARD QString xbmcHost();
    void setXbmcHost(QString host);

    ELCH_NODISCARD int xbmcPort();
    void setXbmcPort(int port);

    ELCH_NODISCARD QString xbmcUser();
    void setXbmcUser(QString xbmcUser);

    ELCH_NODISCARD QString xbmcPassword();
    void setXbmcPassword(QString password);

    ELCH_NODISCARD mediaelch::KodiVersion kodiVersion();
    void setKodiVersion(mediaelch::KodiVersion version);

signals:
    void xbmcHostChanged(QString host);
    void xbmcPortChanged(int port);
    void xbmcUserChanged(QString xbmcUser);
    void xbmcPasswordChanged(QString password);
    void kodiVersionChanged(mediaelch::KodiVersion version);

private:
    Settings& m_settings;
};

} // namespace mediaelch
