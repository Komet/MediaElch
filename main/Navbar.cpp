#include "Navbar.h"
#include "ui_Navbar.h"

#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include "globals/Helper.h"
#include "globals/Manager.h"
#include "settings/Settings.h"

Navbar::Navbar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Navbar)
{
    ui->setupUi(this);

    ui->btnSearch->setShortcut(QKeySequence::Find);
    ui->btnSearch->setToolTip(tr("Scrape (%1)").arg(QKeySequence(QKeySequence::Find).toString(QKeySequence::NativeText)));

    ui->btnSave->setShortcut(QKeySequence::Save);
    ui->btnSave->setToolTip(tr("Save (%1)").arg(QKeySequence(QKeySequence::Save).toString(QKeySequence::NativeText)));

    QKeySequence seqSaveAll(Qt::CTRL+Qt::ShiftModifier+Qt::Key_S);
    ui->btnSaveAll->setShortcut(seqSaveAll);
    ui->btnSaveAll->setToolTip(tr("Save All (%1)").arg(seqSaveAll.toString(QKeySequence::NativeText)));

    ui->btnReload->setShortcut(QKeySequence::Refresh);
    ui->btnReload->setToolTip(tr("Reload all files (%1)").arg(QKeySequence(QKeySequence::Refresh).toString(QKeySequence::NativeText)));

    connect(ui->btnSearch, SIGNAL(clicked()), this, SIGNAL(sigSearch()));
    connect(ui->btnSave, SIGNAL(clicked()), this, SIGNAL(sigSave()));
    connect(ui->btnSaveAll, SIGNAL(clicked()), this, SIGNAL(sigSaveAll()));
    connect(ui->btnReload, SIGNAL(clicked()), this, SIGNAL(sigReload()));
    connect(ui->btnRename, SIGNAL(clicked()), this, SIGNAL(sigRename()));
    connect(ui->btnSettings, SIGNAL(clicked()), this, SIGNAL(sigSettings()));
    connect(ui->btnSync, SIGNAL(clicked()), this, SIGNAL(sigSync()));
    connect(ui->btnExport, SIGNAL(clicked()), this, SIGNAL(sigExport()));
    connect(ui->btnAbout, SIGNAL(clicked()), this, SIGNAL(sigAbout()));
    connect(ui->btnDonate, SIGNAL(clicked()), this, SIGNAL(sigLike()));

    connect(ui->filterWidget, SIGNAL(sigFilterChanged(QList<Filter*>,QString)), this, SIGNAL(sigFilterChanged(QList<Filter*>,QString)));

    connect(Settings::instance(), SIGNAL(sigDonated(bool)), this, SLOT(onDonated(bool)));

    QList<QColor> navbarColors;
    navbarColors << QColor(241, 96, 106, 255);
    navbarColors << QColor(248, 155, 53, 255);
    navbarColors << QColor(248, 155, 53, 255);
    navbarColors << QColor(221, 222, 48, 255);
    navbarColors << QColor(106, 195, 133, 255);
    navbarColors << QColor(106, 195, 133, 255);
    navbarColors << QColor(107, 183, 228, 255);
    navbarColors << QColor(107, 183, 228, 255);
    navbarColors << QColor(206, 139, 188, 255);

    QStringList menuIcons = QStringList() << "scrape" << "save" << "saveall" << "rename" << "sync" << "export" << "reload" << "settings" << "about";

    int i=0;
    foreach (QToolButton *btn, ui->widget->findChildren<QToolButton*>()) {
        if (!btn->property("iconName").isValid())
            continue;
#ifndef Q_OS_MAC
        btn->setIconSize(QSize(32, 32));
        btn->setIcon(QIcon(":/menu/" + menuIcons.takeFirst()));
#else
        btn->setIcon(Manager::instance()->iconFont()->icon(btn->property("iconName").toString(),
                                                           navbarColors.at(i++%navbarColors.count()),
                                                           btn->property("iconPainter").toString(),
                                                           1.0
                                                           ));
#endif
    }

    if (Helper::instance()->devicePixelRatio(this) == 1) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
        effect->setColor(QColor(0, 0, 0, 30));
        effect->setOffset(2);
        effect->setBlurRadius(4);
        ui->btnDonate->setGraphicsEffect(effect);
    }
    ui->btnDonate->setVisible(!Settings::instance()->donated());
}

Navbar::~Navbar()
{
    delete ui;
}

void Navbar::setActionSearchEnabled(bool enabled)
{
    ui->btnSearch->setEnabled(enabled);
}

void Navbar::setActionSaveEnabled(bool enabled)
{
    ui->btnSave->setEnabled(enabled);
}

void Navbar::setActionSaveAllEnabled(bool enabled)
{
    ui->btnSaveAll->setEnabled(enabled);
}

void Navbar::setActionReloadEnabled(bool enabled)
{
    ui->btnReload->setEnabled(enabled);
}

void Navbar::setActionRenameEnabled(bool enabled)
{
    ui->btnRename->setEnabled(enabled);
}

void Navbar::setReloadToolTip(QString toolTip)
{
    ui->btnReload->setToolTip(toolTip);
}

void Navbar::setFilterWidgetEnabled(bool enabled)
{
    ui->filterWidget->setEnabled(enabled);
}

void Navbar::setActiveWidget(MainWidgets widget)
{
    ui->filterWidget->setActiveWidget(widget);
}

void Navbar::onDonated(bool donated)
{
    ui->btnDonate->setVisible(!donated);
}
