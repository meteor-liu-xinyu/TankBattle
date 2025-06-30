#include "tank.h"

Tank::Tank(int x, int y, TankType tanktype, int level, int unitSize, int num, QObject *parent)
    : QObject{parent}, Entity(unitSize)
{
    this->level = level;
    this->num = num;
    rect = QRect(x, y, unitSize * 5 / 3, unitSize * 5 / 3);
    nextPos = QPoint(x / unitSize, y / unitSize);
    temptargetPos = nextPos;
    picRootPath = ":/tank/images/";
    if (tanktype == PlayerTank)
    {
        picRootPath += "tank_L" + QString::number(level) + "_";
        tag = "-1";
        health = level + 2;
        shell = 2;
    }
    else
    {
        picRootPath += "enemy_" + QString::number(level) + "_";
        tag = QString::number(num);
        if (level == 3)
        {
            health = 2;
        }
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
    setPixmap(picRootPath + "1.png");
}

void Tank::moveDown()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x(), rect.y() + speed);
    direction = Down;
    setPixmap(picRootPath + "2.png");
}

void Tank::moveLeft()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x() - speed, rect.y());
    direction = Left;
    setPixmap(picRootPath + "3.png");
}

void Tank::moveRight()
{
    if (tick < 3)
    {
        return;
    }
    rect.moveTo(rect.x() + speed, rect.y());
    direction = Right;
    setPixmap(picRootPath + "4.png");
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
        setPixmap(picRootPath + "1.png");
    }
    else
    {
        setPixmap(":/tank/images/appear_" + QString::number(tick + 1) + ".png");
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
            setPixmap(picRootPath + "1.png");
            break;
        case Down:
            setPixmap(picRootPath + "2.png");
            break;
        case Left:
            setPixmap(picRootPath + "3.png");
            break;
        case Right:
            setPixmap(picRootPath + "4.png");
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
    if (level < 3)
    {
        bulletSpeed = speed * 4 * level;
    }
    else
    {
        bulletSpeed = speed * 12;
    }
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

void Tank::setTankSpeed(int speed) // 设置坦克速度
{
    this->speed = speed;
    bulletSpeed = speed * 4;
    if (tag == "-1")
    {
        if (level < 3)
        {
            bulletSpeed *= level;
        }
        else
        {
            bulletSpeed *= 3;
        }
    }
}
