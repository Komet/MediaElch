#include "ui/settings/NetworkSettingsWidget.h"
#include "ui_NetworkSettingsWidget.h"

#include "settings/Settings.h"

#include <QFileDialog>

NetworkSettingsWidget::NetworkSettingsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::NetworkSettingsWidget)
{
    ui->setupUi(this);

    connect(ui->chkUseProxy, &QAbstractButton::clicked, this, &NetworkSettingsWidget::onUseProxy);
}

NetworkSettingsWidget::~NetworkSettingsWidget()
{
    delete ui;
}

void NetworkSettingsWidget::setSettings(Settings& settings)
{
    m_settings = &settings;
}

void NetworkSettingsWidget::loadSettings()
{
    const auto& netSettings = m_settings->networkSettings();
    ui->chkUseProxy->setChecked(netSettings.useProxy());
    ui->chkUseProxyForKodi->setChecked(netSettings.useProxyForKodi());
    ui->proxyType->setCurrentIndex(netSettings.proxyType());
    ui->proxyHost->setText(netSettings.proxyHost());
    ui->proxyPort->setValue(netSettings.proxyPort());
    ui->proxyUsername->setText(netSettings.proxyUsername());
    ui->proxyPassword->setText(netSettings.proxyPassword());
    onUseProxy();
}

void NetworkSettingsWidget::saveSettings()
{
    m_settings->networkSettings().setUseProxy(ui->chkUseProxy->isChecked());
    m_settings->networkSettings().setUseProxyForKodi(ui->chkUseProxyForKodi->isChecked());
    m_settings->networkSettings().setProxyType(ui->proxyType->currentIndex());
    m_settings->networkSettings().setProxyHost(ui->proxyHost->text());
    // Explicitly check that we are in the allowed range of 2^16
    if (ui->proxyPort->value() > 0 && ui->proxyPort->value() < (1 << 16)) {
        m_settings->networkSettings().setProxyPort(static_cast<uint16_t>(ui->proxyPort->value()));
    }
    m_settings->networkSettings().setProxyUsername(ui->proxyUsername->text());
    m_settings->networkSettings().setProxyPassword(ui->proxyPassword->text());
}

void NetworkSettingsWidget::onUseProxy()
{
    const bool enabled = ui->chkUseProxy->isChecked();
    ui->chkUseProxyForKodi->setEnabled(enabled);
    ui->proxyType->setEnabled(enabled);
    ui->proxyHost->setEnabled(enabled);
    ui->proxyPort->setEnabled(enabled);
    ui->proxyUsername->setEnabled(enabled);
    ui->proxyPassword->setEnabled(enabled);
}
