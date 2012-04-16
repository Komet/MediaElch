#include "SettingsWidget.h"
#include "ui_SettingsWidget.h"
#include <QFileDialog>
#include "Manager.h"
#include "MessageBox.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    QFont font = ui->labelMovies->font();
    font.setPointSize(font.pointSize()+3);
    ui->labelMovies->setFont(font);
    ui->labelScrapers->setFont(font);

    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings()) {
            QWidget *scraperSettings = scraper->settingsWidget();
            scraperSettings->setParent(ui->groupBox_2);
            ui->verticalLayoutScrapers->addWidget(new QLabel(scraper->name(), ui->groupBox_2));
            ui->verticalLayoutScrapers->addWidget(scraperSettings);
            QFrame *line = new QFrame(ui->groupBox_2);
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            ui->verticalLayoutScrapers->addWidget(line);
        }
    }
    ui->verticalLayoutScrapers->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

    connect(ui->buttonAddMovieDir, SIGNAL(clicked()), this, SLOT(addMovieDir()));
    connect(ui->buttonRemoveMovieDir, SIGNAL(clicked()), this, SLOT(removeMovieDir()));
    connect(ui->listMovieDirs, SIGNAL(currentRowChanged(int)), this, SLOT(movieListRowChanged(int)));

    this->loadSettings();
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::loadSettings()
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

void SettingsWidget::saveSettings()
{
    m_settings.setValue("FirstTime", false);
    m_settings.setValue("Directories/Movies", m_movieDirectories);
    foreach (ScraperInterface *scraper, Manager::instance()->scrapers()) {
        if (scraper->hasSettings())
            scraper->saveSettings();
    }
    Manager::instance()->movieFileSearcher()->setMovieDirectories(this->movieDirectories());
    MessageBox::instance()->showMessage(tr("Settings saved"));
}

void SettingsWidget::addMovieDir()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a directory containing your movies"), QDir::homePath());
    if (!dir.isEmpty()) {
        if( !m_movieDirectories.contains(dir) ) {
            m_movieDirectories.append(dir);
            new QListWidgetItem(dir, ui->listMovieDirs);
        }
    }
}

void SettingsWidget::removeMovieDir()
{
    int row = ui->listMovieDirs->currentRow();
    if (row < 0) {
        return;
    }

    m_movieDirectories.removeOne(ui->listMovieDirs->currentItem()->text());
    delete ui->listMovieDirs->takeItem(ui->listMovieDirs->currentRow());
}

void SettingsWidget::movieListRowChanged(int currentRow)
{
    ui->buttonRemoveMovieDir->setDisabled(currentRow < 0);
}

/*** GETTER ***/

QSize SettingsWidget::mainWindowSize()
{
    return m_mainWindowSize;
}

QPoint SettingsWidget::mainWindowPosition()
{
    return m_mainWindowPosition;
}

QStringList SettingsWidget::movieDirectories()
{
    return m_movieDirectories;
}

bool SettingsWidget::firstTime()
{
    return m_firstTime;
}

/*** SETTER ***/

void SettingsWidget::setMainWindowSize(QSize mainWindowSize)
{
    int height = mainWindowSize.height();
    // don't know where 42px are left...
#ifdef Q_WS_MAC
    height += 42;
#endif
    mainWindowSize.setHeight(height);
    m_mainWindowSize = mainWindowSize;
    m_settings.setValue("MainWindowSize", mainWindowSize);
}

void SettingsWidget::setMainWindowPosition(QPoint mainWindowPosition)
{
    m_mainWindowPosition = mainWindowPosition;
    m_settings.setValue("MainWindowPosition", mainWindowPosition);
}
