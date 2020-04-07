#pragma once

#include "scrapers/movie/tmdb/TmdbMovie.h"

#include <QObject>

namespace mediaelch {
namespace settings {

class ScraperSettingsAdaptor : public QObject {
    Q_OBJECT
public:
    explicit ScraperSettingsAdaptor(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ScraperSettingsAdaptor() = default;

public slots:
    virtual void loadSettings(QSettings& settingsStore) = 0;
    virtual void saveSettings(QSettings& settingsStore) = 0;
};

} // namespace settings
}
