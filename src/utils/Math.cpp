#include "utils/Meta.h"

#include <QtGlobal>

namespace mediaelch {
namespace math {

int clamp(int min, int max, int value)
{
    return qMax(min, qMin(max, value));
}

} // namespace math
} // namespace mediaelch
