#include "gameoverui.h"
#include "ui_gameoverui.h"

GameoverUI::GameoverUI(QWidget *parent, bool ifwin) :
    QWidget(parent),
    ui(new Ui::GameoverUI)
{
    ui->setupUi(this);

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect();
    opacityEffect->setOpacity(1.0); // 设置透明度为100%
    ui->label->setGraphicsEffect(opacityEffect);

    this->move(parent->x() + 30, parent->y() + 400);

    if (ifwin)
    {
        ui->label->setText("<html><head/><body><p align=\"center\"><span style=\" font-size:48pt; font-weight:600; color:#ffffff;\">VICTORY</span></p></body></html>");
    }
}

GameoverUI::~GameoverUI()
{
    delete ui;
}

void GameoverUI::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_R)
    {
        emit restart();
        this->close();
    }
}
