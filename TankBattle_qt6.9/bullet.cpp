#include "bullet.h"

Bullet::Bullet(Tank *tank, int unitSize): Entity(unitSize)
{
    int x = tank->getPos().x();
    int y = tank->getPos().y();
    this->speed = tank->getBulletSpeed(); // 设置子弹速度
    this->direction = tank->getDirection(); // 设置子弹方向
    this->tag = tank->getTag();
    owner = tank; // 设置子弹所有者
    picPath = ":/bullet/images/bullet_";
    switch (direction)
    {
    case Up:
        picPath += "up.png";
        shootRect = QRect(x, 0, 58, y);
        x += unitSize * 2 / 3; // 从坦克左上角修正到中心
        break;
    case Down:
        picPath += "down.png";
        shootRect = QRect(x, y, unitSize * 2, 59 * unitSize - y);
        x += unitSize * 2 / 3; // 从坦克左上角修正到中心
        break;
    case Left:
        picPath += "left.png";
        shootRect = QRect(0, y, x, unitSize * 2);
        y += unitSize * 2 / 3; // 从坦克左上角修正到中心
        break;
    case Right:
        picPath += "right.png";
        shootRect = QRect(x, y, 59 * unitSize - x, unitSize * 2);
        y += unitSize * 2 / 3; // 从坦克左上角修正到中心
        break;
    default:
        break;
    }
    rect = QRect(x, y, unitSize / 2, unitSize / 2); // 设置子弹矩形区域
    setPixmap(picPath);
}

void Bullet::move()
{
    switch (direction)
    {
    case Up: // 上
        rect.moveTop(rect.top() - speed);
        shootRect.moveTop(shootRect.top() - speed); // 更新射击矩形区域
        break;
    case Down: // 下
        rect.moveTop(rect.top() + speed);
        shootRect.moveTop(shootRect.top() + speed); // 更新射击矩形区域
        break;
    case Left: // 左
        rect.moveLeft(rect.left() - speed);
        shootRect.moveLeft(shootRect.left() - speed); // 更新射击矩形区域
        break;
    case Right: // 右
        rect.moveLeft(rect.left() + speed);
        shootRect.moveLeft(shootRect.left() + speed); // 更新射击矩形区域
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

int Bullet::getSpeed()
{
    return speed;
}

QRect Bullet::getShootRect()
{
    return shootRect;
}
