#include "ui/settings/KodiSettingsWidget.h"
#include "ui_KodiSettingsWidget.h"

#include "settings/Settings.h"

#include <QIntValidator>

using namespace mediaelch;

KodiSettingsWidget::KodiSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::KodiSettingsWidget)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    QFont smallFont = ui->lblWebserverDescription->font();
    smallFont.setPointSize(smallFont.pointSize() - 1);
    ui->lblWebserverDescription->setFont(smallFont);
#endif

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));

    for (const auto& version : KodiVersion::all()) {
        const int v = version.toInt();
        ui->kodiVersion->addItem(QString("v%1").arg(v), version.toInt());
    }
}

KodiSettingsWidget::~KodiSettingsWidget()
{
    delete ui;
}

void KodiSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void KodiSettingsWidget::loadSettings()
{
    ui->xbmcHost->setText(m_settings->kodiSettings().xbmcHost());
    if (m_settings->kodiSettings().xbmcPort() != 0) {
        ui->xbmcPort->setText(QString::number(m_settings->kodiSettings().xbmcPort()));
    } else {
        ui->xbmcPort->clear();
    }
    ui->xbmcUser->setText(m_settings->kodiSettings().xbmcUser());
    ui->xbmcPassword->setText(m_settings->kodiSettings().xbmcPassword());

    const int version = m_settings->kodiSettings().kodiVersion().toInt();
    const int index = ui->kodiVersion->findData(version);
    if (index != -1) {
        ui->kodiVersion->setCurrentIndex(index);
    } else {
        qWarning() << "The GUI doesn't provide an entry for" << version << "; this is a bug";
    }
}

void KodiSettingsWidget::saveSettings()
{
    m_settings->kodiSettings().setXbmcHost(ui->xbmcHost->text());
    m_settings->kodiSettings().setXbmcPort(ui->xbmcPort->text().toInt());
    m_settings->kodiSettings().setXbmcUser(ui->xbmcUser->text());
    m_settings->kodiSettings().setXbmcPassword(ui->xbmcPassword->text());
    const int version = ui->kodiVersion->currentData().toInt();
    m_settings->kodiSettings().setKodiVersion(KodiVersion(version));
    if (!KodiVersion::isValid(version)) {
        qWarning() << "Selected invalid Kodi version. The GUI shouldn't allow that.";
    }
}
