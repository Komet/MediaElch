#pragma once

#include <QString>

class Album;
class Artist;
class Concert;
class Movie;

namespace test {

QString serializeForReference(const Album& album);
QString serializeForReference(const Artist& album);
QString serializeForReference(const Concert& concert);
QString serializeForReference(const Movie& concert);

} // namespace test
