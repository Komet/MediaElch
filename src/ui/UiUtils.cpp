#include "ui/UiUtils.h"

#include "globals/Helper.h"

namespace mediaelch {
namespace ui {

void fillStereoModeCombo(QComboBox* box)
{
    bool blocked = box->blockSignals(true);
    box->clear();
    box->addItem("", "");
    QMap<QString, QString> modes = helper::stereoModes();
    QMapIterator<QString, QString> it(modes);
    while (it.hasNext()) {
        it.next();
        box->addItem(it.value(), it.key());
    }
    box->blockSignals(blocked);
}

} // namespace ui
} // namespace mediaelch
