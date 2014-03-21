#include "CertificationWidget.h"
#include "ui_CertificationWidget.h"

#include "globals/Helper.h"
#include "globals/LocaleStringCompare.h"
#include "globals/Manager.h"
#include "notifications/NotificationBox.h"
#include "sets/MovieListDialog.h"

/**
 * @brief CertificationWidget::CertificationWidget
 * @param parent
 */
CertificationWidget::CertificationWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CertificationWidget)
{
    ui->setupUi(this);
    ui->certifications->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

#ifdef Q_OS_MAC
    QFont certificationsFont = ui->certifications->font();
    certificationsFont.setPointSize(certificationsFont.pointSize()-2);
    ui->certifications->setFont(certificationsFont);
#endif

#ifndef Q_OS_MAC
    QFont nameFont = ui->certificationName->font();
    nameFont.setPointSize(nameFont.pointSize()-4);
    ui->certificationName->setFont(nameFont);
#endif

    ui->certifications->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableContextMenu = new QMenu(this);
    QAction *actionAddCertification = new QAction(tr("Add Certification"), this);
    QAction *actionDeleteCertification = new QAction(tr("Delete Certification"), this);
    m_tableContextMenu->addAction(actionAddCertification);
    m_tableContextMenu->addAction(actionDeleteCertification);
    connect(actionAddCertification, SIGNAL(triggered()), this, SLOT(addCertification()));
    connect(actionDeleteCertification, SIGNAL(triggered()), this, SLOT(deleteCertification()));
    connect(ui->certifications, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCertificationsContextMenu(QPoint)));

    connect(ui->certifications, SIGNAL(itemSelectionChanged()), this, SLOT(onCertificationSelected()));
    connect(ui->certifications, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onCertificationNameChanged(QTableWidgetItem*)));
    connect(ui->movies, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(onJumpToMovie(QTableWidgetItem*)));

    Helper::instance()->applyStyle(ui->groupBox);
}

/**
 * @brief CertificationWidget::~GenreWidget
 */
CertificationWidget::~CertificationWidget()
{
    delete ui;
}

/**
 * @brief Executes the certifications table context menu
 * @param point Point where the menu will be shown
 */
void CertificationWidget::showCertificationsContextMenu(QPoint point)
{
    m_tableContextMenu->exec(ui->certifications->mapToGlobal(point));
}

/**
 * @brief Returns the splitter
 * @return The splitter
 */
QSplitter *CertificationWidget::splitter()
{
    return ui->splitter;
}

/**
 * @brief Clears the genres table
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
 * @brief Loads all certifications from movies and fills the certifications table
 */
void CertificationWidget::loadCertifications()
{
    emit setActionSaveEnabled(false, WidgetCertifications);
    ui->certifications->blockSignals(true);
    clear();
    QStringList certifications;
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (!movie->certification().isEmpty() && !certifications.contains(movie->certification()))
            certifications.append(movie->certification());
    }

    foreach (const QString &certification, m_addedCertifications) {
        if (!certification.isEmpty() && !certifications.contains(certification))
            certifications.append(certification);
    }

    qSort(certifications.begin(), certifications.end(), LocaleStringCompare());

    foreach (const QString &certification, certifications) {
        QTableWidgetItem *item = new QTableWidgetItem(certification);
        item->setData(Qt::UserRole, certification);
        int row = ui->certifications->rowCount();
        ui->certifications->insertRow(row);
        ui->certifications->setItem(row, 0, item);
    }

    ui->certifications->blockSignals(false);
    emit setActionSaveEnabled(true, WidgetCertifications);
}

/**
 * @brief Fills the movies table with movies
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

    QString certificationName = ui->certifications->item(ui->certifications->currentRow(), 0)->text();
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == certificationName) {
            int row = ui->movies->rowCount();
            QTableWidgetItem *item = new QTableWidgetItem(movie->name());
            item->setData(Qt::UserRole, QVariant::fromValue(movie));
            ui->movies->insertRow(row);
            ui->movies->setItem(row, 0, item);
        }
    }
    ui->certificationName->setText(certificationName);
    ui->movies->setSortingEnabled(true);
    ui->movies->sortByColumn(0, Qt::AscendingOrder);
}

/**
 * @brief Renames a certification
 * @param item Changed item in certifications table
 */
void CertificationWidget::onCertificationNameChanged(QTableWidgetItem *item)
{
    QString newName = item->text();
    QString origName = item->data(Qt::UserRole).toString();
    if (newName == origName)
        return;

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == origName)
            movie->setCertification(newName);
    }
    ui->certificationName->setText(newName);
    item->setData(Qt::UserRole, newName);
    if (m_addedCertifications.contains(origName)) {
        m_addedCertifications.removeOne(origName);
        if (!m_addedCertifications.contains(newName))
            m_addedCertifications.append(newName);
    }
    loadCertifications();
}

void CertificationWidget::addCertification()
{
    m_tableContextMenu->close();
    QString certificationName = tr("New Certification");
    int adder = -1;
    bool certificationExists;
    do {
        adder++;
        certificationExists = false;
        for (int i=0, n=ui->certifications->rowCount() ; i<n ; ++i) {
            if ((adder == 0 && ui->certifications->item(i, 0)->text() == certificationName) ||
                (adder > 0 && ui->certifications->item(i, 0)->text() == QString("%1 %2").arg(certificationName).arg(adder))) {
                certificationExists = true;
                break;
            }
        }
    } while(certificationExists);

    if (adder > 0)
        certificationName.append(QString(" %1").arg(adder));

    m_addedCertifications << certificationName;

    ui->certifications->blockSignals(true);
    QTableWidgetItem *item = new QTableWidgetItem(certificationName);
    item->setData(Qt::UserRole, certificationName);
    int row = ui->certifications->rowCount();
    ui->certifications->insertRow(row);
    ui->certifications->setItem(row, 0, item);
    ui->certifications->blockSignals(false);
}

/**
 * @brief Removes the current genre from all corresponding movies
 */
void CertificationWidget::deleteCertification()
{
    m_tableContextMenu->close();
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid row" << ui->certifications->currentRow();
        return;
    }

    QString certificationName = ui->certifications->item(ui->certifications->currentRow(), 0)->data(Qt::UserRole).toString();
    QString origCertificationName = ui->certifications->item(ui->certifications->currentRow(), 0)->data(Qt::UserRole).toString();
    ui->certifications->removeRow(ui->certifications->currentRow());

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == certificationName)
            movie->setCertification("");
    }
    m_addedCertifications.removeOne(origCertificationName);
}

/**
 * @brief Remove a movie from the current certification
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

    Movie *movie = ui->movies->item(ui->movies->currentRow(), 0)->data(Qt::UserRole).value<Movie*>();
    movie->setCertification("");
    ui->movies->removeRow(ui->movies->currentRow());
}

/**
 * @brief Add a movie to the current certification
 */
void CertificationWidget::addMovie()
{
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid certification row" << ui->certifications->currentRow();
        return;
    }


    if (MovieListDialog::instance()->execWithoutCertification(ui->certifications->item(ui->certifications->currentRow(), 0)->text()) == QDialog::Accepted) {
        QList<Movie*> movies = MovieListDialog::instance()->selectedMovies();
        if (movies.isEmpty())
            return;

        QString certificationName = ui->certifications->item(ui->certifications->currentRow(), 0)->text();
        foreach (Movie *movie, movies)
            movie->setCertification(certificationName);
        onCertificationSelected();
    }
}

/**
 * @brief Saves all changed movies
 */
void CertificationWidget::onSaveInformation()
{
    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->hasChanged())
            movie->controller()->saveData(Manager::instance()->mediaCenterInterface());
    }
    m_addedCertifications.clear();
    loadCertifications();
    NotificationBox::instance()->showMessage(tr("All Movies Saved"));
}

void CertificationWidget::onJumpToMovie(QTableWidgetItem *item)
{
    Movie *movie = item->data(Qt::UserRole).value<Movie*>();
    emit sigJumpToMovie(movie);
}
