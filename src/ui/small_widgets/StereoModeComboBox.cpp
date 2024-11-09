#include "ui/small_widgets/StereoModeComboBox.h"

#include "media/StreamDetails.h"

StereoModeComboBox::StereoModeComboBox(QWidget* parent) : QComboBox(parent)
{
    blockSignals(true);

    addItem("", "");
    QMap<QString, QString> modes = StreamDetails::stereoModes();
    QMapIterator<QString, QString> it(modes);
    while (it.hasNext()) {
        it.next();
        addItem(it.value(), it.key());
    }

    blockSignals(false);
}
