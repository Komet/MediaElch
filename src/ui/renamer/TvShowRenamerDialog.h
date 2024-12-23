#pragma once

#include "data/tv_show/TvShow.h"
#include "data/tv_show/TvShowEpisode.h"
#include "ui/renamer/RenamerDialog.h"

class TvShowRenamerDialog final : public RenamerDialog
{
    Q_OBJECT

public:
    explicit TvShowRenamerDialog(QWidget* parent = nullptr);
    ~TvShowRenamerDialog() override;

    void setShows(QVector<TvShow*> shows);
    void setEpisodes(QVector<TvShowEpisode*> episodes);

private:
    void renameType(bool isDryRun) override;
    void rejectImpl() override;
    QString dialogInfoLabel() override;
    void initPlaceholders() override;

    void renameEpisodes(QVector<TvShowEpisode*> episodes, const RenamerConfig& config);
    void renameTvShows(const QVector<TvShow*>& shows, const RenamerConfig& config);

    QStringList fileNameDefaults() override;
    QStringList fileNameMultiDefaults() override;
    QStringList directoryNameDefaults() override;

private:
    QVector<TvShow*> m_shows;
    QVector<TvShowEpisode*> m_episodes;
};
