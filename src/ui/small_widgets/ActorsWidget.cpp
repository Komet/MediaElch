#include "ui/small_widgets/ActorsWidget.h"
#include "ui_ActorsWidget.h"

#include "globals/Helper.h"
#include "globals/Manager.h"
#include "model/ActorModel.h"

#include <QFileDialog>

ActorsWidget::ActorsWidget(QWidget* parent) : QWidget(parent), ui(new Ui::ActorsWidget)
{
    ui->setupUi(this);

    QFont font = ui->actorResolution->font();
#ifndef Q_OS_MAC
    font.setPointSize(font.pointSize() - 1);
#else
    font.setPointSize(font.pointSize() - 2);
#endif
    ui->actorResolution->setFont(font);

    m_actorModel = new ActorModel(this);
    ui->actors->setModel(m_actorModel);
    ui->actors->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->actors->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(ui->buttonAddActor, &QAbstractButton::clicked, this, &ActorsWidget::addActor);
    connect(ui->buttonRemoveActor, &QAbstractButton::clicked, this, &ActorsWidget::removeActor);
    connect(ui->actor, &MyLabel::clicked, this, &ActorsWidget::onChangeActorImage);

    connect(m_actorModel, &ActorModel::dataChanged, this, &ActorsWidget::onActorEdited);
    connect(ui->actors->selectionModel(),
        &QItemSelectionModel::selectionChanged,
        this,
        &ActorsWidget::onActorSelectionChanged);
}

ActorsWidget::~ActorsWidget()
{
    delete ui;
}

void ActorsWidget::setMovie(Movie* movie)
{
    m_movie = movie;
    m_actorModel->setActors(&movie->actors());
}

void ActorsWidget::clear()
{
    bool blocked = ui->actors->blockSignals(true);
    m_actorModel->setActors(nullptr);
    ui->actors->blockSignals(blocked);

    QPixmap pixmap(":/img/man.png");
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    ui->actor->setPixmap(pixmap);

    ui->actorResolution->setText("");
}

void ActorsWidget::addActor()
{
    Actor a;
    a.name = tr("Unknown Actor");
    a.role = tr("Unknown Role");

    m_actorModel->addActor(a);

    ui->actors->scrollToBottom();
    ui->actors->selectRow(m_actorModel->rowCount() - 1);
    emit actorsChanged();
}

void ActorsWidget::removeActor()
{
    QModelIndex index = ui->actors->selectionModel()->currentIndex();
    if (!ui->actors->selectionModel()->hasSelection() || !index.isValid()) {
        qCInfo(generic) << "[ActorsWidget] Cannot remove actor because none is selected!";
        return;
    }

    m_actorModel->removeRows(index.row(), 1);
    emit actorsChanged();
}

void ActorsWidget::onActorEdited()
{
    emit actorsChanged();
}

void ActorsWidget::onActorSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    Q_UNUSED(deselected)

    if (selected.count() <= 0) {
        updateActorImage(nullptr);
        return;
    }

    QModelIndex currentIndex = selected.first().indexes().first();
    if (!currentIndex.isValid()) {
        updateActorImage(nullptr);
        return;
    }

    Actor* actor = m_actorModel->data(currentIndex, ActorModel::ActorRole).value<Actor*>();
    updateActorImage(actor);
}

void ActorsWidget::updateActorImage(Actor* actor)
{
    const auto resetImage = [this]() {
        QPixmap pixmap(":/img/man.png");
        pixmap.setDevicePixelRatio(devicePixelRatioF());
        ui->actor->setPixmap(pixmap);
        ui->actorResolution->setText("");
    };

    if (actor == nullptr) {
        resetImage();
        return;
    }

    const auto usePixmap = [this](QPixmap& p) {
        ui->actorResolution->setText(QStringLiteral("%1 x %2").arg(p.width()).arg(p.height()));
        p = p.scaled(QSize(120, 180) * devicePixelRatioF(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        p.setDevicePixelRatio(devicePixelRatioF());
        ui->actor->setPixmap(p);
    };

    if (!actor->image.isNull()) {
        QPixmap p = QPixmap::fromImage(QImage::fromData(actor->image));
        usePixmap(p);

    } else if (m_movie != nullptr
               && !Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor).isEmpty()) {
        QPixmap p(Manager::instance()->mediaCenterInterface()->actorImageName(m_movie, *actor));
        usePixmap(p);

    } else {
        resetImage();
    }
}

void ActorsWidget::onChangeActorImage()
{
    static QString lastPath = QDir::homePath();

    QModelIndex index = ui->actors->selectionModel()->currentIndex();
    if (!ui->actors->selectionModel()->hasSelection() || !index.isValid()) {
        return;
    }

    emit actorsChanged();

    QString fileName = QFileDialog::getOpenFileName(parentWidget(),
        tr("Choose Image"), //
        lastPath,
        tr("Images (*.jpg *.jpeg *.png)"));
    if (fileName.isNull()) {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    // TODO: We should probably do this through the model.
    //       As no name/role has changed, no dataChanged() signal is required, yet.
    auto* actor = m_actorModel->data(index, ActorModel::ActorRole).value<Actor*>();
    actor->image = file.readAll();
    actor->imageHasChanged = true;
    emit actorsChanged();

    updateActorImage(actor);

    file.close();

    QFileInfo fi(fileName);
    lastPath = fi.absolutePath();
}
