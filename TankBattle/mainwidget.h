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
    int win = 0;
    int total = 0;
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
    void initGame();
    void gameEnded();

signals:
    void playerTankDestroyed(); // 坦克被摧毁
    void enemyTankDestroyed(int i);
    void pickFood(FoodType foodtype);

private slots:
    void on_startButton_clicked();

    void updateGame(); // 更新游戏状态
    void dealplayerTankDestroyed(); // 玩家坦克被摧毁
    void dealenemyTankDestroyed(int i); // 敌人坦克被摧毁
    void dealpickFood(FoodType foodtype);

private:
    Ui::MainWidget *ui;

    int tickrate = 60; // 用于控制游戏的帧率
    QTimer *timer = NULL; // 定时器
    QTimer *foodTimer = NULL; // 道具定时器
    QTimer *unbeatableTimer = NULL;
    QTimer* fixTimer = NULL;
    QTimer *gameTimer = NULL;
    QTimer *shellInfTimer = NULL;

    int min = 0;
    int second = 0;
    int msecond = 0;

    Obstacle* home = NULL; // 家
    QList<Obstacle*> brickGroup; // 磁砖组
    QList<Obstacle*> ironGroup; // 铁组
    QList<Obstacle*> riverGroup; // 河流组
    QList<Obstacle*> treeGroup; // 树组
    QList<Tank*> enemyTank; // 坦克列表
    int countSimpleEnemy = 0; // 简单敌人数量
    Tank* playerTank = NULL; // 玩家坦克
    QList<Bullet*> bullets; // 子弹列表
    QList<Boom*> booms;
    QList<Food*> foods;

    QHash<QPoint,Obstacle*> unitOccupied;

    Information information; // 游戏信息

    bool gameend = true;
    bool win = false;
    bool homeDestroyed;

    QList<Direction> movingDirection;

    StopWidget *stopwidget = NULL;
    GameoverUI *gameoverui = NULL;

    bool checkTankCollision(const Direction &dir);
    bool checkBulletCollision(Bullet *bullet, const Direction &dir);

    void generateBrickGroup(int x, int y);
    void generateIronGroup(int x, int y);
    void generateRiverGroup(int x, int y);
    void generateTreeGroup(int x, int y);
    bool nearHome(int x, int y);

    QString getWhiteString(QString str);

    void shoot(Tank *tank); // 发射子弹

    QPoint findNextDirection(Tank *tank, QPoint start, QPoint target);
    void enemyMove(Tank *enemy);
    bool unitReachable(QPoint pos);
    Direction toDirection(int dir);
    Direction moveToUnit(Tank *tank);
    int manhattanDistance(QPoint start, QPoint target);
    void sortInsert(QQueue<QPoint> &shortPath, const QHash<QPoint,int> &priority, const QPoint &point);
    void ifShootPlayer(Tank* enemy);
    void ifShootHome(Tank* enemy);
    void avoidBullet(Bullet *bullet, QPoint tankPos, Tank *enemy);
    bool ifInShootRect(Tank *tank, Bullet *bullet);
};

#endif // MAINWIDGET_H
