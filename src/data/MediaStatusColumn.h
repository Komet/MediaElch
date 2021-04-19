#pragma once

#include "globals/Meta.h"

#include <QHash>
#include <QIcon>
#include <QString>

enum class MediaStatusState : int
{
    GREEN,
    RED,
    YELLOW
};

QIcon iconWithMediaStatusColor(const QString& iconName, MediaStatusState state);

constexpr inline ELCH_QHASH_RETURN_TYPE qHash(const MediaStatusState& key, uint seed = 0) noexcept
{
    return static_cast<uint>(key) ^ seed;
}
