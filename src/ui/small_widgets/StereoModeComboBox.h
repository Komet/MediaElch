#pragma once

#include <QComboBox>

/// \brief A combo box that lists stereo modes.
class StereoModeComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit StereoModeComboBox(QWidget* parent = nullptr);
    ~StereoModeComboBox() override = default;
};
