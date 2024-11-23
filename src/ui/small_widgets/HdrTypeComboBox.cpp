#include "ui/small_widgets/HdrTypeComboBox.h"

#include "media/StreamDetails.h"

HdrTypeComboBox::HdrTypeComboBox(QWidget* parent) : QComboBox(parent)
{
    blockSignals(true);

    addItem("", "");
    auto types = StreamDetails::hdrTypes();
    for (QString& type : types) {
        addItem(type, type);
    }
    blockSignals(false);
}
