#pragma once

#include "renamer/Renamer.h"

#include "RenamerPlaceholders.h"
#include "globals/Helper.h"
#include "media/StreamDetails.h"

class RenamerDialog;
class Movie;

namespace mediaelch {

class MovieRenamerPlaceholders : public RenamerPlaceholders
{
public:
    ~MovieRenamerPlaceholders() override;

    QVector<Placeholder> placeholders() override;
};

class MovieRenamerData : public RenamerData
{
public:
    explicit MovieRenamerData(Movie& movie) : m_movie(movie) {}
    ~MovieRenamerData() override;

    void setPartNo(int partNo) { m_partNo = partNo; }
    void setExtension(QString extension) { m_extension = extension; }
    void setIsBluRay(bool isBluRay) { m_isBluRay = isBluRay; }
    void setIsDvd(bool isDvd) { m_isDvd = isDvd; }
    void setVideoDetails(QMap<StreamDetails::VideoDetails, QString> videoDetails) { m_videoDetails = videoDetails; }

    ELCH_NODISCARD QString value(const QString& name) const override;
    ELCH_NODISCARD bool passesCondition(const QString& name) const override;

private:
    Movie& m_movie;
    QString m_extension;
    QMap<StreamDetails::VideoDetails, QString> m_videoDetails;
    int m_partNo{0};
    bool m_isBluRay{false};
    bool m_isDvd{false};
};

} // namespace mediaelch

class MovieRenamer : public Renamer
{
public:
    MovieRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog);
    RenameError renameMovie(Movie& movie);
};
