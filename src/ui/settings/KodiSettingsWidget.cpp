#include "ui/settings/KodiSettingsWidget.h"
#include "ui_KodiSettingsWidget.h"

#include <QIntValidator>

using namespace mediaelch;

KodiSettingsWidget::KodiSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::KodiSettingsWidget)
{
    ui->setupUi(this);

    ui->xbmcPort->setValidator(new QIntValidator(0, 99999, ui->xbmcPort));
}

void KodiSettingsWidget::init(mediaelch::KodiSettings* settings)
{
    ui->xbmcHost->setText(settings->xbmcHost());
    connect(ui->xbmcHost, &QLineEdit::textChanged, this, [settings](QString host) { settings->setXbmcHost(host); });
    connect(settings, &mediaelch::KodiSettings::xbmcHostChanged, this, [this](QString host) {
        const bool blocked = ui->xbmcHost->blockSignals(true); // avoid triggering save-logic
        ui->xbmcHost->setText(host);
        ui->xbmcHost->blockSignals(blocked);
    });


    ui->xbmcPort->setText(QString::number(settings->xbmcPort()));
    connect(ui->xbmcPort, &QLineEdit::textChanged, this, [settings](QString portStr) {
        bool ok = false;
        int port = portStr.toInt(&ok);
        if (ok) {
            settings->setXbmcPort(port);
        }
        // TODO: handle error state
    });
    connect(settings, &mediaelch::KodiSettings::xbmcPortChanged, this, [this](int port) {
        const bool blocked = ui->xbmcPort->blockSignals(true); // avoid triggering save-logic
        ui->xbmcPort->setText(QString::number(port));
        ui->xbmcPort->blockSignals(blocked);
    });


    ui->xbmcUser->setText(settings->xbmcUser());
    connect(ui->xbmcUser, &QLineEdit::textChanged, this, [settings](QString user) { settings->setXbmcUser(user); });
    connect(settings, &mediaelch::KodiSettings::xbmcUserChanged, this, [this](QString user) {
        const bool blocked = ui->xbmcUser->blockSignals(true); // avoid triggering save-logic
        ui->xbmcUser->setText(user);
        ui->xbmcUser->blockSignals(blocked);
    });


    ui->xbmcPassword->setText(settings->xbmcPassword());
    connect(ui->xbmcPassword, &QLineEdit::textChanged, this, [settings](QString password) {
        settings->setXbmcPassword(password);
    });
    connect(settings, &mediaelch::KodiSettings::xbmcPasswordChanged, this, [this](QString password) {
        const bool blocked = ui->xbmcPassword->blockSignals(true); // avoid triggering save-logic
        ui->xbmcPassword->setText(password);
        ui->xbmcPassword->blockSignals(blocked);
    });


    const auto& allVersions = mediaelch::KodiVersion::all();
    for (const auto& version : allVersions) {
        const int v = version.toInt();
        ui->kodiVersion->addItem(QString("v%1").arg(v), version.toInt());
    }

    setKodiVersion(settings->kodiVersion());
    connect(ui->kodiVersion, &QComboBox::activated, this, [this, settings]() {
        const int version = ui->kodiVersion->currentData().toInt();
        settings->setKodiVersion(KodiVersion(version));
    });
    connect(settings, &mediaelch::KodiSettings::kodiVersionChanged, this, [this](KodiVersion kodiVersion) {
        const bool blocked = ui->kodiVersion->blockSignals(true); // avoid triggering save-logic or infinite loop
        setKodiVersion(kodiVersion);
        ui->kodiVersion->blockSignals(blocked);
    });
}

KodiSettingsWidget::~KodiSettingsWidget()
{
    delete ui;
}

void KodiSettingsWidget::setKodiVersion(mediaelch::KodiVersion kodiVersion)
{
    const int version = kodiVersion.toInt();
    if (KodiVersion::isValid(version)) {
        const int index = ui->kodiVersion->findData(version);
        if (index != -1) {
            ui->kodiVersion->setCurrentIndex(index);
        }
    }
}
