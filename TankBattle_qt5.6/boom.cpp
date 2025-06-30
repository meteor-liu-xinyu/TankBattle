#include "boom.h"
#include "ui_boom.h"

Boom::Boom(QRect rect, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Boom)
{
    ui->setupUi(this);

    picpath = ":/boom/images/boom_";
    this->rect = rect;
    tick = -1;
    timer = new QTimer(this);
    timer->start(100);
    connect(timer, &QTimer::timeout, this, &Boom::destroyBoom);
}

Boom::~Boom()
{
    rect = QRect(0, 0, 0, 0);
    delete ui;
}

// 爆炸动画
void Boom::destroyBoom()
{
    if (tick < 0)
    {
        tick++;
        return;
    }
    else if (tick == 6)
    {
        timer->stop();
        emit boomFinished(); // 发送信号，通知爆炸结束
    }
    else
    {
        // 更新图片
        QString path = picpath + QString::number(tick) + ".png";
        entityPixmap = QPixmap(path);
        entityPixmap = entityPixmap.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    tick++;
}
