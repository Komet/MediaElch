#include "tv_shows/TvMazeId.h"

#include <QRegularExpression>
#include <QString>
#include <utility>

TvMazeId::TvMazeId(QString tvmazeId) : m_tvmazeId{tvmazeId}
{
}

TvMazeId::TvMazeId(int tvmazeId)
{
    if (tvmazeId > 0) { // id 0 is not valid
        m_tvmazeId = QString::number(tvmazeId);
    }
}

const TvMazeId TvMazeId::NoId = TvMazeId();

bool TvMazeId::operator==(const TvMazeId& other) const
{
    // Do not care whether the ID is valid.
    return m_tvmazeId == other.m_tvmazeId;
}

bool TvMazeId::operator!=(const TvMazeId& other) const
{
    return !(*this == other);
}

QString TvMazeId::toString() const
{
    return m_tvmazeId;
}

bool TvMazeId::isValid() const
{
    return TvMazeId::isValidFormat(m_tvmazeId);
}

bool TvMazeId::isValidFormat(const QString& tvmazeId)
{
    QRegularExpression rx("^\\d+$");
    return rx.match(tvmazeId).hasMatch();
}

std::ostream& operator<<(std::ostream& os, const TvMazeId& id)
{
    return os << id.toString().toStdString();
}

QDebug operator<<(QDebug debug, const TvMazeId& id)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "TvMazeId(" << id.toString() << ')';
    return debug;
}
