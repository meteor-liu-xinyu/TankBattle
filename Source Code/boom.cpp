#include "boom.h"

Boom::Boom(QRect rect, int unitSize, QObject *parent)
    : QObject{parent}, Entity(unitSize)
{
    picRootPath = ":/boom/images/boom_";
    this->rect = rect;
    tick = -1;
    timer = new QTimer(this);
    timer->start(100);
    connect(timer, &QTimer::timeout, this, &Boom::destroyBoom);
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
        setPixmap(picRootPath + QString::number(tick) + ".png");
    }
    tick++;
}
