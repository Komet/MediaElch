#pragma once

#include "globals/Globals.h"
#include "tv_shows/SeasonNumber.h"

#include <QObject>

class TvShow;
class TvShowModelItem;

class TvShowBaseModelItem : public QObject
{
    Q_OBJECT

public:
    explicit TvShowBaseModelItem() : QObject(nullptr) {}
    virtual ~TvShowBaseModelItem() = default;

    virtual QVariant data(int column) const = 0;
    virtual TvShowBaseModelItem* child(int index) const = 0;
    virtual TvShowBaseModelItem* parent() const = 0;
    virtual int indexInParent() const = 0;

    /// Get the number of children
    virtual int childCount() const = 0;
    virtual bool removeChildren(int position, int rows) = 0;

    virtual const TvShow* tvShow() const = 0;
    virtual TvShow* tvShow() = 0;

    virtual TvShowType type() const = 0;
};
