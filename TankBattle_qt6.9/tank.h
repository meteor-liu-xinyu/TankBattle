#ifndef TANK_H
#define TANK_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QPainter>
#include <QTimer>

class Bullet; // 前向声明

#include "entity.h"
#include "bullet.h"

enum TankType {
    PlayerTank,
    EnemyTank
};

class Tank : public QObject, public Entity
{
    Q_OBJECT
public:
    explicit Tank(int x, int y, TankType tanktype, int level, int unitSize,int num = -1, QObject *parent = nullptr);

    // 移动
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void changeDirection(const Direction &dir); // 改变方向
    Direction getDirection(); // 获取方向
    void setBullet(Bullet *bullet); // 设置子弹
    int getRemainBulletNum(); // 获取剩余子弹数量
    void removeBullet(Bullet *bullet); // 移除子弹
    int getBulletSpeed(); // 获取子弹速度
    int getHealth(); // 获取生命值
    void healthDecrease(); // 生命值减少
    int getLevel(); // 获取等级
    void upgrade(); // 升级
    void setDistance(int distance); // 设置距离
    int getDistance(); // 获取距离
    void setNextPos(QPoint nextPos); // 设置下一个位置
    QPoint getNextPos(); // 获取下一个位置
    int getnum(); // 获取编号
    void setUnbeatable(bool unbeatable); // 设置无敌状态
    void setTargetPos(QPoint targetPos); // 设置目标位置
    QPoint getTargetPos(); // 获取目标位置
    void setTempTargetPos(QPoint temptargetPos); // 设置临时目标(随机巡逻)位置
    QPoint getTempTargetPos(); // 获取临时目标位置
    void setShellInf(bool flag); // 设置无限子弹
    void setTankSpeed(int speed); // 设置坦克速度

private slots:
    void appear(); // 出现动画

private:
    QTimer *timer;
    int tick;

    Direction direction = Up; // 方向
    int bulletSpeed = 1; // 子弹速度
    int health = 1; // 生命值
    int level = 1; // 等级
    int speed; // 移动速度
    int shell = 1; // 子弹数量
    QList<Bullet *> bullets; // 子弹列表
    int distance = 0; // 距离
    QPoint nextPos; // 下一个位置
    int num; // 编号
    QPoint targetPos; // 目标位置
    QPoint temptargetPos; // 临时目标(随机巡逻)位置
};

#endif // TANK_H
