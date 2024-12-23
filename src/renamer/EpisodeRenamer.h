#pragma once
#include "renamer/Renamer.h"

#include "RenamerPlaceholders.h"
#include "globals/Helper.h"
#include "media/StreamDetails.h"

class RenamerDialog;
class TvShowEpisode;
class TvShow;

namespace mediaelch {

class TvShowRenamerPlaceholders : public RenamerPlaceholders
{
public:
    ~TvShowRenamerPlaceholders() override;

    QVector<Placeholder> placeholders() override;
};

class EpisodeRenamerPlaceholders : public RenamerPlaceholders
{
public:
    ~EpisodeRenamerPlaceholders() override;

    QVector<Placeholder> placeholders() override;
};

class TvShowRenamerData : public RenamerData
{
public:
    explicit TvShowRenamerData(TvShow& tvShow) : m_tvShow(tvShow) {}
    ~TvShowRenamerData() override;

    ELCH_NODISCARD QString value(const QString& name) const override;
    ELCH_NODISCARD bool passesCondition(const QString& name) const override;

private:
    TvShow& m_tvShow;
};

class EpisodeRenamerData : public RenamerData
{
public:
    explicit EpisodeRenamerData(TvShowEpisode& episode) : m_episode(episode) {}
    ~EpisodeRenamerData() override;

    void setPartNo(int partNo) { m_partNo = partNo; }
    void setExtension(QString extension) { m_extension = extension; }
    void setIsBluRay(bool isBluRay) { m_isBluRay = isBluRay; }
    void setIsDvd(bool isDvd) { m_isDvd = isDvd; }
    void setMultiEpisodes(QVector<TvShowEpisode*> multiEpisodes);
    void setVideoDetails(QMap<StreamDetails::VideoDetails, QString> videoDetails) { m_videoDetails = videoDetails; }

    ELCH_NODISCARD QString value(const QString& name) const override;
    ELCH_NODISCARD bool passesCondition(const QString& name) const override;

private:
    TvShowEpisode& m_episode;
    QString m_extension;
    QString m_episodeString;
    QMap<StreamDetails::VideoDetails, QString> m_videoDetails;
    int m_partNo{0};
    bool m_isBluRay{false};
    bool m_isDvd{false};
};

} // namespace mediaelch


class EpisodeRenamer : public Renamer
{
public:
    EpisodeRenamer(RenamerConfig renamerConfig, RenamerDialog* dialog);
    RenameError renameEpisode(TvShowEpisode& episode, QVector<TvShowEpisode*>& episodesRenamed);
    RenameError renameTvShow(TvShow& tvShow);
};
