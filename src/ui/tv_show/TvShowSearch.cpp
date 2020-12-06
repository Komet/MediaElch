#include "TvShowSearch.h"
#include "ui_TvShowSearch.h"

#include "globals/Manager.h"
#include "globals/Math.h"
#include "settings/Settings.h"
#include "ui/small_widgets/MyCheckBox.h"

using namespace mediaelch::scraper;

TvShowSearch::TvShowSearch(QWidget* parent) : QDialog(parent), ui(new Ui::TvShowSearch)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(ui->buttonClose, &QAbstractButton::clicked, this, &QDialog::reject);
    connect(ui->tvShowSearchWidget, &TvShowSearchWidget::sigResultClicked, this, &QDialog::accept);
}

TvShowSearch::~TvShowSearch()
{
    delete ui;
}

int TvShowSearch::execWithSearch(QString searchString)
{
    QSize newSize;
    newSize.setHeight(qMax(400, parentWidget()->size().height() - 200));
    newSize.setWidth(mediaelch::math::clamp(640, 840, parentWidget()->size().width() - 380));
    resize(newSize);

    ui->tvShowSearchWidget->search(searchString);
    return exec();
}

void TvShowSearch::setSearchType(TvShowType type)
{
    ui->tvShowSearchWidget->setSearchType(type);
}

QString TvShowSearch::showIdentifier()
{
    return ui->tvShowSearchWidget->showIdentifier();
}

mediaelch::scraper::TvScraper* TvShowSearch::scraper()
{
    return ui->tvShowSearchWidget->scraper();
}

SeasonOrder TvShowSearch::seasonOrder() const
{
    return ui->tvShowSearchWidget->seasonOrder();
}

const mediaelch::Locale& TvShowSearch::locale() const
{
    return ui->tvShowSearchWidget->locale();
}

const QSet<ShowScraperInfo>& TvShowSearch::showDetailsToLoad() const
{
    return ui->tvShowSearchWidget->showDetailsToLoad();
}

const QSet<EpisodeScraperInfo>& TvShowSearch::episodeDetailsToLoad() const
{
    return ui->tvShowSearchWidget->episodeDetailsToLoad();
}

TvShowUpdateType TvShowSearch::updateType() const
{
    return ui->tvShowSearchWidget->updateType();
}
