#pragma once

#include <QComboBox>

/// \brief A combo box that lists HDR types.
class HdrTypeComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit HdrTypeComboBox(QWidget* parent = nullptr);
    ~HdrTypeComboBox() override = default;
};
