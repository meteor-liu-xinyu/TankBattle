#ifndef BULLET_H
#define BULLET_H

#include <QPixmap>
#include <QRect>
#include <QPainter>

class Tank; // 前向声明

#include "entity.h"
#include "tank.h"

class Bullet : public Entity
{
public:
    Bullet(Tank *tank, int unitSize);
    void move();
    Direction getDirection(); // 获取子弹方向
    Tank* getOwner(); // 获取子弹所有者
    int getSpeed(); // 获取子弹速度
    QRect getShootRect(); // 获取子弹射击矩形区域

private:
    int speed; // 子弹速度
    Direction direction; // 子弹方向
    Tank *owner; // 子弹所有者
    QRect shootRect; // 子弹射击矩形区域
};

#endif // BULLET_H
