#pragma once

#include "data/Locale.h"

#include <QComboBox>

namespace Ui {
class LanguageCombo;
}

class LanguageCombo : public QComboBox
{
    Q_OBJECT

public:
    explicit LanguageCombo(QWidget* parent = nullptr);
    ~LanguageCombo() override = default;

signals:
    void languageChanged();

public:
    /// \brief   Setup the language dropdown with a pre-selected entry.
    /// \details This function does not emit any signals while adding entries.
    void setupLanguages(const QVector<mediaelch::Locale>& locales, const mediaelch::Locale& selected);

    /// \brief Special case if no languages can be added. Show invalid state.
    void setInvalid();
    /// \brief True, if the combo box's current index is a valid locale.
    bool isCurrentValid();

    mediaelch::Locale currentLocale();

private:
    mediaelch::Locale localeAt(int index);

private slots:
    void onIndexChanged(int index);
};
