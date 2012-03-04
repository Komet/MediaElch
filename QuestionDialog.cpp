#include "QuestionDialog.h"
#include "ui_QuestionDialog.h"

#include <QPainter>

QuestionDialog::QuestionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QuestionDialog)
{
    ui->setupUi(this);
    Qt::WindowFlags flags = windowFlags();
#ifdef Q_WS_WIN
    flags |= Qt::Dialog;
#else
    flags |= Qt::SplashScreen;
    setStyleSheet(styleSheet() + " #MovieImageDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#endif
    setWindowFlags(flags);

    QPainter p;
    QImage img(":/img/warning1.png");
    p.begin(&img);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(img.rect(), QColor(255, 255, 255, 140));
    p.end();
    ui->labelIcon->setPixmap(QPixmap::fromImage(img));

    connect(ui->buttonBack, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->buttonDiscard, SIGNAL(clicked()), this, SLOT(accept()));
}

QuestionDialog::~QuestionDialog()
{
    delete ui;
}

QuestionDialog *QuestionDialog::instance(QWidget *parent)
{
    static QuestionDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new QuestionDialog(parent);
    }
    return m_instance;
}

int QuestionDialog::exec()
{
    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    return QDialog::exec();
}
