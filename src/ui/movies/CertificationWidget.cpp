#include "CertificationWidget.h"
#include "ui_CertificationWidget.h"

#include "globals/Helper.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "movies/Movie.h"
#include "ui/movie_sets/MovieListDialog.h"
#include "ui/notifications/NotificationBox.h"

CertificationWidget::CertificationWidget(QWidget* parent) : QWidget(parent), ui(new Ui::CertificationWidget)
{
    ui->setupUi(this);
    ui->certifications->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

#ifndef Q_OS_MAC
    QFont nameFont = ui->certificationName->font();
    nameFont.setPointSize(nameFont.pointSize() - 4);
    ui->certificationName->setFont(nameFont);
#endif

    ui->certifications->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableContextMenu = new QMenu(this);
    auto* actionAddCertification = new QAction(tr("Add Certification"), this);
    auto* actionDeleteCertification = new QAction(tr("Delete Certification"), this);
    m_tableContextMenu->addAction(actionAddCertification);
    m_tableContextMenu->addAction(actionDeleteCertification);

    // clang-format off
    connect(actionAddCertification,    &QAction::triggered, this, &CertificationWidget::addCertification);
    connect(actionDeleteCertification, &QAction::triggered, this, &CertificationWidget::deleteCertification);

    connect(ui->certifications, &QWidget::customContextMenuRequested, this, &CertificationWidget::showCertificationsContextMenu);
    connect(ui->certifications, &QTableWidget::itemSelectionChanged,  this, &CertificationWidget::onCertificationSelected);
    connect(ui->certifications, &QTableWidget::itemChanged,           this, &CertificationWidget::onCertificationNameChanged);
    connect(ui->movies,         &QTableWidget::itemDoubleClicked,     this, &CertificationWidget::onJumpToMovie);
    // clang-format on

    helper::applyStyle(ui->infoGroupBox);
    helper::applyStyle(ui->certifications);
    helper::applyStyle(ui->label_2);
    helper::applyStyle(ui->label_3);
}

/**
 * \brief CertificationWidget::~GenreWidget
 */
CertificationWidget::~CertificationWidget()
{
    delete ui;
}

/**
 * \brief Executes the certifications table context menu
 * \param point Point where the menu will be shown
 */
void CertificationWidget::showCertificationsContextMenu(QPoint point)
{
    m_tableContextMenu->exec(ui->certifications->mapToGlobal(point));
}

/**
 * \brief Returns the splitter
 * \return The splitter
 */
QSplitter* CertificationWidget::splitter()
{
    return ui->splitter;
}

/**
 * \brief Clears the genres table
 */
void CertificationWidget::clear()
{
    ui->certifications->clearContents();
    ui->certifications->setRowCount(0);
    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->certificationName->clear();
    m_addedCertifications.clear();
}

/**
 * \brief Loads all certifications from movies and fills the certifications table
 */
void CertificationWidget::loadCertifications()
{
    emit setActionSaveEnabled(false, MainWidgets::Certifications);
    ui->certifications->blockSignals(true);
    clear();
    QStringList certifications;
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        QString certStr = movie->certification().toString();
        if (movie->certification().isValid() && !certifications.contains(certStr)) {
            certifications.append(certStr);
        }
    }

    for (const Certification& certification : m_addedCertifications) {
        if (certification.isValid() && !certifications.contains(certification.toString())) {
            certifications.append(certification.toString());
        }
    }

    std::sort(certifications.begin(), certifications.end(), LocaleStringCompare());

    for (const QString& certification : certifications) {
        auto* item = new QTableWidgetItem(certification);
        item->setData(Qt::UserRole, certification);
        int row = ui->certifications->rowCount();
        ui->certifications->insertRow(row);
        ui->certifications->setItem(row, 0, item);
    }

    ui->certifications->blockSignals(false);
    emit setActionSaveEnabled(true, MainWidgets::Certifications);
}

/**
 * \brief Fills the movies table with movies
 */
void CertificationWidget::onCertificationSelected()
{
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid row" << ui->certifications->currentRow();
        return;
    }

    ui->movies->clearContents();
    ui->movies->setRowCount(0);
    ui->movies->setSortingEnabled(false);

    Certification certification = Certification(ui->certifications->item(ui->certifications->currentRow(), 0)->text());
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == certification) {
            const int row = ui->movies->rowCount();
            auto* item = new QTableWidgetItem(movie->name());
            item->setData(Qt::UserRole, QVariant::fromValue(movie));
            ui->movies->insertRow(row);
            ui->movies->setItem(row, 0, item);
        }
    }
    ui->certificationName->setText(certification.toString());
    ui->movies->setSortingEnabled(true);
    ui->movies->sortByColumn(0, Qt::AscendingOrder);
}

/**
 * \brief Renames a certification
 * \param item Changed item in certifications table
 */
void CertificationWidget::onCertificationNameChanged(QTableWidgetItem* item)
{
    const auto newName = Certification(item->text());
    const auto origName = Certification(item->data(Qt::UserRole).toString());
    if (newName == origName) {
        return;
    }

    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == origName) {
            movie->setCertification(newName);
        }
    }

    ui->certificationName->setText(newName.toString());
    item->setData(Qt::UserRole, newName.toString());

    if (m_addedCertifications.contains(origName)) {
        m_addedCertifications.removeOne(origName);
        if (!m_addedCertifications.contains(newName)) {
            m_addedCertifications.append(newName);
        }
    }
    loadCertifications();
}

void CertificationWidget::addCertification()
{
    m_tableContextMenu->close();
    QString certificationName = tr("New Certification");
    int adder = -1;
    bool certificationExists = false;
    do {
        adder++;
        certificationExists = false;
        for (int i = 0, n = ui->certifications->rowCount(); i < n; ++i) {
            if ((adder == 0 && ui->certifications->item(i, 0)->text() == certificationName)
                || (adder > 0
                    && ui->certifications->item(i, 0)->text() == QString("%1 %2").arg(certificationName).arg(adder))) {
                certificationExists = true;
                break;
            }
        }
    } while (certificationExists);

    if (adder > 0) {
        certificationName.append(QString(" %1").arg(adder));
    }

    m_addedCertifications << Certification(certificationName);

    ui->certifications->blockSignals(true);
    auto* item = new QTableWidgetItem(certificationName);
    item->setData(Qt::UserRole, certificationName);
    int row = ui->certifications->rowCount();
    ui->certifications->insertRow(row);
    ui->certifications->setItem(row, 0, item);
    ui->certifications->blockSignals(false);
}

/**
 * \brief Removes the current genre from all corresponding movies
 */
void CertificationWidget::deleteCertification()
{
    m_tableContextMenu->close();
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid row" << ui->certifications->currentRow();
        return;
    }

    const auto certification =
        Certification(ui->certifications->item(ui->certifications->currentRow(), 0)->data(Qt::UserRole).toString());
    ui->certifications->removeRow(ui->certifications->currentRow());

    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == certification) {
            movie->setCertification(Certification::NoCertification);
        }
    }
    m_addedCertifications.removeOne(certification);
}

/**
 * \brief Remove a movie from the current certification
 */
void CertificationWidget::removeMovie()
{
    if (ui->movies->currentRow() < 0 || ui->movies->currentRow() >= ui->movies->rowCount()) {
        qWarning() << "Invalid row" << ui->movies->currentRow();
        return;
    }
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid certifications row" << ui->certifications->currentRow();
        return;
    }

    auto* movie = ui->movies->item(ui->movies->currentRow(), 0)->data(Qt::UserRole).value<Movie*>();
    movie->setCertification(Certification::NoCertification);
    ui->movies->removeRow(ui->movies->currentRow());
}

void CertificationWidget::addMovie()
{
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid certification row" << ui->certifications->currentRow();
        return;
    }

    const auto cert = Certification(ui->certifications->item(ui->certifications->currentRow(), 0)->text());

    auto* listDialog = new MovieListDialog(this);
    const int exitCode = listDialog->execWithoutCertification(cert);
    QVector<Movie*> movies = listDialog->selectedMovies();
    listDialog->deleteLater();

    if (exitCode != QDialog::Accepted || movies.isEmpty()) {
        return;
    }

    for (Movie* movie : movies) {
        movie->setCertification(cert);
    }
    onCertificationSelected();
}

/**
 * \brief Saves all changed movies
 */
void CertificationWidget::onSaveInformation()
{
    for (Movie* movie : Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged()) {
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
        }
    }
    m_addedCertifications.clear();
    loadCertifications();
    NotificationBox::instance()->showSuccess(tr("All Movies Saved"));
}

void CertificationWidget::onJumpToMovie(QTableWidgetItem* item)
{
    auto* movie = item->data(Qt::UserRole).value<Movie*>();
    emit sigJumpToMovie(movie);
}
