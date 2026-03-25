#include "Update.h"

#include <QCheckBox>
#include <QMessageBox>
#include <QUrl>

#include "globals/Helper.h"
#include "log/Log.h"
#include "settings/Settings.h"
#include "settings/UpdateCheck.h"

Update::Update(QObject* parent) : QObject(parent)
{
    connect(&m_updateCheck, &mediaelch::UpdateCheck::updateCheckFinished, this, &Update::onUpdateCheckFinished);
}

Update* Update::instance(QObject* parent)
{
    static Update* m_instance = nullptr;
    if (m_instance == nullptr) {
        m_instance = new Update(parent);
    }
    return m_instance;
}

void Update::checkForUpdate()
{
    m_updateCheck.checkForUpdate();
}

void Update::onUpdateCheckFinished(mediaelch::UpdateCheck::Result result)
{
    if (!result.isNewVersionAvailable) {
        return; // nothing to do
    }
    const QString downloadLink = QStringLiteral("<a href=\"%1\">https://mediaelch.github.io</a>")
                                     .arg(result.downloadUrl.toString(QUrl::FullyEncoded));

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(tr("Updates available"));
    msgBox.setText(tr("%1 is now available.<br>Get it now on %2").arg(result.versionName).arg(downloadLink));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIconPixmap(QPixmap(":/img/MediaElch.png").scaledToWidth(64, Qt::SmoothTransformation));

    QCheckBox dontCheck(tr("Don't check for updates"), &msgBox);
    dontCheck.blockSignals(true);
    msgBox.addButton(&dontCheck, QMessageBox::ActionRole);
    msgBox.exec();
    if (dontCheck.checkState() == Qt::Checked) {
        Settings::instance()->setCheckForUpdates(false);
        Settings::instance()->saveSettings();
    }
}
