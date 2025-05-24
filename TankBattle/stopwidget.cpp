#include "stopwidget.h"
#include "ui_stopwidget.h"

StopWidget::StopWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StopWidget)
{
    ui->setupUi(this);

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect();
    opacityEffect->setOpacity(1.0); // 设置透明度为100%
    ui->label->setGraphicsEffect(opacityEffect);
    ui->continueButton->setGraphicsEffect(opacityEffect);
    ui->restartButton->setGraphicsEffect(opacityEffect);

    this->move(parent->x() + 50, parent->y() + 400);

    ui->continueButton->clearFocus();
    ui->restartButton->clearFocus();

}

StopWidget::~StopWidget()
{
    delete ui;
}

void StopWidget::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Escape)
    {
        on_continueButton_clicked();
    }
    else if (k->key() == Qt::Key_R)
    {
        on_restartButton_clicked();
    }
}

void StopWidget::on_continueButton_clicked()
{
    this->hide();
    emit continuesgn();
}

void StopWidget::on_restartButton_clicked()
{
    emit restart();
    this->hide();
}
