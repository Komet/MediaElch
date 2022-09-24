#pragma once

// Utils for MediaElch's UI.
// TODO: We should try to move these functions to better places.
//       For example create a custom combobox instead of fillStereoModeCombo().

#include <QComboBox>
#include <QWidget>

namespace mediaelch {
namespace ui {

void fillStereoModeCombo(QComboBox* box);

void removeFocusRect(QWidget* widget);

void applyStyle(QWidget* widget, bool removeFocus = true, bool isTable = false);
void applyEffect(QWidget* parent);

} // namespace ui
} // namespace mediaelch
