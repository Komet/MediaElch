#include "DownloadsWidget.h"
#include "ui_DownloadsWidget.h"

#include <QComboBox>
#include <QDirIterator>
#include <QMessageBox>
#include <QMutexLocker>

#include "data/Storage.h"
#include "data/TvShowModel.h"
#include "downloads/ImportActions.h"
#include "downloads/UnpackButtons.h"
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "notifications/MacNotificationHandler.h"
#include "notifications/Notificator.h"
#include "settings/Settings.h"
#include "smallWidgets/MessageLabel.h"
#include "smallWidgets/MyTableWidgetItem.h"

DownloadsWidget::DownloadsWidget(QWidget *parent) : QWidget(parent), ui(new Ui::DownloadsWidget)
{
    ui->setupUi(this);

    ui->tablePackages->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableImports->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tablePackages->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tablePackages->setColumnWidth(3, 200);
    ui->tableImports->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    Helper::instance()->setButtonStyle(ui->btnImportMakeMkv, Helper::ButtonInfo);

#ifdef Q_OS_WIN32
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

    connect(m_extractor, SIGNAL(sigError(QString, QString)), this, SLOT(onExtractorError(QString, QString)));
    connect(m_extractor, SIGNAL(sigFinished(QString, bool)), this, SLOT(onExtractorFinished(QString, bool)));
    connect(m_extractor, SIGNAL(sigProgress(QString, int)), this, SLOT(onExtractorProgress(QString, int)));
    connect(ui->btnImportMakeMkv, SIGNAL(clicked()), this, SLOT(onImportWithMakeMkv()));

    connect(Manager::instance()->tvShowFileSearcher(), SIGNAL(tvShowsLoaded(int)), this, SLOT(scanDownloadFolders()));

    scanDownloadFolders();

    Helper::instance()->applyStyle(ui->tablePackages, true, false);
    Helper::instance()->applyStyle(ui->tableImports, true, false);
}

DownloadsWidget::~DownloadsWidget()
{
    delete ui;
}

void DownloadsWidget::scanDownloadFolders(bool scanDownloads, bool scanImports)
{
    QMutexLocker locker(&m_mutex);

    QMap<QString, Package> packages;
    QMap<QString, Import> imports;
    foreach (SettingsDir settingsDir, Settings::instance()->downloadDirectories()) {
        QString dir = settingsDir.path;
        QDirIterator it(dir,
            QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files,
            QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);
        while (it.hasNext()) {
            it.next();
            if (isPackage(it.fileInfo())) {
                QString base = baseName(it.fileInfo());
                if (packages.contains(base)) {
                    packages[base].files.append(it.filePath());
                    packages[base].size += it.fileInfo().size();
                } else {
                    Package p;
                    p.baseName = base;
                    p.size = it.fileInfo().size();
                    p.files << it.filePath();
                    packages.insert(base, p);
                }
            } else if (isImportable(it.fileInfo()) || isSubtitle(it.fileInfo())) {
                QString base = it.fileInfo().completeBaseName();
                if (imports.contains(base)) {
                    if (isSubtitle(it.fileInfo()))
                        imports[base].extraFiles.append(it.filePath());
                    else
                        imports[base].files.append(it.filePath());
                    imports[base].size += it.fileInfo().size();
                } else {
                    Import i;
                    i.baseName = base;
                    if (isSubtitle(it.fileInfo()))
                        i.extraFiles << it.filePath();
                    else
                        i.files << it.filePath();
                    i.size = it.fileInfo().size();
                    imports.insert(base, i);
                }
            }
        }
    }

    QMapIterator<QString, Import> it(imports);
    QStringList onlyExtraFiles;
    while (it.hasNext()) {
        it.next();
        if (it.value().files.isEmpty())
            onlyExtraFiles.append(it.key());
    }
    foreach (const QString &base, onlyExtraFiles)
        imports.remove(base);

    if (scanDownloads)
        updatePackagesList(packages);
    if (scanImports)
        updateImportsList(imports);

    emit sigScanFinished(!packages.isEmpty() || !imports.isEmpty());
}

QString DownloadsWidget::baseName(QFileInfo fileInfo) const
{
    QString fileName = fileInfo.fileName();
    QRegExp rx("(.*)(part[0-9]*)\\.rar");
    if (rx.exactMatch(fileName))
        return rx.cap(1).endsWith(".") ? rx.cap(1).mid(0, rx.cap(1).length() - 1) : rx.cap(1);

    rx.setPattern("(.*)\\.r(ar|[0-9]*)");
    if (rx.exactMatch(fileName))
        return rx.cap(1);

    return fileName;
}

bool DownloadsWidget::isPackage(QFileInfo file) const
{
    if (file.suffix() == "rar")
        return true;

    QRegExp rx("r[0-9]*");
    if (rx.exactMatch(file.suffix()))
        return true;

    return false;
}

bool DownloadsWidget::isImportable(QFileInfo file) const
{
    QStringList filters;
    filters << Settings::instance()->advanced()->movieFilters();
    filters << Settings::instance()->advanced()->tvShowFilters();
    filters << Settings::instance()->advanced()->concertFilters();
    filters.removeDuplicates();

    foreach (const QString &filter, filters) {
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName()))
            return true;
    }
    return false;
}

bool DownloadsWidget::isSubtitle(QFileInfo file) const
{
    foreach (const QString &filter, Settings::instance()->advanced()->subtitleFilters()) {
        QRegExp rx(filter);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(file.fileName()))
            return true;
    }
    return false;
}

void DownloadsWidget::updatePackagesList(QMap<QString, Package> packages)
{
    m_packages = packages;

    ui->tablePackages->clearContents();
    ui->tablePackages->setRowCount(0);

    QMapIterator<QString, Package> it(packages);
    while (it.hasNext()) {
        it.next();
        QStringList files = it.value().files;
        files.sort();

        int row = ui->tablePackages->rowCount();
        ui->tablePackages->insertRow(row);

        MyTableWidgetItem *item0 = new MyTableWidgetItem(it.value().baseName);
        item0->setData(Qt::UserRole, it.value().baseName);
        MyTableWidgetItem *item1 = new MyTableWidgetItem(tr("%n file(s)", "", files.length()), files.length());
        item1->setToolTip(files.join("\n"));
        ui->tablePackages->setItem(row, 0, item0);
        ui->tablePackages->setItem(row, 1, item1);
        ui->tablePackages->setItem(row, 2, new MyTableWidgetItem(it.value().size, true));

        UnpackButtons *buttons = new UnpackButtons(this);
        buttons->setBaseName(it.value().baseName);
        connect(buttons, SIGNAL(sigUnpack(QString, QString)), this, SLOT(onUnpack(QString, QString)));
        connect(buttons, SIGNAL(sigStop(QString)), m_extractor, SLOT(stopExtraction(QString)));
        connect(buttons, SIGNAL(sigDelete(QString)), this, SLOT(onDelete(QString)));
        ui->tablePackages->setCellWidget(row, 3, buttons);
    }
}

void DownloadsWidget::onUnpack(QString baseName, QString password)
{
    if (!m_packages.contains(baseName))
        return;

    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            static_cast<UnpackButtons *>(ui->tablePackages->cellWidget(row, 3))->setProgress(0);
            static_cast<UnpackButtons *>(ui->tablePackages->cellWidget(row, 3))->setShowProgress(true);
            ui->tablePackages->setCellWidget(row, 4, 0);
        }
    }
    m_extractor->extract(baseName, m_packages[baseName].files, password);
}

void DownloadsWidget::onDelete(QString baseName)
{
    if (!m_packages.contains(baseName))
        return;

    foreach (const QString &fileName, m_packages[baseName].files)
        QFile::remove(fileName);

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
    if (!m_imports.contains(baseName))
        return;

    foreach (const QString &fileName, m_imports[baseName].files)
        QFile::remove(fileName);

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
    if (MacNotificationHandler::instance()->hasUserNotificationCenterSupport())
        Notificator::instance()->notify(Notificator::Warning,
            tr("Extraction failed"),
            tr("Extraction of %1 has failed: %2").arg(baseName).arg(msg));
    else
        QMessageBox::warning(
            this, tr("Extraction failed"), tr("Extraction of %1 has failed: %2").arg(baseName).arg(msg));
#else
    QMessageBox::warning(this, tr("Extraction failed"), tr("Extraction of %1 has failed: %2").arg(baseName).arg(msg));
#endif
}

void DownloadsWidget::onExtractorFinished(QString baseName, bool success)
{
    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            MessageLabel *label = new MessageLabel(this, Qt::AlignCenter | Qt::AlignVCenter);
            if (success)
                label->setSuccessMessage(tr("Extraction finished"));
            else
                label->setErrorMessage(tr("Extraction failed"));
            static_cast<UnpackButtons *>(ui->tablePackages->cellWidget(row, 3))->setShowProgress(false);
            ui->tablePackages->setCellWidget(row, 4, label);
        }
    }
    if (success && Settings::instance()->deleteArchives())
        onDelete(baseName);

    if (success)
        Notificator::instance()->notify(
            Notificator::Information, tr("Extraction finished"), tr("Extraction of %1 finished").arg(baseName));

    scanDownloadFolders(true, true);
}

void DownloadsWidget::onExtractorProgress(QString baseName, int progress)
{
    for (int row = 0, n = ui->tablePackages->rowCount(); row < n; ++row) {
        if (ui->tablePackages->item(row, 0)->data(Qt::UserRole).toString() == baseName) {
            static_cast<UnpackButtons *>(ui->tablePackages->cellWidget(row, 3))->setShowProgress(true);
            static_cast<UnpackButtons *>(ui->tablePackages->cellWidget(row, 3))->setProgress(progress);
        }
    }
}

void DownloadsWidget::updateImportsList(QMap<QString, Import> imports)
{
    m_imports = imports;

    ui->tableImports->clearContents();
    ui->tableImports->setRowCount(0);

    QMapIterator<QString, Import> it(imports);
    while (it.hasNext()) {
        it.next();
        QStringList files = it.value().files;
        files << it.value().extraFiles;
        files.sort();

        int row = ui->tableImports->rowCount();
        ui->tableImports->insertRow(row);
        MyTableWidgetItem *item0 = new MyTableWidgetItem(it.value().baseName);
        item0->setData(Qt::UserRole, it.value().baseName);
        MyTableWidgetItem *item1 = new MyTableWidgetItem(tr("%n file(s)", "", files.length()), files.length());
        item1->setToolTip(files.join("\n"));

        ui->tableImports->setItem(row, 0, item0);
        ui->tableImports->setItem(row, 1, item1);
        ui->tableImports->setItem(row, 2, new MyTableWidgetItem(it.value().size, true));

        QString guessedType;
        QString guessedDir;
        bool guessed = Manager::instance()->database()->guessImport(it.value().baseName, guessedType, guessedDir);

        QComboBox *importType = new QComboBox(this);
        importType->setProperty("baseName", it.value().baseName);
        importType->addItem(tr("Movie"), "movie");
        importType->addItem(tr("TV Show"), "tvshow");
        importType->addItem(tr("Concert"), "concert");
        connect(importType, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeImportType(int)));
        ui->tableImports->setCellWidget(row, 3, importType);

        QComboBox *importDetail = new QComboBox(this);
        importDetail->setProperty("baseName", it.value().baseName);
        connect(importDetail, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeImportDetail(int)));
        ui->tableImports->setCellWidget(row, 4, importDetail);

        ImportActions *actions = new ImportActions(this);
        actions->setButtonEnabled(false);
        actions->setBaseName(it.value().baseName);
        ui->tableImports->setCellWidget(row, 5, actions);
        connect(actions, SIGNAL(sigDelete(QString)), this, SLOT(onDeleteImport(QString)));
        connect(actions, SIGNAL(sigDialogClosed()), this, SLOT(scanDownloadFolders()));

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
                    if (importDetail->itemData(i, Qt::UserRole).value<Storage *>()->show()->dir() == guessedDir) {
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

void DownloadsWidget::onChangeImportType(int currentIndex, QComboBox *sender)
{
    QComboBox *box;
    if (sender)
        box = sender;
    else
        box = static_cast<QComboBox *>(QObject::sender());
    if (currentIndex < 0 || currentIndex >= box->count())
        return;

    QString type = box->itemData(currentIndex, Qt::UserRole).toString();
    QString baseName = box->property("baseName").toString();
    int row = -1;
    for (int i = 0, n = ui->tableImports->rowCount(); i < n; ++i) {
        if (ui->tableImports->item(i, 0)->data(Qt::UserRole).toString() == baseName) {
            row = i;
            break;
        }
    }
    if (row == -1)
        return;

    QComboBox *detailBox = static_cast<QComboBox *>(ui->tableImports->cellWidget(row, 4));
    detailBox->clear();

    bool sub = false;
    if (type == "movie") {
        foreach (SettingsDir dir, Settings::instance()->movieDirectories()) {
            detailBox->addItem(dir.path);
            sub = true;
        }
    } else if (type == "tvshow") {
        foreach (TvShow *show, Manager::instance()->tvShowModel()->tvShows()) {
            detailBox->addItem(show->name(), Storage::toVariant(this, show));
            sub = true;
        }
    } else if (type == "concert") {
        foreach (SettingsDir dir, Settings::instance()->concertDirectories()) {
            detailBox->addItem(dir.path);
            sub = true;
        }
    }

    static_cast<ImportActions *>(ui->tableImports->cellWidget(row, 5))->setButtonEnabled(sub);
}

void DownloadsWidget::onChangeImportDetail(int currentIndex, QComboBox *sender)
{
    QComboBox *box;
    if (sender)
        box = sender;
    else
        box = static_cast<QComboBox *>(QObject::sender());
    if (currentIndex < 0 || currentIndex >= box->count())
        return;

    QString baseName = box->property("baseName").toString();
    if (!m_imports.contains(baseName))
        return;

    int row = -1;
    for (int i = 0, n = ui->tableImports->rowCount(); i < n; ++i) {
        if (ui->tableImports->item(i, 0)->data(Qt::UserRole).toString() == baseName) {
            row = i;
            break;
        }
    }
    if (row == -1)
        return;

    QComboBox *typeBox = static_cast<QComboBox *>(ui->tableImports->cellWidget(row, 3));
    ImportActions *actions = static_cast<ImportActions *>(ui->tableImports->cellWidget(row, 5));
    QString type = typeBox->itemData(typeBox->currentIndex(), Qt::UserRole).toString();
    actions->setType(type);
    if (type == "movie")
        actions->setImportDir(box->currentText());
    else if (type == "tvshow")
        actions->setTvShow(box->itemData(currentIndex, Qt::UserRole).value<Storage *>()->show());
    else if (type == "concert")
        actions->setImportDir(box->currentText());

    actions->setFiles(m_imports[baseName].files);
    actions->setExtraFiles(m_imports[baseName].extraFiles);
}

int DownloadsWidget::hasNewItems()
{
    return m_imports.count() + m_packages.count();
}

void DownloadsWidget::onImportWithMakeMkv()
{
    if (!QFileInfo(Settings::instance()->makeMkvCon()).isExecutable()) {
        QMessageBox::warning(this,
            tr("makemkvcon missing"),
            tr("Please set the correct path to makemkvcon in MediaElchs settings."),
            QMessageBox::Ok);
        return;
    }

    m_makeMkvDialog->exec();
}
