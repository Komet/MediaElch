#include "CertificationWidget.h"
#include "ui_CertificationWidget.h"

#include "globals/Manager.h"
#include "main/MessageBox.h"
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
#if QT_VERSION >= 0x050000
    ui->certifications->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
    ui->certifications->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->movies->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
    QFont font = ui->certificationName->font();
    font.setPointSize(font.pointSize()+4);
    ui->certificationName->setFont(font);

#ifdef Q_OS_MAC
    QFont certificationsFont = ui->certifications->font();
    certificationsFont.setPointSize(certificationsFont.pointSize()-2);
    ui->certifications->setFont(certificationsFont);
#endif

    ui->certifications->setContextMenuPolicy(Qt::CustomContextMenu);
    m_tableContextMenu = new QMenu(this);
    QAction *actionDeleteGenre = new QAction(tr("Delete Certification"), this);
    m_tableContextMenu->addAction(actionDeleteGenre);
    connect(actionDeleteGenre, SIGNAL(triggered()), this, SLOT(deleteCertification()));
    connect(ui->certifications, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showCertificationsContextMenu(QPoint)));

    connect(ui->certifications, SIGNAL(itemSelectionChanged()), this, SLOT(onCertificationSelected()));
    connect(ui->certifications, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onCertificationNameChanged(QTableWidgetItem*)));
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
    certifications.sort();

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
    loadCertifications();
}

/**
 * @brief Removes the current genre from all corresponding movies
 */
void CertificationWidget::deleteCertification()
{
    if (ui->certifications->currentRow() < 0 || ui->certifications->currentRow() >= ui->certifications->rowCount()) {
        qWarning() << "Invalid row" << ui->certifications->currentRow();
        return;
    }

    QString certificationName = ui->certifications->item(ui->certifications->currentRow(), 0)->data(Qt::UserRole).toString();
    ui->certifications->removeRow(ui->certifications->currentRow());

    foreach (Movie *movie, Manager::instance()->movieModel()->movies()) {
        if (movie->certification() == certificationName)
            movie->setCertification("");
    }
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
        Movie *movie = MovieListDialog::instance()->selectedMovie();
        QString certificationName = ui->certifications->item(ui->certifications->currentRow(), 0)->text();
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
            movie->saveData(Manager::instance()->mediaCenterInterface());
    }

    loadCertifications();
    MessageBox::instance()->showMessage(tr("All Movies Saved"));
}
