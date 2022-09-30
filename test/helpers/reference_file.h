#pragma once

#include <QString>

class Concert;
class Movie;

namespace test {

QString serializeForReference(const Concert& concert);
QString serializeForReference(const Movie& concert);

} // namespace test
