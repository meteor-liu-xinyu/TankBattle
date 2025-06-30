#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QHash>
#include <QMessageBox>
#include <QDebug>
#include <QKeyEvent>
#include <QPixmap>
#include <QTimer>
#include <QTime>
#include <QRect>
#include <QPainter>
#include <QPoint>
#include <QList>
#include <QQueue>

#include "entity.h"
#include "bullet.h"
#include "tank.h"
#include "obstacle.h"
#include "boom.h"
#include "stopwidget.h"
#include "gameoverui.h"
#include "food.h"

// QPoint hash 函数
inline uint qHash(const QPoint &key, uint seed = 0) noexcept {
    return qHash(static_cast<quint64>(key.x()) ^ (static_cast<quint64>(key.y()) << 32), seed);
}

namespace Ui {
class MainWidget;
}

struct Information
{
    int enemyleft = 10; // 剩余敌人数量
    int playerlife = 2; // 玩家剩余坦克数
    int win = 0; // 胜利次数
    int total = 0; // 总游戏次数
};

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = 0);
    ~MainWidget();
    void paintEvent(QPaintEvent *event) override; // 用于绘制游戏界面
    void keyPressEvent(QKeyEvent *k) override; // 处理键盘输入
    void keyReleaseEvent(QKeyEvent *k) override; // 处理键盘释放事件

    void generateGameMap(); // 生成游戏地图
    void generatePlayerTank(); // 生成坦克
    void generateEnemyTank(); // 生成敌人坦克
    void generateFood(int x = -1, int y = -1); // 生成道具
    void initGame(); // 初始化游戏
    void gameEnded(); // 游戏结束
    void gameStop(); // 游戏暂停

signals:
    void playerTankHit(); // 坦克被击中
    void enemyTankHit(int i); // 敌人坦克被击中
    void pickFood(FoodType foodtype); // 拾取道具

private slots:
    void on_startButton_clicked(); // 开始游戏

    void updateGame(); // 更新游戏状态
    void dealplayerTankHit(); // 处理玩家坦克被击中
    void dealenemyTankHit(int i); // 处理敌人坦克被击中
    void dealpickFood(FoodType foodtype); // 处理拾取道具

private:
    Ui::MainWidget *ui;

    int tickrate = 60; // 游戏的帧率
    QTimer *timer = NULL; // 定时器
    QTimer *foodTimer = NULL; // 道具定时器
    QTimer *unbeatableTimer = NULL; // 无敌定时器
    QTimer* fixTimer = NULL; // 修复定时器
    QTimer *gameTimer = NULL; // 游戏计时器
    QTimer *shellInfTimer = NULL; // 无限子弹定时器

    int min = 0; // 分钟
    int second = 0; // 秒
    int millisecond = 0; // 毫秒

    Obstacle* home = NULL; // 基地
    QList<Obstacle*> brickGroup; // 砖块组
    QList<Obstacle*> ironGroup; // 铁组
    QList<Obstacle*> riverGroup; // 河流组
    QList<Obstacle*> treeGroup; // 树组
    QList<Tank*> enemyTank; // 坦克列表
    int countSimpleEnemy = 0; // 简单敌人数量
    Tank* playerTank = NULL; // 玩家坦克
    QList<Bullet*> bullets; // 子弹列表
    QList<Boom*> booms; // 爆炸列表
    QList<Food*> foods; // 道具列表

    QHash<QPoint,Obstacle*> unitOccupied; // 占用单位

    Information information; // 游戏信息

    bool gameend = true; // 游戏结束
    bool win = false; // 游戏胜利
    bool homeDestroyed; // 基地被摧毁

    QList<Direction> movingDirection; // 移动方向

    StopWidget *stopwidget = NULL; // 暂停界面
    GameoverUI *gameoverui = NULL; // 游戏结束界面

    bool checkTankCollision(const Direction &dir); // 检测玩家坦克碰撞,返回true表示碰撞
    bool checkBulletCollision(Bullet *bullet, const Direction &dir); // 检测子弹碰撞,返回true表示碰撞

    void generateBrickGroup(int x, int y); // 生成砖块组
    void generateIronGroup(int x, int y); // 生成铁块组
    void generateRiverGroup(int x, int y); // 生成河流组
    void generateTreeGroup(int x, int y); // 生成树组
    bool nearHome(int x, int y); // 检测是否靠近基地

    QString getWhiteString(QString str); // 获取白色html字符串

    void shoot(Tank *tank); // 发射子弹

    QPoint findNextDirection(Tank *tank, QPoint start, QPoint target); // 人机寻路, 使用A*算法
    void enemyMove(Tank *enemy); // 人机移动
    bool unitReachable(QPoint pos); // 检测单位是否可达
    Direction toDirection(int dir); // int 转 Direction
    Direction moveToUnit(Tank *tank); // 移动到nextPos
    int manhattanDistance(QPoint start, QPoint target); // 曼哈顿距离
    void sortInsert(QQueue<QPoint> &shortPath, const QHash<QPoint,int> &priority, const QPoint &point); // 插入优先队列
    void ifShootPlayer(Tank* enemy); // 检测人机是否射击玩家
    void ifShootHome(Tank* enemy); // 检测人机是否射击基地
    void avoidBullet(Bullet *bullet, QPoint tankPos, Tank *enemy); // 避免子弹
    bool ifInShootRect(Tank *tank, Bullet *bullet); // 检测是否在射击范围内
};

#endif // MAINWIDGET_H
