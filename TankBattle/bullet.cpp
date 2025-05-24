#include "bullet.h"

#define UNIT 24

Bullet::Bullet(Tank *tank, int speedBase)
{
    int x = tank->getPos().x();
    int y = tank->getPos().y();
    this->speed = tank->getBulletSpeed() * speedBase; // 设置子弹速度
    this->direction = tank->getDirection(); // 设置子弹方向，0表示上，1表示下，2表示左，3表示右
    this->tag = tank->getTag();
    owner = tank;
    QString picpath = ":/bullet/images/bullet_";
    switch (direction)
    {
    case Up:
        picpath += "up.png";
        shootRect = QRect(x, 0, 58, y);
        x += 16;
        break;
    case Down:
        picpath += "down.png";
        shootRect = QRect(x, y, 48, 59 * UNIT - y);
        x += 16;
        break;
    case Left:
        picpath += "left.png";
        shootRect = QRect(0, y, x, 48);
        y += 16;
        break;
    case Right:
        picpath += "right.png";
        shootRect = QRect(x, y, 59 * UNIT - x, 48);
        y += 16;
        break;
    default:
        break;
    }
    rect = QRect(x, y, 12, 12); // 设置子弹矩形区域
    entityPixmap = QPixmap(picpath); // 加载子弹图片
}

void Bullet::move()
{
    switch (direction)
    {
    case Up: // 上
        rect.moveTop(rect.top() - speed);
        shootRect.moveTop(shootRect.top() - speed);
        break;
    case Down: // 下
        rect.moveTop(rect.top() + speed);
        shootRect.moveTop(shootRect.top() + speed);
        break;
    case Left: // 左
        rect.moveLeft(rect.left() - speed);
        shootRect.moveLeft(shootRect.left() - speed);
        break;
    case Right: // 右
        rect.moveLeft(rect.left() + speed);
        shootRect.moveLeft(shootRect.left() + speed);
        break;
    default:
        break;
    }
}

Direction Bullet::getDirection()
{
    return direction;
}

Tank *Bullet::getOwner()
{
    return owner;
}

Bullet::~Bullet()
{
    rect = QRect(0, 0, 0, 0); // 清空矩形区域
    timer->stop(); // 停止定时器
    delete timer; // 删除定时器
}

int Bullet::getSpeed()
{
    return speed;
}

QRect Bullet::getShootRect()
{
    return shootRect;
}
