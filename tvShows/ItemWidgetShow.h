#ifndef ITEMWIDGETSHOW_H
#define ITEMWIDGETSHOW_H

#include <QWidget>

namespace Ui {
class ItemWidgetShow;
}

class ItemWidgetShow : public QWidget
{
    Q_OBJECT

public:
    explicit ItemWidgetShow(QWidget *parent = 0);
    ~ItemWidgetShow();
    void setTitle(const QString &title);
    void setEpisodeCount(const int &episodeCount);
    void setNew(const bool &isNew);
    void setSyncNeeded(const bool &syncNeeded);

    void setHasPoster(const bool &has);
    void setHasFanart(const bool &has);
    void setHasExtraFanart(const bool &has);
    void setHasLogo(const bool &has);
    void setHasClearArt(const bool &has);
    void setHasCharacterArt(const bool &has);
    void setHasBanner(const bool &has);
    void setHasThumb(const bool &has);
    void setMissingEpisodes(const bool &missing);

private:
    Ui::ItemWidgetShow *ui;
};

#endif // ITEMWIDGETSHOW_H
