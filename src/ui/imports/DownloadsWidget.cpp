#include "DownloadsWidget.h"
#include "ui_DownloadsWidget.h"

#include <QComboBox>
#include <QMessageBox>
#include <QMutexLocker>
#include <QThread>

#include "data/Storage.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "settings/Settings.h"
#include "tv_shows/TvShowModel.h"
#include "ui/imports/ImportActions.h"
#include "ui/imports/UnpackButtons.h"
#include "ui/notifications/MacNotificationHandler.h"
#include "ui/notifications/Notificator.h"
#include "ui/small_widgets/MessageLabel.h"
#include "ui/small_widgets/MyTableWidgetItem.h"

DownloadsWidget::DownloadsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::DownloadsWidget)
{
    ui->setupUi(this);

    ui->tablePackages->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableImports->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tablePackages->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tablePackages->setColumnWidth(3, 200);
    ui->tableImports->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    helper::setButtonStyle(ui->btnImportMakeMkv, helper::ButtonInfo);

#ifdef Q_OS_WIN
    ui->tableImports->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->tablePackages->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
#endif

#ifndef Q_OS_MAC
    QFont titleFont = ui->labelArchives->font();
    titleFont.setPointSize(titleFont.pointSize() - 4);
    ui->labelArchives->setFont(titleFont);
    ui->labelImportable->setFont(titleFont);
#endif

    m_extractor = new Extractor(this);
    m_makeMkvDialog = new MakeMkvDialog(this);

    connect(m_extractor, &Extractor::sigError, this, &DownloadsWidget::onExtractorError);
    connect(m_extractor, &Extractor::sigFinished, this, &DownloadsWidget::onExtractorFinished);
    connect(m_extractor, &Extractor::sigProgress, this, &DownloadsWidget::onExtractorProgress);
    connect(ui->btnImportMakeMkv, &QAbstractButton::clicked, this, &DownloadsWidget::onImportWithMakeMkv);

    connect(Manager::instance()->tvShowFileSearcher(),
        &TvShowFileSearcher::tvShowsLoaded,
        this,
        &DownloadsWidget::scanDownloadsAndImports);

    scanDownloadFolders();

    helper::applyStyle(ui->tablePackages, true, false);
    helper::applyStyle(ui->tableImports, true, false);
}

DownloadsWidget::~DownloadsWidget()
{
    delete ui;
}

void DownloadsWidget::scanDownloadsAndImports()
{
    scanDownloadFolders(true, true);
}

void DownloadsWidget::scanDownloadFolders(bool scanDownloads, bool scanImports)
{
    using namespace mediaelch;

    QMutexLocker locker(&m_mutex);
    if (m_isSearchInProgress) {
        qInfo() << "[DownloadsWidget] Cannot start scan: Already in progress";
        return;
    }
    m_isSearchInProgress = true;

    qInfo() << "[DownloadsWidget] Start scanning for imports/downloads. Start Timer.";
    m_scanTimer.start();

    locker.unlock();

    // Run the file searcher in a worker thread.
    // \todo: Cleanup
    auto* thread = new QThread;
    /// File searcher. Is deleted in onScanFinished().
    auto* searcher = new DownloadFileSearcher(scanDownloads, scanImports);
    searcher->moveToThread(thread);
    connect(searcher, &DownloadFileSearcher::sigScanFinished, this, &DownloadsWidget::onScanFinished);
    connect(searcher, &DownloadFileSearcher::sigScanFinished, thread, &QThread::quit);
    connect(thread, &QThread::started, searcher, &DownloadFileSearcher::scan);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}


void DownloadsWidget::updatePackagesList(const QMap<QString, mediaelch::DownloadFileSearcher::Package>& packages)
{
    using namespace mediaelch;

    m_packages = packages;

    ui->tablePackages->clearContents();
    ui->tablePackages->setRowCount(0);

    QMapIterator<QString, DownloadFileSearcher::Package> it(packages);
    while (it.hasNext()) {
        it.next();
        QStringList files = it.value().files;
        files.sort();

        int row = ui->tablePackages->rowCount();
        ui->tablePackages->insertRow(row);

        auto* item0 = new MyTableWidgetItem(it.value().baseName);
        item0->setData(Qt::UserRole, it.value().baseName);
        auto* item1 = new MyTableWidgetItem(tr("%n files", "", files.length()), files.length());
        item1->setToolTip(files.join("\n"));
        ui->tablePackages->setItem(row, 0, item0);
        ui->tablePackages->setItem(row, 1, item1);
        ui->tablePackages->setItem(row, 2, new MyTableWidgetItem(it.value().size, true));

        auto* buttons = new UnpackButtons(this);
        buttons->setBaseName(it.value().baseName);
        connect(buttons, &UnpackButtons::sigUnpack, this, &DownloadsWidget::onUnpack);
        connect(buttons, &UnpackButtons::sigStop, m_extractor, &Extractor::stopExtraction);
        connect(buttons, &UnpackButtons::sigDelete, this, &DownloadsWidget::onDelete);
        ui->tablePackages->setCellWidget(row, 3, buttons);
    }
}

void DownloadsWidget::onUnpack(QString baseName, QString password)
{
    if (!m_packages.contains(baseName)) {
        return;
    }

    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            dynamic_cast<UnpackButtons*>(ui->tablePackages->cellWidget(row, 3))->setProgress(0);
            dynamic_cast<UnpackButtons*>(ui->tablePackages->cellWidget(row, 3))->setShowProgress(true);
            ui->tablePackages->setCellWidget(row, 4, nullptr);
        }
    }
    m_extractor->extract(baseName, m_packages[baseName].files, password);
}

void DownloadsWidget::onDelete(QString baseName)
{
    if (!m_packages.contains(baseName)) {
        return;
    }

    for (const QString& fileName : m_packages[baseName].files) {
        QFile::remove(fileName);
    }

    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            ui->tablePackages->removeRow(row);
            break;
        }
    }

    scanDownloadFolders(true, false);
}

void DownloadsWidget::onDeleteImport(QString baseName)
{
    if (!m_imports.contains(baseName)) {
        return;
    }

    for (const QString& fileName : m_imports[baseName].files) {
        QFile::remove(fileName);
    }

    for (int row = 0, n = ui->tableImports->rowCount(); row < n; ++row) {
        if (ui->tableImports->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            ui->tableImports->removeRow(row);
            break;
        }
    }

    scanDownloadFolders(false, true);
}

void DownloadsWidget::onExtractorError(QString baseName, QString msg)
{
#ifdef Q_OS_MAC
    if (MacNotificationHandler::instance()->hasUserNotificationCenterSupport()) {
        Notificator::instance()->notify(Notificator::Warning,
            tr("Extraction failed"),
            tr("Extraction of %1 has failed: %2").arg(baseName).arg(msg));
    } else {
        QMessageBox::warning(
            this, tr("Extraction failed"), tr("Extraction of %1 has failed: %2").arg(baseName).arg(msg));
    }
#else
    QMessageBox::warning(this, tr("Extraction failed"), tr("Extraction of %1 has failed: %2").arg(baseName).arg(msg));
#endif
}

void DownloadsWidget::onExtractorFinished(QString baseName, bool success)
{
    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            auto* label = new MessageLabel(this, Qt::AlignCenter | Qt::AlignVCenter);
            if (success) {
                label->setSuccessMessage(tr("Extraction finished"));
            } else {
                label->setErrorMessage(tr("Extraction failed"));
            }
            dynamic_cast<UnpackButtons*>(ui->tablePackages->cellWidget(row, 3))->setShowProgress(false);
            ui->tablePackages->setCellWidget(row, 4, label);
        }
    }
    if (success && Settings::instance()->deleteArchives()) {
        onDelete(baseName);
    }

    if (success) {
        Notificator::instance()->notify(
            Notificator::Information, tr("Extraction finished"), tr("Extraction of %1 finished").arg(baseName));
    }

    scanDownloadFolders(true, true);
}

void DownloadsWidget::onExtractorProgress(QString baseName, int progress)
{
    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            dynamic_cast<UnpackButtons*>(ui->tablePackages->cellWidget(row, 3))->setShowProgress(true);
            dynamic_cast<UnpackButtons*>(ui->tablePackages->cellWidget(row, 3))->setProgress(progress);
        }
    }
}

void DownloadsWidget::updateImportsList(const QMap<QString, mediaelch::DownloadFileSearcher::Import>& imports)
{
    m_imports = imports;

    ui->tableImports->clearContents();
    ui->tableImports->setRowCount(0);

    QMapIterator<QString, mediaelch::DownloadFileSearcher::Import> it(imports);
    while (it.hasNext()) {
        it.next();
        QStringList files = it.value().files;
        files << it.value().extraFiles;
        files.sort();

        int row = ui->tableImports->rowCount();
        ui->tableImports->insertRow(row);
        auto* itemBaseName = new MyTableWidgetItem(it.value().baseName);
        itemBaseName->setData(Qt::UserRole, it.value().baseName);
        auto* itemFileCount = new MyTableWidgetItem(tr("%n files", "", files.length()), files.length());
        itemFileCount->setToolTip(files.join("\n"));

        ui->tableImports->setItem(row, 0, itemBaseName);
        ui->tableImports->setItem(row, 1, itemFileCount);
        ui->tableImports->setItem(row, 2, new MyTableWidgetItem(it.value().size, true));

        QString guessedType;
        QString guessedDir;
        bool guessed = Manager::instance()->database()->guessImport(it.value().baseName, guessedType, guessedDir);

        auto* importType = new QComboBox(this);
        importType->setProperty("baseName", it.value().baseName);
        importType->addItem(tr("Movie"), "movie");
        importType->addItem(tr("TV Show"), "tvshow");
        importType->addItem(tr("Concert"), "concert");
        connect(importType,
            elchOverload<int>(&QComboBox::currentIndexChanged),
            this,
            elchOverload<int>(&DownloadsWidget::onChangeImportType));
        ui->tableImports->setCellWidget(row, 3, importType);

        auto* importDetail = new QComboBox(this);
        importDetail->setProperty("baseName", it.value().baseName);
        connect(importDetail,
            elchOverload<int>(&QComboBox::currentIndexChanged),
            this,
            elchOverload<int>(&DownloadsWidget::onChangeImportDetail));
        ui->tableImports->setCellWidget(row, 4, importDetail);

        auto* actions = new ImportActions(this);
        actions->setButtonEnabled(false);
        actions->setBaseName(it.value().baseName);
        ui->tableImports->setCellWidget(row, 5, actions);
        connect(actions, &ImportActions::sigDelete, this, &DownloadsWidget::onDeleteImport);
        connect(actions, &ImportActions::sigDialogClosed, this, &DownloadsWidget::scanDownloadsAndImports);

        onChangeImportType(0, importType);

        if (guessed) {
            importType->blockSignals(true);
            importDetail->blockSignals(true);
            if (guessedType == "movie") {
                importType->setCurrentIndex(0);
                onChangeImportType(0, importType);
                for (int i = 0, n = importDetail->count(); i < n; ++i) {
                    if (importDetail->itemText(i) == guessedDir) {
                        importDetail->setCurrentIndex(i);
                        onChangeImportDetail(i, importDetail);
                        break;
                    }
                }
            } else if (guessedType == "tvshow") {
                importType->setCurrentIndex(1);
                onChangeImportType(1, importType);
                for (int i = 0, n = importDetail->count(); i < n; ++i) {
                    if (importDetail->itemData(i, Qt::UserRole).value<Storage*>()->show()->dir() == guessedDir) {
                        importDetail->setCurrentIndex(i);
                        onChangeImportDetail(i, importDetail);
                        break;
                    }
                }
            } else if (guessedType == "concert") {
                importType->setCurrentIndex(2);
                onChangeImportType(2, importType);
                for (int i = 0, n = importDetail->count(); i < n; ++i) {
                    if (importDetail->itemText(i) == guessedDir) {
                        importDetail->setCurrentIndex(i);
                        onChangeImportDetail(i, importDetail);
                        break;
                    }
                }
            }
            importType->blockSignals(false);
            importDetail->blockSignals(false);
        }
    }
}
void DownloadsWidget::onChangeImportType(int currentIndex)
{
    auto* box = dynamic_cast<QComboBox*>(QObject::sender());
    onChangeImportType(currentIndex, box);
}

void DownloadsWidget::onChangeImportType(int currentIndex, QComboBox* box)
{
    if (box == nullptr) {
        qCritical() << "[DownloadsWidget] Import type change: Cannot get QComboBox from sender";
        return;
    }

    if (currentIndex < 0 || currentIndex >= box->count()) {
        return;
    }

    QString type = box->itemData(currentIndex, Qt::UserRole).toString();
    QString baseName = box->property("baseName").toString();
    int row = -1;
    for (int i = 0, n = ui->tableImports->rowCount(); i < n; ++i) {
        if (ui->tableImports->item(i, 0)->data(Qt::UserRole).toString() == baseName) {
            row = i;
            break;
        }
    }
    if (row == -1) {
        return;
    }

    auto* detailBox = dynamic_cast<QComboBox*>(ui->tableImports->cellWidget(row, 4));
    if (detailBox == nullptr) {
        qCritical() << "[DownloadsWidget] Import type change: Cannot get QComboBox from download table";
        return;
    }

    detailBox->clear();

    bool sub = false;
    if (type == "movie") {
        for (const SettingsDir& dir : Settings::instance()->directorySettings().movieDirectories()) {
            detailBox->addItem(dir.path.path());
            sub = true;
        }
    } else if (type == "tvshow") {
        for (TvShow* show : Manager::instance()->tvShowModel()->tvShows()) {
            detailBox->addItem(show->title(), Storage::toVariant(this, show));
            sub = true;
        }
    } else if (type == "concert") {
        for (const SettingsDir& dir : Settings::instance()->directorySettings().concertDirectories()) {
            detailBox->addItem(dir.path.path());
            sub = true;
        }
    }

    auto* actions = dynamic_cast<ImportActions*>(ui->tableImports->cellWidget(row, 5));
    if (actions == nullptr) {
        qCritical() << "[DownloadsWidget] Import type change: Cannot get ImportActions from download table";
        return;
    }

    actions->setButtonEnabled(sub);
}

void DownloadsWidget::onChangeImportDetail(int currentIndex)
{
    auto* box = dynamic_cast<QComboBox*>(QObject::sender());
    onChangeImportDetail(currentIndex, box);
}

void DownloadsWidget::onChangeImportDetail(int currentIndex, QComboBox* box)
{
    if (currentIndex < 0 || currentIndex >= box->count()) {
        return;
    }

    QString baseName = box->property("baseName").toString();
    if (!m_imports.contains(baseName)) {
        return;
    }

    int row = -1;
    for (int i = 0, n = ui->tableImports->rowCount(); i < n; ++i) {
        if (ui->tableImports->item(i, 0)->data(Qt::UserRole).toString() == baseName) {
            row = i;
            break;
        }
    }
    if (row == -1) {
        return;
    }

    auto* typeBox = dynamic_cast<QComboBox*>(ui->tableImports->cellWidget(row, 3));
    auto* actions = dynamic_cast<ImportActions*>(ui->tableImports->cellWidget(row, 5));
    QString type = typeBox->itemData(typeBox->currentIndex(), Qt::UserRole).toString();
    actions->setType(type);
    if (type == "movie") {
        actions->setImportDir(box->currentText());
    } else if (type == "tvshow") {
        actions->setTvShow(box->itemData(currentIndex, Qt::UserRole).value<Storage*>()->show());
    } else if (type == "concert") {
        actions->setImportDir(box->currentText());
    }

    actions->setFiles(m_imports[baseName].files);
    actions->setExtraFiles(m_imports[baseName].extraFiles);
}

int DownloadsWidget::hasNewItems()
{
    return m_imports.count() + m_packages.count();
}

void DownloadsWidget::onImportWithMakeMkv()
{
    if (!QFileInfo(Settings::instance()->importSettings().makeMkvCon()).isExecutable()) {
        QMessageBox::warning(this,
            tr("makemkvcon missing"),
            tr("Please set the correct path to makemkvcon in MediaElch's settings."),
            QMessageBox::Ok);
        return;
    }

    m_makeMkvDialog->exec();
}

void DownloadsWidget::onScanFinished(mediaelch::DownloadFileSearcher* searcher)
{
    QMutexLocker locker(&m_mutex);
    m_isSearchInProgress = false;
    locker.unlock();

    qInfo() << "[DownloadsWidget] Scanning for imports/downloads took:" << m_scanTimer.elapsed() << "ms";
    m_scanTimer.restart();

    const auto packages = searcher->packages();
    const auto imports = searcher->imports();

    if (!packages.isEmpty()) {
        updatePackagesList(packages);
    }

    if (!imports.isEmpty()) {
        updateImportsList(imports);
    }

    // Delete only after we have used it's members because "searcher" lives in another
    // thread, calling deleteLater() deletes it likely immediately.
    searcher->deleteLater();

    qInfo() << "[DownloadsWidget] Updating imports/downloads lists:" << m_scanTimer.elapsed() << "ms";
    m_scanTimer.invalidate();

    const bool hasDownloads = !packages.isEmpty() || !imports.isEmpty();
    emit sigScanFinished(hasDownloads);
}
