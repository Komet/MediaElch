#ifndef ARTIST_H
#define ARTIST_H

#include <QObject>
#include "globals/Globals.h"
#include "MusicModelItem.h"

class MusicModelItem;

class Artist : public QObject
{
    Q_OBJECT
public:
    explicit Artist(QString path, QObject *parent = 0);
    ~Artist();

    QString path() const;
    void setPath(const QString &path);

    QString name() const;
    void setName(const QString &name);

    QStringList genres() const;
    void setGenres(const QStringList &genres);
    void addGenre(const QString &genre);
    void removeGenre(const QString &genre);

    QStringList styles() const;
    void setStyles(const QStringList &styles);
    void addStyle(const QString &style);
    void removeStyle(const QString &style);

    QStringList moods() const;
    void setMoods(const QStringList &moods);
    void addMood(const QString &mood);
    void removeMood(const QString &mood);

    QString yearsActive() const;
    void setYearsActive(const QString &yearsActive);

    QString formed() const;
    void setFormed(const QString &formed);

    QString born() const;
    void setBorn(const QString &born);

    QString died() const;
    void setDied(const QString &died);

    QString disbanded() const;
    void setDisbanded(const QString &disbanded);

    QString biography() const;
    void setBiography(const QString &biography);

    QList<Poster> images(int imageType) const;
    void addImage(int imageType, Poster image);

    QByteArray rawImage(int imageType);
    void setRawImage(int imageType, QByteArray image);
    void removeRawImage(int imageType);
    void clearImages();

    bool hasChanged() const;
    void setHasChanged(bool hasChanged);

    void clear();
    void clear(QList<int> infos);

    MusicModelItem *modelItem() const;
    void setModelItem(MusicModelItem *modelItem);

    QString nfoContent() const;
    void setNfoContent(const QString &nfoContent);

    static QList<int> imageTypes();

    QList<int> imagesToRemove() const;
    void setImagesToRemove(const QList<int> &imagesToRemove);

signals:
    void sigChanged(Artist*);

private:
    QString m_path;
    QString m_name;
    QStringList m_genres;
    QStringList m_styles;
    QStringList m_moods;
    QString m_yearsActive;
    QString m_formed;
    QString m_biography;
    QString m_born;
    QString m_died;
    QString m_disbanded;
    bool m_hasChanged;
    QMap<int, QList<Poster> > m_images;
    QMap<int, QByteArray> m_rawImages;
    QList<int> m_imagesToRemove;
    MusicModelItem *m_modelItem;
    QString m_nfoContent;
};

#endif // ARTIST_H
