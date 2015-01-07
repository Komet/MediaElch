/*
    Copyright 2012-2014 Daniel Kabel.
    Copyright 2014 Udo Schläpfer.

    This file is part of MediaElch.

    MediaElch is free software: you can redistribute it and/or modify it under the terms of
    the GNU Lesser General Public License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    MediaElch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
    without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
    the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with MediaElch.
    If not, see <http://www.gnu.org/licenses/>.
*/

#include "globals/Globals.h"
#include <QDebug>

QDebug operator<<(QDebug lhs, const ScraperSearchResult& rhs)
{
    lhs.nospace() << "(id: "      << rhs.id                        << ", "
                  << "name: "     << rhs.name                      << ", "
                  << "released: " << rhs.released.toString("yyyy") << ")";
    return lhs;
}
