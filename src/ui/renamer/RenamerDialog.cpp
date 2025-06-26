#include "ui/renamer/RenamerDialog.h"
#include "ui_RenamerDialog.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "log/Log.h"
#include "renamer/MovieRenamer.h"

#include <QTimer>
#include <algorithm>

RenamerDialog::RenamerDialog(QWidget* parent) : QDialog(parent), ui(new Ui::RenamerDialog)
{
    ui->setupUi(this);

    ui->resultsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->resultsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
#ifdef Q_OS_MAC
    QFont font = ui->resultsTable->font();
    font.setPointSize(font.pointSize() - 2);
    ui->resultsTable->setFont(font);
#endif

    connect(ui->chkDirectoryNaming, &QCheckBox::stateChanged, this, &RenamerDialog::onChkRenameDirectories);
    connect(ui->chkFileNaming, &QCheckBox::stateChanged, this, &RenamerDialog::onChkRenameFiles);
    connect(ui->chkSeasonDirectories, &QCheckBox::stateChanged, this, &RenamerDialog::onChkUseSeasonDirectories);
    connect(ui->chkReplaceDelimiter, &QCheckBox::stateChanged, this, &RenamerDialog::onChkReplaceDelimiter);
    connect(ui->btnDryRun, &QAbstractButton::clicked, this, &RenamerDialog::onDryRun);
    connect(ui->btnRename, &QAbstractButton::clicked, this, &RenamerDialog::onRename);

    onChkRenameDirectories();
    onChkRenameFiles();
    onChkReplaceDelimiter();

    m_extraFiles = Settings::instance()->advanced()->subtitleFilters();
    ui->helpLabel->setText(tr("Please see %1 for help and examples on how to use the renamer.")
            .arg("<a "
                 "href=\"https://mediaelch.github.io/mediaelch-doc/renaming.html\">"
                 "Renaming Files</a>"));
}

RenamerDialog::~RenamerDialog()
{
    delete ui;
}

int RenamerDialog::exec()
{
    m_filesRenamed = false;
    m_renameErrorOccurred = false;

    const QString infoLabel = dialogInfoLabel();
    ui->infoLabel->setText(infoLabel);

    QString fileName;
    QString fileNameMulti;
    QString directoryName;
    QString seasonName;
    QString newDelimiter;
    bool renameFiles = false;
    bool renameFolders = false;
    bool useSeasonDirectories = false;
    bool replaceDelimiter = false;
    Settings::instance()->renamePatterns(
        m_renameType, fileName, fileNameMulti, directoryName, seasonName, newDelimiter);
    Settings::instance()->renamings(m_renameType, renameFiles, renameFolders, useSeasonDirectories, replaceDelimiter);

    // Default texts for combo box.
    QStringList fileNameDefaults = this->fileNameDefaults();

    QStringList fileNameMultiDefaults = this->fileNameMultiDefaults();
    QStringList dirNameDefaults = directoryNameDefaults();

    QStringList seasonNameDefaults = QStringList{//
        "Season <season>",
        "Season <season> - <seasonName>"};

    ui->fileNaming->setItems(fileNameDefaults);
    ui->fileNaming->setText(fileName);

    ui->fileNamingMulti->setItems(fileNameMultiDefaults);
    ui->fileNamingMulti->setText(fileNameMulti);

    ui->directoryNaming->setItems(dirNameDefaults);
    ui->directoryNaming->setText(directoryName);

    ui->seasonNaming->setItems(seasonNameDefaults);
    ui->seasonNaming->setText(seasonName);

    const QStringList delimiters{"_", "-"};
    ui->newDelimiterNaming->addItems(delimiters);
    ui->newDelimiterNaming->setCurrentIndex(std::max<elch_ssize_t>(delimiters.indexOf(newDelimiter), 0));

    ui->chkFileNaming->setChecked(renameFiles);
    ui->chkDirectoryNaming->setChecked(renameFolders);
    ui->chkSeasonDirectories->setChecked(useSeasonDirectories);
    ui->chkReplaceDelimiter->setChecked(replaceDelimiter);

    ui->chkSeasonDirectories->setVisible(m_renameType == RenameType::TvShows);
    ui->seasonNaming->setVisible(m_renameType == RenameType::TvShows);
    ui->labelSeasonDirectory->setVisible(m_renameType == RenameType::TvShows);

    initPlaceholders();

    ui->results->clear();
    ui->resultsTable->setRowCount(0);
    ui->btnDryRun->setEnabled(true);
    ui->btnRename->setEnabled(true);

    ui->tabWidget->setCurrentIndex(0);

    return QDialog::exec();
}

void RenamerDialog::reject()
{
    rejectImpl();

    Settings::instance()->setRenamePatterns(m_renameType,
        ui->fileNaming->text(),
        ui->fileNamingMulti->text(),
        ui->directoryNaming->text(),
        ui->seasonNaming->text(),
        ui->newDelimiterNaming->currentText());
    Settings::instance()->setRenamings(m_renameType,
        ui->chkFileNaming->isChecked(),
        ui->chkDirectoryNaming->isChecked(),
        ui->chkSeasonDirectories->isChecked(),
        ui->chkReplaceDelimiter->isChecked());

    QDialog::reject();
    if (m_filesRenamed) {
        QTimer::singleShot(0, this, &RenamerDialog::onRenamed);
    }
}

void RenamerDialog::onRenamed()
{
    emit sigFilesRenamed(m_renameType, renameErrorOccurred());
}

bool RenamerDialog::renameErrorOccurred() const
{
    return m_renameErrorOccurred;
}

void RenamerDialog::onChkRenameDirectories()
{
    ui->directoryNaming->setEnabled(ui->chkDirectoryNaming->isChecked());
}

void RenamerDialog::onChkRenameFiles()
{
    ui->fileNaming->setEnabled(ui->chkFileNaming->isChecked());
    ui->fileNamingMulti->setEnabled(ui->chkFileNaming->isChecked());
}

void RenamerDialog::onChkReplaceDelimiter()
{
    ui->newDelimiterNaming->setEnabled(ui->chkReplaceDelimiter->isChecked());
}

void RenamerDialog::onChkUseSeasonDirectories()
{
    ui->seasonNaming->setEnabled(ui->chkSeasonDirectories->isChecked());
}

void RenamerDialog::onRename()
{
    ui->btnRename->setEnabled(false);
    ui->btnDryRun->setEnabled(false);

    renameType(false);
}

void RenamerDialog::onDryRun()
{
    renameType(true);
}

int RenamerDialog::addResultToTable(const QString& oldFileName,
    const QString& newFileName,
    Renamer::RenameOperation operation)
{
    const QString opString = [operation]() -> QString {
        switch (operation) {
        case Renamer::RenameOperation::CreateDir: return tr("Create dir");
        case Renamer::RenameOperation::Move: return tr("Move");
        case Renamer::RenameOperation::Rename: return tr("Rename");
        }
        qCCritical(generic) << "[RenamerDialog] RenameOperation: Missing case.";
        return QString("");
    }();

    QFont font = ui->resultsTable->font();
    font.setBold(true);

    int row = ui->resultsTable->rowCount();
    ui->resultsTable->insertRow(row);
    ui->resultsTable->setItem(row, 0, new QTableWidgetItem(opString));
    ui->resultsTable->setItem(row, 1, new QTableWidgetItem(oldFileName));
    ui->resultsTable->setItem(row, 2, new QTableWidgetItem(newFileName));
    ui->resultsTable->item(row, 0)->setFont(font);

    return row;
}

void RenamerDialog::setResultStatus(int row, Renamer::RenameResult result)
{
    for (int col = 0, n = ui->resultsTable->columnCount(); col < n; ++col) {
        if (result == Renamer::RenameResult::Failed) {
            ui->resultsTable->item(row, col)->setBackground(QColor(242, 222, 222));
            ui->resultsTable->item(row, col)->setForeground(QColor(0, 0, 0));
        }
    }
}

void RenamerDialog::appendResultText(QString str)
{
    ui->results->append(str);
}
