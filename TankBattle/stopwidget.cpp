#include "stopwidget.h"
#include "ui_stopwidget.h"

StopWidget::StopWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StopWidget)
{
    ui->setupUi(this);

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect();
    opacityEffect->setOpacity(1.0); // 设置透明度为100%
    ui->label->setGraphicsEffect(opacityEffect); // 设置不透明
    ui->continueButton->setGraphicsEffect(opacityEffect); // 设置不透明
    ui->restartButton->setGraphicsEffect(opacityEffect); // 设置不透明

    this->move(parent->x() + 50, parent->y() + 400); // 设置位置

    ui->continueButton->clearFocus(); // 清除焦点
    ui->restartButton->clearFocus(); // 清除焦点

}

StopWidget::~StopWidget()
{
    delete ui;
}

void StopWidget::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Escape) // ESC键暂停
    {
        on_continueButton_clicked();
    }
    else if (k->key() == Qt::Key_R) // R键重新开始
    {
        on_restartButton_clicked();
    }
}

void StopWidget::on_continueButton_clicked()
{
    this->hide();
    emit continuesgn(); // 继续游戏
}

void StopWidget::on_restartButton_clicked()
{
    this->hide();
    emit restart(); // 重新开始游戏
}
