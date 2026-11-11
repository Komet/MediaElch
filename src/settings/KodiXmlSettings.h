#pragma once

#include "media/Path.h"
#include "media_center/KodiVersion.h"
#include "settings/DataFile.h"

#include <QVector>

class Settings;
namespace mediaelch {

class KodiXmlSettings
{
public:
    explicit KodiXmlSettings(Settings& settings);

    void init();

    mediaelch::KodiVersion kodiVersion();
    bool writeThumbUrlsToNfo();
    bool useFirstStudioOnly();
    bool ignoreDuplicateOriginalTitle();
    bool usePlotForOutline();
    QVector<DataFile> dataFiles(DataFileType type);
    QVector<DataFile> dataFiles(ImageType type);
    MovieSetArtworkType movieSetArtworkType();
    mediaelch::DirectoryPath movieSetArtworkDirectory();

private:
    Settings& m_settings;
};

} // namespace mediaelch
