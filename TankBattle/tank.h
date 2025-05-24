#ifndef TANK_H
#define TANK_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QPainter>

#include "entity.h"
#include "bullet.h"

enum TankType {
    PlayerTank,
    EnemyTank
};

class Bullet; // 前向声明

class Tank : public Entity
{
    Q_OBJECT

public:
    Tank(int x, int y, TankType tanktype, int level, int speed, int num = -1);
    ~Tank();
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void changeDirection(const Direction &dir);
    Direction getDirection();
    void setBullet(Bullet *bullet);
    int getRemainBulletNum();
    void removeBullet(Bullet *bullet);
    int getBulletSpeed();
    int getHealth();
    void healthDecrease();
    int getLevel();
    void upgrade();
    void setDistance(int distance);
    int getDistance();
    void setNextPos(QPoint nextPos);
    QPoint getNextPos();
    int getnum();
    void setUnbeatable(bool unbeatable);
    void setTargetPos(QPoint targetPos);
    QPoint getTargetPos();
    void setTempTargetPos(QPoint temptargetPos);
    QPoint getTempTargetPos();
    void setShellInf(bool flag);

private slots:
    void appear();

private:
    QStringList tankAppearPaths;
    Direction direction = Up;
    int bulletSpeed = 1; // 速度
    int health = 1; // 生命值
    int level = 1; // 等级
    QString picpath;
    int speed;
    int shell = 1; // 子弹数量
    QList<Bullet *> bullets; // 子弹列表
    int distance = 0;
    QPoint nextPos;
    int num;
    QPoint targetPos;
    QPoint temptargetPos;
};

#endif // TANK_H
