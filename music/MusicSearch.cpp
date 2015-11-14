#include "MusicSearch.h"
#include "ui_MusicSearch.h"

MusicSearch::MusicSearch(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MusicSearch)
{
    ui->setupUi(this);

#ifdef Q_OS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MusicSearch { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->musicSearchWidget, SIGNAL(sigResultClicked()), this, SLOT(accept()));
}

MusicSearch::~MusicSearch()
{
    delete ui;
}

MusicSearch* MusicSearch::instance(QWidget *parent)
{
    static MusicSearch *m_instance = 0;
    if (!m_instance)
        m_instance = new MusicSearch(parent);
    return m_instance;
}

int MusicSearch::exec(QString type, QString searchString, QString artistName)
{
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-200);
    newSize.setWidth(qMin(700, parentWidget()->size().width()-200));
    resize(newSize);
    ui->musicSearchWidget->setType(type);
    ui->musicSearchWidget->setArtistName(artistName);
    ui->musicSearchWidget->search(searchString);
    return QDialog::exec();
}

int MusicSearch::exec()
{
    return 0;
}

int MusicSearch::scraperNo()
{
    return ui->musicSearchWidget->scraperNo();
}

QString MusicSearch::scraperId()
{
    return ui->musicSearchWidget->scraperId();
}

QString MusicSearch::scraperId2()
{
    return ui->musicSearchWidget->scraperId2();
}

QList<int> MusicSearch::infosToLoad()
{
    return ui->musicSearchWidget->infosToLoad();
}
