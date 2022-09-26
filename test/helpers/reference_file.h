#pragma once

#include <QString>

class Concert;
class Movie;

namespace test {

QString serializeForReference(const Concert& concert);

}
