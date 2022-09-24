#pragma once

// Utils for MediaElch's UI.
// TODO: We should try to move these functions to better places.
//       For example create a custom combobox instead of fillStereoModeCombo().

#include <QComboBox>

namespace mediaelch {
namespace ui {

void fillStereoModeCombo(QComboBox* box);

}
} // namespace mediaelch
