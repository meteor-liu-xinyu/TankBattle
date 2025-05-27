#include "tank.h"
#define UNIT 24

Tank::Tank(int x, int y, TankType tanktype, int level, int speed, int num)
{
    this->level = level;
    this->speed = speed;
    this->num = num;
    rect = QRect(x, y, 40, 40);
    nextPos = QPoint(x/UNIT, y/UNIT);
    temptargetPos = nextPos;
    picpath = ":/tank/images/";
    if (tanktype == PlayerTank)
    {
        picpath += "tank_L" + QString::number(level) + "_";
        tag = "-1";
        health = level + 2;
        shell = 2;
    }
    else
    {
        picpath += "enemy_" + QString::number(level) + "_";
        tag = QString::number(num);
        if (level == 3)
        {
            health = 2;
        }
    }

    for (int i = 1; i <= 3; i++)
    {
        tankAppearPaths << ":/tank/images/appear_" + QString::number(i) + ".png";
    }
    timer = new QTimer(this);
    timer->start(100);
    tick = -5;
    connect(timer, &QTimer::timeout, this, &Tank::appear); // 出现动画
}

void Tank::moveUp()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x(), rect.y() - speed);
    direction = Up;
    entityPixmap = QPixmap(picpath + "1.png");
}

void Tank::moveDown()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x(), rect.y() + speed);
    direction = Down;
    entityPixmap = QPixmap(picpath + "2.png");
}

void Tank::moveLeft()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x() - speed, rect.y());
    direction = Left;
    entityPixmap = QPixmap(picpath + "3.png");
}

void Tank::moveRight()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x() + speed, rect.y());
    direction = Right;
    entityPixmap = QPixmap(picpath + "4.png");
}

void Tank::appear()
{
    if (tick < 0)
    {
        tick++;
        return;
    }
    if (tick == 3)
    {
        timer->stop();
        delete timer;
        timer = nullptr;
        entityPixmap = QPixmap(picpath + "1.png");
    }
    else
    {
        QString path = tankAppearPaths[tick];
        entityPixmap = QPixmap(path);
    }

    tick++;
}

void Tank::changeDirection(const Direction &dir)
{
    if (tick < 3)
    {
        return;
    }
    if (direction != dir)
    {
        direction = dir;
        switch (dir)
        {
        case Up:
            entityPixmap = QPixmap(picpath + "1.png");
            break;
        case Down:
            entityPixmap = QPixmap(picpath + "2.png");
            break;
        case Left:
            entityPixmap = QPixmap(picpath + "3.png");
            break;
        case Right:
            entityPixmap = QPixmap(picpath + "4.png");
            break;
        default:
            break;
        }
    }
}

Direction Tank::getDirection()
{
    return direction;
}

Tank::~Tank()
{
    rect = QRect(0, 0, 0, 0);
}

void Tank::setBullet(Bullet *bullet)
{
    bullets.append(bullet);
    if (shell != -1)
    {
        shell--;
    }
}

void Tank::removeBullet(Bullet *bullet)
{
    bullets.removeOne(bullet);
    if (shell != -1)
    {
        shell++;
    }
}

int Tank::getRemainBulletNum()
{
    return shell;
}

int Tank::getBulletSpeed()
{
    return bulletSpeed;
}

int Tank::getHealth()
{
    return health;
}

int Tank::getLevel()
{
    return level;
}

void Tank::setDistance(int distance)
{
    this->distance = distance;
}

int Tank::getDistance()
{
    return distance;
}

void Tank::healthDecrease()
{
    health--;
}

void Tank::setNextPos(QPoint nextPos)
{
    this->nextPos = nextPos;
}

QPoint Tank::getNextPos()
{
    return nextPos;
}

int Tank::getnum()
{
    return num;
}

void Tank::setUnbeatable(bool unbeatable) // 设置无敌状态
{
    if (unbeatable)
    {
        this->health = -1;
    }
    else
    {
        if (tag == "-1")
        {
            this->health = level + 2;
        }
        else
        {
            this->health = 1;
            if (level == 3)
            {
                health = 2;
            }
        }
    }
}

void Tank::setTargetPos(QPoint targetPos)
{
    this->targetPos = targetPos;
}

QPoint Tank::getTargetPos()
{
    return targetPos;
}

void Tank::setTempTargetPos(QPoint temptargetPos)
{
    this->temptargetPos = temptargetPos;
}

QPoint Tank::getTempTargetPos()
{
    return temptargetPos;
}

void Tank::upgrade()
{
    level++;
    health = level + 2;
    bulletSpeed++;
}

void Tank::setShellInf(bool flag) // 设置无限子弹
{
    if (flag)
    {
        shell = -1;
    }
    else
    {
        shell = 2;
    }
}
