#ifndef BULLET_H
#define BULLET_H

#include <QPixmap>
#include <QRect>
#include <QPainter>

#include "entity.h"
#include "tank.h"

class Tank; // 前向声明

class Bullet : public Entity
{
public:
    Bullet(Tank *tank, int speedBase);
    void move();
    Direction getDirection();
    ~Bullet();
    Tank* getOwner(); // 获取子弹所有者
    int getSpeed();
    QRect getShootRect();

private:
    int speed; // 子弹速度
    Direction direction; // 子弹方向
    Tank *owner;
    QRect shootRect;
};

#endif // BULLET_H
