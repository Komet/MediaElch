#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include <QFileDialog>
#include <QListWidgetItem>

#include "Manager.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings()) {
            QWidget *scraperSettings = scraper->settingsWidget();
            scraperSettings->setParent(ui->tabScrapers);
            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->tabScrapers));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
            QFrame *line = new QFrame(ui->tabScrapers);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            ui->verticalLayoutScrapers->addWidget(line);
        }
    }
    ui->verticalLayoutScrapers->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(ui->buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->buttonAddMovieDir, SIGNAL(clicked()), this, SLOT(addMovieDir()));
    connect(ui->buttonRemoveMovieDir, SIGNAL(clicked()), this, SLOT(removeMovieDir()));
    connect(ui->listMovieDirs, SIGNAL(currentRowChanged(int)), this, SLOT(movieListRowChanged(int)));

    this->loadSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

SettingsDialog *SettingsDialog::instance(QWidget *parent)
{
    static SettingsDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new SettingsDialog(parent);
    }
    return m_instance;
}

int SettingsDialog::exec()
{
    this->loadSettings();
    return QDialog::exec();
}

void SettingsDialog::loadSettings()
{
    // Globals
    m_firstTime = m_settings.value("FirstTime", true).toBool();
    m_mainWindowSize = m_settings.value("MainWindowSize").toSize();
    m_mainWindowPosition = m_settings.value("MainWindowPosition").toPoint();

    // Movie Directories
    m_movieDirectories = m_settings.value("Directories/Movies").toStringList();
    ui->listMovieDirs->clear();
    foreach (const QString &dir, m_movieDirectories) {
        new QListWidgetItem(dir, ui->listMovieDirs);
    }
    ui->buttonRemoveMovieDir->setEnabled(!m_movieDirectories.isEmpty());

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->loadSettings();
    }
}

void SettingsDialog::accept()
{
    m_settings.setValue("FirstTime", false);
    m_settings.setValue("Directories/Movies", m_movieDirectories);
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }
    Manager::instance()->movieFileSearcher()->setMovieDirectories(this->movieDirectories());

    QDialog::accept();
}

void SettingsDialog::addMovieDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your movies"), QDir::homePath());
    if (!dir.isEmpty()) {
        if( !m_movieDirectories.contains(dir) ) {
            m_movieDirectories.append(dir);
            new QListWidgetItem(dir, ui->listMovieDirs);
        }
    }
}

void SettingsDialog::removeMovieDir()
{
    int row = ui->listMovieDirs->currentRow();
    if (row < 0) {
        return;
    }

    m_movieDirectories.removeOne(ui->listMovieDirs->currentItem()->text());
    delete ui->listMovieDirs->takeItem(ui->listMovieDirs->currentRow());
}

void SettingsDialog::movieListRowChanged(int currentRow)
{
    ui->buttonRemoveMovieDir->setDisabled(currentRow < 0);
}

/*** GETTER ***/

QSize SettingsDialog::mainWindowSize()
{
    return m_mainWindowSize;
}

QPoint SettingsDialog::mainWindowPosition()
{
    return m_mainWindowPosition;
}

QStringList SettingsDialog::movieDirectories()
{
    return m_movieDirectories;
}

bool SettingsDialog::firstTime()
{
    return m_firstTime;
}

/*** SETTER ***/

void SettingsDialog::setMainWindowSize(QSize mainWindowSize)
{
    m_mainWindowSize = mainWindowSize;
    m_settings.setValue("MainWindowSize", mainWindowSize);
}

void SettingsDialog::setMainWindowPosition(QPoint mainWindowPosition)
{
    m_mainWindowPosition = mainWindowPosition;
    m_settings.setValue("MainWindowPosition", mainWindowPosition);
}
