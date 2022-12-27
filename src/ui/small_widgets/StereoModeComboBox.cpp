#include "ui/small_widgets/StereoModeComboBox.h"

#include "globals/Helper.h"

StereoModeComboBox::StereoModeComboBox(QWidget* parent) : QComboBox(parent)
{
    blockSignals(true);

    addItem("", "");
    QMap<QString, QString> modes = helper::stereoModes();
    QMapIterator<QString, QString> it(modes);
    while (it.hasNext()) {
        it.next();
        addItem(it.value(), it.key());
    }

    blockSignals(false);
}
