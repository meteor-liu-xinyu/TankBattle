#include "mainwidget.h"
#include "ui_mainwidget.h"

#define UNIT 24 // 单位大小
#define TANKSPEED 120 / tickrate // 坦克速度
#define BULLETSPEED 480 / tickrate // 子弹速度
#define INITPLAYERLIFE 2 // 玩家生命数初始值
#define INITENEMYNUM 10 // 敌人数量初始值

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    this->setWindowTitle("坦克大战");
    ui->startButton->clearFocus(); // 清除按钮焦点

    QPixmap cover(":/background/images/cover.png"); // 设置封面图片
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QBrush(cover));
    // 应用 QPalette 到窗口
    this->setPalette(palette);

    information.playerlife = INITPLAYERLIFE;
    information.enemyleft = INITENEMYNUM;

    stopwidget = new StopWidget(this); // 创建暂停窗口
    stopwidget->hide();
}

void MainWidget::on_startButton_clicked()
{
    disconnect(stopwidget, &StopWidget::restart, nullptr, nullptr); // 断开信号槽连接
    if (!gameend || timer || gameTimer || foodTimer) // 如果游戏未结束或计时器未清除
    {
        gameend = true;
        gameEnded();
    }

    ui->startButton->clearFocus(); // 清除按钮焦点

    information.total++; // 游戏次数加1

    initGame(); // 初始化游戏
}

void MainWidget::initGame()
{
    // 清除之前的游戏数据
    gameend = false;
    win = false;
    unitOccupied.clear();
    enemyTank.clear();
    playerTank = NULL;
    home = NULL;
    brickGroup.clear();
    ironGroup.clear();
    riverGroup.clear();
    treeGroup.clear();
    homeDestroyed = false;
    foods.clear();
    countSimpleEnemy = 0;
    movingDirection.clear();
    bullets.clear();
    booms.clear();

    qsrand(QTime::currentTime().msec()); // 初始化随机种子

    timer = new QTimer(this);
    timer->start(1000 / tickrate); // 设置每帧时间
    connect(timer, &QTimer::timeout, this, &MainWidget::updateGame);


    foodTimer = new QTimer(this);
    foodTimer->start(1000 * 30);
    connect(foodTimer, &QTimer::timeout, this, [=](){
        int possibility = qrand() % 10;
        if (possibility < 8)
        {
            generateFood();
        }
    });

    // 计时器
    gameTimer = new QTimer(this);
    gameTimer->start(10);
    min = 0;
    second = 0;
    millisecond = 0;
    connect(gameTimer, &QTimer::timeout, this, [=](){
        millisecond++;
        if (millisecond == 100)
        {
            millisecond = 0;
            second++;
        }
        if (second == 60)
        {
            second = 0;
            min++;
        }
        QString millisecondstr = QString::number(millisecond);
        if (millisecondstr.size() == 1)
        {
            millisecondstr.push_front('0');
        }
        QString secondstr = QString::number(second);
        if (secondstr.size() == 1)
        {
            secondstr.push_front('0');
        }
        ui->timeLabbel->setText(getWhiteString(QString::number(min) + ":" + secondstr + ":" + millisecondstr));
    });

    generateGameMap(); // 生成游戏地图
    generatePlayerTank(); // 生成玩家坦克
    for (int i = 0; i < 1; i++) // 生成敌人坦克
    {
        generateEnemyTank();
    }
    generateFood(); // 生成道具

    if (stopwidget)
    {
        stopwidget->hide();
        stopwidget = NULL;
    }
    if (gameoverui)
    {
        gameoverui->close();
        gameoverui = NULL;
    }

    information.enemyleft = INITENEMYNUM;
    information.playerlife = INITPLAYERLIFE;

    // 设置标签
    ui->levelLabel->setText(getWhiteString(QString::number(playerTank->getLevel())));
    ui->healthLabel->setText(getWhiteString("inf"));
    ui->enemyLabel->setText(getWhiteString(QString::number(information.enemyleft)));
    ui->playerLifeLabel->setText(getWhiteString(QString::number(information.playerlife)));
    ui->shellLabel->setText(getWhiteString(QString::number(playerTank->getRemainBulletNum())));
    ui->winLabel->setText(getWhiteString(QString::number(information.win) + "/" + QString::number(information.total)));
}

void MainWidget::updateGame() // 更新游戏
{
    // 移动
    if (movingDirection.size() != 0 && !gameend && playerTank)
    {
        playerTank->changeDirection(movingDirection.last());
        if (!checkTankCollision(movingDirection.last())) // 如果移动方向没有碰撞
        {
            switch (movingDirection.last())
            {
            case Up:
                playerTank->moveUp();
                break;
            case Down:
                playerTank->moveDown();
                break;
            case Left:
                playerTank->moveLeft();
                break;
            case Right:
                playerTank->moveRight();
                break;
            default:
                break;
            }
        }
    }

    connect(this, &MainWidget::playerTankHit, this, &MainWidget::dealplayerTankHit);
    connect(this, &MainWidget::enemyTankHit, this, &MainWidget::dealenemyTankHit);
    connect(this, &MainWidget::pickFood, this, &MainWidget::dealpickFood);

    // 移除完成的爆炸
    for (auto boom : booms)
    {
        connect(boom, &Boom::boomFinished, this, [=](){
            booms.removeOne(boom);
        });
    }

    // 如果玩家坦克再水中，则坦克死亡
    if (playerTank)
    {
        for (auto river : riverGroup)
        {
            if (river->getRect().intersects(playerTank->getRect()))
            {
                emit dealplayerTankHit();
                break;
            }
        }
    }

    if (bullets.size() != 0 && !gameend)
    {
        for (int i = 0; i < bullets.size(); i++)
        {
            if (checkBulletCollision(bullets[i],bullets[i]->getDirection())) // 如果子弹碰撞
            {
                if (bullets[i]->getTag() == "-1") // 如果是玩家子弹
                {
                    if (playerTank && playerTank->getRemainBulletNum() < 2)
                    {
                        playerTank->removeBullet(bullets[i]); //玩家移除该子弹
                        // 设置标签
                        if (playerTank->getRemainBulletNum() == -1) // 如果是无限子弹
                        {
                            ui->shellLabel->setText(getWhiteString("inf"));
                        }
                        else
                        {
                            ui->shellLabel->setText(getWhiteString(QString::number(playerTank->getRemainBulletNum())));
                        }
                    }
                }
                else
                {
                    if (bullets[i]->getOwner() && bullets[i]->getOwner()->getRemainBulletNum() < 1)
                    {
                        bullets[i]->getOwner()->removeBullet(bullets[i]); //敌人移除该子弹
                    }
                }
                bullets.removeAt(i);
                i--;
                continue;
            }
            bullets[i]->move(); // 移动子弹
        }
    }

    if (enemyTank.size() != 0 && !gameend)
    {
        for (auto enemy : enemyTank)
        {
            if (enemy->getLevel() != 2)
            {
                // 更新玩家位置作为目标
                enemy->setTargetPos(QPoint(playerTank->getPos().x()/UNIT, playerTank->getPos().y()/UNIT));
            }

            enemyMove(enemy); // 敌人移动

            // 检测是否要躲子弹
            for (auto bullet : bullets)
            {
                if (ifInShootRect(enemy, bullet) && bullet->getOwner() != enemy)
                {
                    avoidBullet(bullet, enemy->getNextPos(), enemy);
                }
            }
            Direction nextdir = moveToUnit(enemy); // 获取下一刻运动方向
            ifShootPlayer(enemy); // 判断是否射击玩家
            ifShootHome(enemy); // 判断是否射击基地
            // 移动
            switch (nextdir)
            {
            case Up:
                enemy->moveUp();
                break;
            case Down:
                enemy->moveDown();
                break;
            case Left:
                enemy->moveLeft();
                break;
            case Right:
                enemy->moveRight();
                break;
            default:
                break;
            }
        }
    }



    if (homeDestroyed && home) // 如果基地被摧毁
    {
        home->homeDestroyed(); // 基地被摧毁
    }

    if (gameend) // 如果游戏结束
    {
        movingDirection.clear(); // 清空移动方向

        if (!gameoverui) // 如果游戏结束界面没有显示
        {
            // 显示游戏结束界面
            gameoverui = new GameoverUI(this, win);
            gameoverui->show();
            connect(gameoverui, &GameoverUI::restart, this, &MainWidget::on_startButton_clicked);
        }

        if (booms.empty()) // 如果没有爆炸了
        {
            gameEnded(); // 游戏结束
        }
    }

    this->update(); // 更新画面
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::gameEnded()
{
    // 清除计时器
    timer->stop();
    disconnect(timer, &QTimer::timeout, nullptr, nullptr);
    delete timer;
    timer = NULL;
    qDebug() << "timer deleted";

    gameTimer->stop();
    disconnect(gameTimer, &QTimer::timeout, nullptr, nullptr);
    delete gameTimer;
    gameTimer = NULL;
    qDebug() << "gameTimer deleted";

    foodTimer->stop();
    disconnect(foodTimer, &QTimer::timeout, nullptr, nullptr);
    delete foodTimer;
    foodTimer = NULL;
    qDebug() << "foodTimer deleted";

    if (unbeatableTimer)
    {
        unbeatableTimer->stop();
        disconnect(unbeatableTimer, &QTimer::timeout, nullptr, nullptr);
        delete unbeatableTimer;
    }
    unbeatableTimer = NULL;
    qDebug() << "unbeatableTimer deleted";

    if (fixTimer)
    {
        fixTimer->stop();
        disconnect(fixTimer, &QTimer::timeout, nullptr, nullptr);
        delete fixTimer;
    }
    fixTimer = NULL;
    qDebug() << "fixTimer deleted";

    if (shellInfTimer)
    {
        shellInfTimer->stop();
        disconnect(shellInfTimer, &QTimer::timeout, nullptr, nullptr);
        delete shellInfTimer;
    }
    shellInfTimer = NULL;
    qDebug() << "shellInfTimer deleted";

    if (win)
    {
        information.win++; // 胜利次数加一
    }
}

bool MainWidget::checkTankCollision(const Direction &dir) // 检测碰撞,返回true表示碰撞
{
    if (!playerTank)
    {
        return false;
    }
    QRect nextrect = playerTank->getRect(); // 获取下一刻位置
    switch (dir)
    {
    case Up:
        nextrect.moveTo(nextrect.x(), nextrect.y() - TANKSPEED);
        break;
    case Down:
        nextrect.moveTo(nextrect.x(), nextrect.y() + TANKSPEED);
        break;
    case Left:
        nextrect.moveTo(nextrect.x() - TANKSPEED, nextrect.y());
        break;
    case Right:
        nextrect.moveTo(nextrect.x() + TANKSPEED, nextrect.y());
        break;
    default:
        break;
    }

    if (nextrect.x() < 0 || nextrect.x() > 58 * UNIT + 12 || nextrect.y() < 0 || nextrect.y() > 58 * UNIT + 12)
    {
        return true; // 超出边界
    }

    // 检测附近的障碍物是否有碰撞
    for (int i = nextrect.x() / UNIT - 2; i <= nextrect.x() / UNIT + 2; i++)
    {
        for (int j = nextrect.y() / UNIT - 2; j <= nextrect.y() / UNIT + 2; j++)
        {
            if (unitOccupied.contains(QPoint(i,j)))
            {
                if (nextrect.intersects(unitOccupied[QPoint(i,j)]->getRect()) && unitOccupied[QPoint(i,j)]->getTag() != "river")
                {
                    return true; // 碰撞
                }
            }
        }
    }

    nextrect = QRect(nextrect.x(), nextrect.y(), nextrect.width() - 6, nextrect.height() - 6);

    // 检测敌人是否有碰撞
    for (auto enemy : enemyTank)
    {
        if (nextrect.intersects(enemy->getRect()))
        {
            return true; // 碰撞
        }
    }

    // 检测是否捡到道具
    for (auto food : foods)
    {
        if (nextrect.intersects(food->getRect()))
        {
            emit pickFood(food->getFoodType()); // 发射信号
            qDebug() << "food picked";
            foods.removeOne(food); // 移除道具
        }
    }

    // 检测是否碰到基地
    if (nextrect.intersects(home->getRect()))
    {
        return true;
    }

    return false;
}

bool MainWidget::checkBulletCollision(Bullet *bullet, const Direction &dir) // 检测子弹碰撞,返回true表示碰撞
{
    QRect nextrect = bullet->getRect(); // 获取下一刻位置
    switch (dir)
    {
    case Up:
        nextrect.moveTo(nextrect.x(), nextrect.y() - bullet->getSpeed());
        break;
    case Down:
        nextrect.moveTo(nextrect.x(), nextrect.y() + bullet->getSpeed());
        break;
    case Left:
        nextrect.moveTo(nextrect.x() - bullet->getSpeed(), nextrect.y());
        break;
    case Right:
        nextrect.moveTo(nextrect.x() + bullet->getSpeed(), nextrect.y());
        break;
    default:
        break;
    }

    if (nextrect.x() < 0 || nextrect.x() > 59 * UNIT + 12 || nextrect.y() < 0 || nextrect.y() > 59 * UNIT + 12)
    {
        return true; // 超出边界
    }

    bool flag = false;
    // 检测附近的障碍物是否有碰撞
    for (int i = nextrect.x() / UNIT - 2; i <= nextrect.x() / UNIT + 2; i++)
    {
        for (int j = nextrect.y() / UNIT - 2; j <= nextrect.y() / UNIT + 2; j++)
        {
            if (unitOccupied.contains(QPoint(i, j)))
            {
                if (nextrect.intersects(unitOccupied[QPoint(i,j)]->getRect()))
                {
                    if (unitOccupied[QPoint(i,j)]->getTag() == "brick") // 砖块
                    {
                        booms.append(new Boom(unitOccupied[QPoint(i,j)]->getRect())); // 爆炸
                        brickGroup.removeOne(unitOccupied[QPoint(i,j)]); // 移除砖块
                        unitOccupied.remove(QPoint(i, j));
                        flag = true;
                    }
                    else if (unitOccupied[QPoint(i,j)]->getTag() == "iron") // 铁
                    {
                        flag = true;
                    }
                }
            }
        }
    }
    if (flag)
    {
        return true;
    }

    // 检测是否命中基地
    if (nextrect.intersects(home->getRect()))
    {
        booms.append(new Boom(home->getRect()));
        gameend = true; // 游戏结束
        homeDestroyed = true;
        win = false;
        return true;
    }

    // 检测是否命中敌人或玩家
    if (bullet->getTag() == "-1")
    {
        for (int i = 0; i < enemyTank.size(); i++)
        {
            if (nextrect.intersects(enemyTank[i]->getRect()))
            {
                qDebug() << "hit enemy tank";
                emit enemyTankHit(i);
                booms.append(new Boom(bullet->getRect()));
                return true;
            }
        }
    }
    else
    {
        if (nextrect.intersects(playerTank->getRect()))
        {
            qDebug() << "hit player tank";
            emit playerTankHit();
            booms.append(new Boom(bullet->getRect()));
            return true;
        }
    }

    return false;
}


void MainWidget::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_R) // 按下R键重新开始
    {
        on_startButton_clicked();
    }
    else if (k->key() == Qt::Key_Space && !gameend && playerTank) // 按下空格键射击
    {
        shoot(playerTank);
    }
    else if (k->key() == Qt::Key_Escape) // 按下Esc键暂停
    {
        gameStop();
    }
    if ((k->key() == Qt::Key_Up || k->key() == Qt::Key_W) && !gameend)
    {
        movingDirection.append(Up);
    }
    if ((k->key() == Qt::Key_Down || k->key() == Qt::Key_S) && !gameend)
    {
        movingDirection.append(Down);
    }
    if ((k->key() == Qt::Key_Left || k->key() == Qt::Key_A) && !gameend)
    {
        movingDirection.append(Left);
    }
    if ((k->key() == Qt::Key_Right ||k->key() == Qt::Key_D) && !gameend)
    {
        movingDirection.append(Right);
    }
}

void MainWidget::gameStop()
{
    if (gameend)
    {
        return;
    }
    if (timer != NULL)
    {
        if (timer->isActive())
        {
            timer->stop();
            gameTimer->stop();
            if (!stopwidget) // 没有暂停窗口
            {
                stopwidget = new StopWidget(this);
            }
            stopwidget->show();
            connect(stopwidget, &StopWidget::restart, this, &MainWidget::on_startButton_clicked);
            connect(stopwidget, &StopWidget::continuesgn, this, [=](){
                disconnect(stopwidget, &StopWidget::continuesgn, nullptr, nullptr);
                timer->start();
                gameTimer->start();
                if (stopwidget)
                {
                    stopwidget->hide();
                }
            });
        }
        else
        {
            timer->start();
            gameTimer->start();
            if (stopwidget)
            {
                stopwidget->hide();
            }
        }
    }
}

void MainWidget::keyReleaseEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_Up || k->key() == Qt::Key_W)
    {
        movingDirection.removeOne(Up);
    }
    if (k->key() == Qt::Key_Down || k->key() == Qt::Key_S)
    {
        movingDirection.removeOne(Down);
    }
    if (k->key() == Qt::Key_Left || k->key() == Qt::Key_A)
    {
        movingDirection.removeOne(Left);
    }
    if (k->key() == Qt::Key_Right || k->key() == Qt::Key_D)
    {
        movingDirection.removeOne(Right);
    }
}

void MainWidget::paintEvent(QPaintEvent *event) // 绘制
{
    QPainter painter(this);
    // lambda表达式
    auto paintObstacle = [&](QList<Obstacle*> entityGroup){
        for (auto entity : entityGroup)
        {
            entity->draw(painter);
        }
    };

    paintObstacle(brickGroup);
    paintObstacle(ironGroup);
    paintObstacle(riverGroup);
    paintObstacle(treeGroup);

    if (home)
    {
        home->draw(painter);
    }
    if (playerTank)
    {
        playerTank->draw(painter);
    }
    for (auto enemy : enemyTank)
    {
        enemy->draw(painter);
    }
    for (auto boom : booms)
    {
        boom->draw(painter);
    }
    for (auto food : foods)
    {
        food->draw(painter);
    }
    for (auto bullet : bullets)
    {
        bullet->draw(painter);
    }
}

void MainWidget::generatePlayerTank()
{
    // 生成玩家坦克
    playerTank = new Tank(29 * UNIT, 54 * UNIT, PlayerTank, 1, TANKSPEED);
    playerTank->setUnbeatable(true);

    ui->healthLabel->setText(getWhiteString("inf"));

    // 设置无敌状态
    unbeatableTimer = new QTimer(this);
    unbeatableTimer->start(3000);
    connect(unbeatableTimer, &QTimer::timeout, this, [=](){
        playerTank->setUnbeatable(false);
        ui->healthLabel->setText(getWhiteString(QString::number(playerTank->getHealth())));
        unbeatableTimer->stop();
        disconnect(unbeatableTimer);
        delete unbeatableTimer;
        unbeatableTimer = NULL;
    });
}

void MainWidget::generateEnemyTank()
{
    // 随机生成敌人坦克位置
    int x = 0, y = 0;
    while (true)
    {
        x = (qrand() % 28 + 1) * 2;
        y = (qrand() % 10 + 1) * 2;
        if (!unitOccupied.contains(QPoint(x, y)))
        {
            // 不生成在玩家附近
            if (playerTank && (abs(playerTank->getPos().x()/UNIT - x) > 6 && abs(playerTank->getPos().y()/UNIT - y) > 3))
            {
                break;
            }
        }
    }
    int enemyLevel = qrand() % 5 + 1; // 1-4
    if (enemyLevel != 4)
    {
        if (enemyLevel == 5)
        {
            enemyLevel = 2;
        }
        countSimpleEnemy++;
        if (countSimpleEnemy == 4)
        {
            countSimpleEnemy = 0;
        }
    }
    else
    {
        countSimpleEnemy = 0;
    }

    enemyTank.append(new Tank(x * UNIT, y * UNIT, EnemyTank, enemyLevel, TANKSPEED, enemyTank.size()));

    // 设置目标
    QPoint targetPos;
    if (enemyTank.last()->getLevel() == 2)
    {
        int possibility = qrand() % 5;
        if (possibility < 2)
        {
            targetPos = QPoint(26, 58);
        }
        else if (possibility == 2)
        {
            targetPos = QPoint(32, 58);
        }
        else
        {
            targetPos = QPoint(29, 54);
        }
    }
    else
    {
        targetPos = QPoint(playerTank->getPos().x()/UNIT, playerTank->getPos().y()/UNIT);
    }
}

void MainWidget::generateFood(int x, int y)
{
    if (x == -1 &&y == -1) // 如果没有指定位置
    {
        // 随机生成道具位置
        while (true)
        {
            x = (qrand() % 28 + 1) * 2; // 1-28
            y = (qrand() % 20 + 1) * 2; // 1-20
            if (!unitOccupied.contains(QPoint(x, y)))
            {
                break;
            }
        }
    }

    int foodType = qrand() % 4; // 0-3
    Food *food = new Food(x * UNIT, y * UNIT, foodType);
    foods.append(food);
    QTimer *disappear = new QTimer(this);
    disappear->start(15000);
    connect(disappear, &QTimer::timeout, this, [=](){
        disappear->stop();
        if (food)
        {
            foods.removeOne(food);
        }
    });
}

void MainWidget::generateGameMap()
{
    // 设置背景
    QPixmap background(":/background/images/background.png");
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QBrush(background));
    // 应用 QPalette 到窗口
    this->setPalette(palette);

    // 生成基地（固定在底部中央位置：第14 15列中间，第29行）
    home = new Obstacle(29 * UNIT, 58 * UNIT, Home);

    // 生成基地保护墙（左/右/上三个方向，每个方向生成24x24单块砖）
    for (int j = 57; j < 60; j++)
    {
        brickGroup.append(new Obstacle(28 * UNIT, j * UNIT, Brick));
        unitOccupied[QPoint(28,j)] = brickGroup.last();
        brickGroup.append(new Obstacle(31 * UNIT, j * UNIT, Brick));
        unitOccupied[QPoint(31,j)] = brickGroup.last();
    }
    brickGroup.append(new Obstacle(29 * UNIT, 57 * UNIT, Brick));
    unitOccupied[QPoint(29,57)] = brickGroup.last();
    brickGroup.append(new Obstacle(30 * UNIT, 57 * UNIT, Brick));
    unitOccupied[QPoint(30,57)] = brickGroup.last();

    // 生成河水
    #define riverWidth 10
    #define riverHigh 12
    for (int x = 15 - riverWidth / 2; x < 15 + riverWidth / 2; x++)
    {
        generateRiverGroup(x * 2, riverHigh * 2);
    }

    generateIronGroup(26, 46);
    generateIronGroup(28, 46);
    generateIronGroup(30, 46);
    generateIronGroup(32, 46);

    // 固定生成砖墙
    #define maxcolumnBricknum 10
    for (int x = 28; x > 0; x -= 4)
    {
        for (int j = 0; j < maxcolumnBricknum;)
        {
            int y = (qrand() % 28 + 1) * 2;      // 1-28
            // 检查未被占用
            if (!unitOccupied.contains(QPoint(x, y)) && !nearHome(x,y))
            {
                generateBrickGroup(x , y); // 生成2x2砖块组
                j++;
            }
        }
        for (int j = 0; j < maxcolumnBricknum;)
        {
            int y = (qrand() % 28 + 1) * 2;      // 1-28
            // 检查未被占用
            if (!unitOccupied.contains(QPoint(58 - x, y)) && !nearHome(58 - x,y))
            {
                generateBrickGroup(58 - x , y); // 生成2x2砖块组
                j++;
            }
        }
    }

    // 生成铁墙
    #define ironGroups 20
    for (int i = 0; i < ironGroups; )
    {
        int x = (qrand() % 28 + 1) * 2;
        int y = (qrand() % 16 + 8) * 2; // 8-25

        if (unitOccupied.contains(QPoint(x, y)) || unitOccupied.contains(QPoint(x + 1, y)) || unitOccupied.contains(QPoint(x, y + 1)) || unitOccupied.contains(QPoint(x + 1, y + 1)))
        {
            continue;
        }
        else
        {
            int type = qrand() % 3;
            if (type == 0)
            {
                generateIronGroup(x, y);
            }
            else if (type == 2)
            {
                generateIronGroup(x, y + 1);
            }
            else
            {
                generateIronGroup(x, y);
                generateIronGroup(x, y + 1);
            }
            i++;
        }
    }

    // 随机生成砖墙
    #define minRandomBricks 20
    #define maxRandomBricks 50
    int randomBrickGroups = minRandomBricks + (qrand() % (maxRandomBricks - minRandomBricks + 1));

    for (int i = 0; i < randomBrickGroups; )
    {
        int x = (qrand() % 28 + 1) * 2;      // 1-28
        int y = (qrand() % 27 + 1) * 2;      // 1-27

        // 检查未被占用
        if (!unitOccupied.contains(QPoint(x, y)) && !nearHome(x,y))
        {
            generateBrickGroup(x, y); // 生成2x2砖块组
            i++;
        }
    }

    // 生成树叶
    #define treeGroups 16
    for (int i = 0; i < treeGroups; )
    {
        int x = (qrand() % 28 + 1) * 2;
        int y = (qrand() % 27 + 1) * 2;  // 避免最底部行

        if (nearHome(x, y) || unitOccupied.contains(QPoint(x, y)) || unitOccupied.contains(QPoint(x + 1, y)) || unitOccupied.contains(QPoint(x, y + 1)) || unitOccupied.contains(QPoint(x + 1, y + 1)))
        {
            continue;
        }

        generateTreeGroup(x, y);
        i++;
    }
}

void MainWidget::generateBrickGroup(int x, int y)
{
    // 生成4个砖块（2x2布局覆盖48x48 UNIT）
    brickGroup.append(new Obstacle(x * UNIT, y * UNIT, Brick));
    unitOccupied[QPoint(x,y)] = brickGroup.last();
    brickGroup.append(new Obstacle(x * UNIT + 24, y * UNIT, Brick));
    unitOccupied[QPoint(x+1,y)] = brickGroup.last();
    brickGroup.append(new Obstacle(x * UNIT, y * UNIT + 24, Brick));
    unitOccupied[QPoint(x,y+1)] = brickGroup.last();
    brickGroup.append(new Obstacle(x * UNIT + 24, y * UNIT + 24, Brick));
    unitOccupied[QPoint(x+1,y+1)] = brickGroup.last();
}

void MainWidget::generateIronGroup(int x, int y)
{
    // 生成2个铁块（水平排列）
    ironGroup.append(new Obstacle(x * UNIT, y * UNIT, Iron));
    unitOccupied[QPoint(x,y)] = ironGroup.last();
    ironGroup.append(new Obstacle(x * UNIT + 24, y * UNIT, Iron));
    unitOccupied[QPoint(x+1,y)] = ironGroup.last();
}

void MainWidget::generateRiverGroup(int x, int y)
{
    // 生成4个河水（4x4布局）
    riverGroup.append(new Obstacle(x * UNIT, y * UNIT, River));
    unitOccupied[QPoint(x,y)] = riverGroup.last();
    riverGroup.append(new Obstacle(x * UNIT + 24, y * UNIT, River));
    unitOccupied[QPoint(x+1,y)] = riverGroup.last();
    riverGroup.append(new Obstacle(x * UNIT, y * UNIT + 24, River));
    unitOccupied[QPoint(x,y+1)] = riverGroup.last();
    riverGroup.append(new Obstacle(x * UNIT + 24, y * UNIT + 24, River));
    unitOccupied[QPoint(x+1,y+1)] = riverGroup.last();
}

void MainWidget::generateTreeGroup(int x, int y)
{
    // 生成16个树叶（4x4布局）
    treeGroup.append(new Obstacle(x * UNIT, y * UNIT, Tree));
    treeGroup.append(new Obstacle(x * UNIT + 24, y * UNIT, Tree));
    treeGroup.append(new Obstacle(x * UNIT, y * UNIT + 24, Tree));
    treeGroup.append(new Obstacle(x * UNIT + 24, y * UNIT + 24, Tree));
}

bool MainWidget::nearHome(int x, int y)
{
    if (x >= 25 && x <= 34 && y >= 54 && y <= 59)
    {
        return true; // 在基地附近
    }
    else
    {
        return false; // 不在基地附近
    }
}

QString MainWidget::getWhiteString(QString str)
{
    return "<span style='color: white;'>" + str + "</span>";
}

Direction MainWidget::toDirection(int dir)
{
    switch (dir)
    {
    case 0:
        return Up;
    case 1:
        return Down;
    case 2:
        return Left;
    case 3:
        return Right;
    default:
        return Stop;
    }
}

void MainWidget::dealplayerTankHit()
{
    disconnect(this, &MainWidget::playerTankHit, this, &MainWidget::dealplayerTankHit);

    // 玩家坦克落水
    bool inRiver = false;
    for (auto river : riverGroup)
    {
        if (river->getRect().intersects(playerTank->getRect()))
        {
            inRiver = true;
            break;
        }
    }

    if (playerTank->getHealth() == 1 || inRiver)
    {
        booms.append(new Boom(playerTank->getRect()));

        if (information.playerlife == 1)
        {
            gameend = true;
            win = false;
            return;
        }
        else
        {
            generatePlayerTank(); // 重新生成玩家坦克
            // 更新标签
            information.playerlife--;
            ui->healthLabel->setText(getWhiteString("inf"));
            ui->playerLifeLabel->setText(getWhiteString(QString::number(information.playerlife)));
            ui->levelLabel->setText(getWhiteString(QString::number(playerTank->getLevel())));
            ui->shellLabel->setText(getWhiteString(QString::number(playerTank->getRemainBulletNum())));
        }
    }
    else if (playerTank->getHealth() != -1)
    {
        playerTank->healthDecrease();
        ui->healthLabel->setText(getWhiteString(QString::number(playerTank->getHealth())));
    }
}

void MainWidget::dealenemyTankHit(int i)
{
    disconnect(this, &MainWidget::enemyTankHit, this, &MainWidget::dealenemyTankHit);

    if (enemyTank[i]->getHealth() == 1)
    {
        information.enemyleft--;
        booms.append(new Boom(enemyTank[i]->getRect()));
        if (enemyTank[i]->getLevel() == 4) // 目标坦克击败后生成道具
        {
            generateFood(enemyTank[i]->getPos().x()/UNIT, enemyTank[i]->getPos().y()/UNIT);
        }
        enemyTank.removeAt(i);
        ui->enemyLabel->setText(getWhiteString(QString::number(information.enemyleft)));
        if (information.enemyleft == 0)
        {
            gameend = true;
            win = true;
            return;
        }
        else
        {
            generateEnemyTank(); // 重新生成敌人坦克
        }
    }
    else
    {
        enemyTank[i]->healthDecrease();
    }
}

void MainWidget::shoot(Tank *tank)
{
    if (tank->getRemainBulletNum() != 0)
    {
        Bullet *bullet = new Bullet(tank, BULLETSPEED);
        bullets.append(bullet);
        tank->setBullet(bullet);
        if (tank == playerTank)
        {
            if (tank->getRemainBulletNum() == -1)
            {
                ui->shellLabel->setText(getWhiteString("inf"));
            }
            else
            {
                ui->shellLabel->setText(getWhiteString(QString::number(tank->getRemainBulletNum())));
            }
        }
    }
}

bool MainWidget::unitReachable(QPoint pos)
{
    QRect nextrect(pos.x() * UNIT, pos.y() * UNIT, 40, 40);

    if (nextrect.x() < 0 || nextrect.x() > 58 * UNIT + 12 || nextrect.y() < 0 || nextrect.y() > 58 * UNIT + 12)
    {
        return false; // 超出边界
    }

    for (int i = nextrect.x() / UNIT - 2; i <= nextrect.x() / UNIT + 2; i++)
    {
        for (int j = nextrect.y() / UNIT - 2; j <= nextrect.y() / UNIT + 2; j++)
        {
            if (unitOccupied.contains(QPoint(i, j)))
            {
                if (unitOccupied[QPoint(i,j)]->getTag() != "tree" && unitOccupied[QPoint(i, j)]->getRect().intersects(nextrect))
                {
                    return false; // 碰撞
                }
            }
        }
    }

    return true;
}

Direction MainWidget::moveToUnit(Tank *tank)
{
    QPoint start = tank->getPos();
    QPoint target = QPoint(tank->getNextPos().x() * UNIT, tank->getNextPos().y() * UNIT);
    if (start == target)
    {
        return Stop;
    }
    if (start.x() < target.x())
    {
        if (start.y() % UNIT != 0)
        {
            return Up;
        }
        else
        {
            return Right;
        }
    }
    else if (start.x() > target.x())
    {
        if (start.y() % UNIT != 0)
        {
            return Up;
        }
        else
        {
            return Left;
        }
    }
    else if (start.y() < target.y())
    {
        if (start.x() % UNIT != 0)
        {
            return Left;
        }
        else
        {
            return Down;
        }
    }
    else if (start.y() > target.y())
    {
        if (start.x() % UNIT != 0)
        {
            return Left;
        }
        else
        {
            return Up;
        }
    }
    return Stop;
}

int MainWidget::manhattanDistance(QPoint start, QPoint target)
{
    return abs(start.x() - target.x()) + abs(start.y() - target.y());
}

void MainWidget::sortInsert(QQueue<QPoint> &shortPath, const QHash<QPoint,int> &priority, const QPoint &point)
{
    for (int i = 0; i < shortPath.size(); i++)
    {
        if (priority[point] < priority[shortPath[i]])
        {
            shortPath.insert(i, point);
            return;
        }
    }
    shortPath.enqueue(point);
}

QPoint MainWidget::findNextDirection(Tank *tank, QPoint start, QPoint target) // 人机寻路, 使用A*算法
{
    QHash<QPoint, int> priority;
    QQueue<QPoint> shortPath;
    QHash<QPoint, QPoint> prePoint;
    QHash<QPoint, int> distance;
    QHash<QPoint, bool> visited;

    priority[start] = manhattanDistance(start, target);
    shortPath.enqueue(start);
    prePoint[start] = QPoint(-1, -1); // 使用一个特殊值表示起点
    distance[start] = 0;

    QPoint unitPoint[4] = {QPoint(0, -1), QPoint(0, 1), QPoint(-1, 0), QPoint(1, 0)};

    while (!shortPath.isEmpty())
    {
        QPoint current = shortPath.dequeue();
        visited[current] = true;

        if (current == target)
        {
            break;
        }

        for (int i = 0; i < 4; i++)
        {
            QPoint next = current + unitPoint[i];
            if (unitReachable(next) && !visited.contains(next))
            {
                int prior = manhattanDistance(next, target) + distance[current] + 1;
                if (!priority.contains(next) || priority[next] > prior)
                {
                    priority[next] = prior;
                    sortInsert(shortPath, priority, next);
                    prePoint[next] = current;
                    distance[next] = distance[current] + 1;
                }
            }
        }
    }

    tank->setDistance(distance[target]); // 将距离存储在坦克对象中

    // 回溯路径

    QPoint p = target;
    QPoint q;
    while (p != QPoint(-1, -1))
    {
        q = p;
        p = prePoint[p];
        if (p == start)
        {
            return q; // 返回下一个点
        }
    }

    return QPoint(-1, -1); // 没有路径
}

void MainWidget::enemyMove(Tank *enemy)
{
    Direction nextdir = moveToUnit(enemy); // 优先运动到nextPos
    if (nextdir == Stop) // 如果已经到达目标点
    {
        QPoint temppoint(-1,-1);
        QPoint enemyPos(enemy->getPos().x()/UNIT, enemy->getPos().y()/UNIT);

        if (enemy->getTempTargetPos() == enemyPos)
        {
            // 在15格范围附近随机选点
            do
            {
                int randX = qrand() % 15 - 7;
                int randY = qrand() % 15 - 7;
                temppoint = QPoint(enemyPos.x() + randX, enemyPos.y() + randY);
            } while (!unitReachable(temppoint));
            enemy->setTempTargetPos(temppoint);

            bool inTree = false;
            for (auto tree : treeGroup)
            {
                if (playerTank->getRect().intersects(tree->getRect()))
                {
                    inTree = true;
                    break;
                }
            }

            if (!inTree && manhattanDistance(enemyPos, QPoint(playerTank->getPos().x()/UNIT, playerTank->getPos().y()/UNIT)) < 40)
            {
                // 目标设为玩家
                enemy->setTempTargetPos(QPoint(playerTank->getPos().x()/UNIT, playerTank->getPos().y()/UNIT));
            }
            if (enemy->getLevel() == 2) // 目标设为基地
            {
                int possibility = qrand() % 10;
                if (possibility < 6 && manhattanDistance(enemyPos, enemy->getTargetPos()) < 70)
                {
                    enemy->setTempTargetPos(enemy->getTargetPos());
                }
            }
        }

        temppoint = findNextDirection(enemy, enemyPos, enemy->getTempTargetPos()); // 寻路到目标

        if (temppoint == QPoint(-1,-1))
        {
            return;
        }

        // 检测是否与玩家坦克碰撞
        QRect temprect(temppoint.x() * UNIT, temppoint.y() * UNIT, 40, 40);
        if (temprect.intersects(playerTank->getRect()))
        {
            return;
        }

        enemy->setNextPos(QPoint(temppoint.x(), temppoint.y())); // 设置下一个目标点
    }
}

void MainWidget::avoidBullet(Bullet *bullet, QPoint tankPos, Tank *enemy)
{
    int distance = 120;
    QPoint hidePoint(-1,-1);
    // 在附近5*5范围内寻找最近躲避点
    for (int i = tankPos.x() - 2; i <= tankPos.x() + 2; i++)
    {
        for (int j = tankPos.y() - 3; j <= tankPos.y() + 2; j++)
        {
            if (bullet->getShootRect().intersects(QRect(i * UNIT, j * UNIT, 40, 40)) || !unitReachable(QPoint(i, j)))
            {
                continue;
            }
            QPoint temp = findNextDirection(enemy, QPoint(enemy->getPos().x() / UNIT, enemy->getPos().y() / UNIT), QPoint(i, j));
            if (enemy->getDistance() < distance)
            {
                hidePoint = temp;
                distance = enemy->getDistance();
            }
        }
    }
    if (hidePoint != QPoint(-1,-1))
    {
        enemy->setNextPos(hidePoint); // 设置下一个目标点
    }
}

void MainWidget::ifShootPlayer(Tank *enemy)
{
    if (enemy->getRemainBulletNum() == 0)
    {
        return;
    }
    // 如果玩家在树下，则不射击
    for (auto tree : treeGroup)
    {
        if (playerTank->getRect().intersects(tree->getRect()))
        {
            return;
        }
    }
    QPoint enemyPos = enemy->getPos();
    QPoint playerPos = playerTank->getPos();
    Direction shootDir = Stop;
    QPoint unitPoint;
    if (enemyPos.x() < playerPos.x() && abs(enemyPos.y() - playerPos.y()) < UNIT)
    {
        shootDir = Right;
        unitPoint = QPoint(UNIT, 0);
    }
    else if (enemyPos.x() > playerPos.x() && abs(enemyPos.y() - playerPos.y()) < UNIT)
    {
        shootDir = Left;
        unitPoint = QPoint(-UNIT, 0);
    }
    else if (enemyPos.y() < playerPos.y() && abs(enemyPos.x() - playerPos.x()) < UNIT)
    {
        shootDir = Down;
        unitPoint = QPoint(0, UNIT);
    }
    else if (enemyPos.y() > playerPos.y() && abs(enemyPos.x() - playerPos.x()) < UNIT)
    {
        shootDir = Up;
        unitPoint = QPoint(0, -UNIT);
    }
    else
    {
        return;
    }

    // 沿射击区域前进，判断是否有障碍物遮挡
    while (true)
    {
        if (unitOccupied.contains(QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)) && unitOccupied[QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)]->getTag() != "river")
        {
            // 如果障碍物为墙且距离玩家近，则射击破墙
            if (unitOccupied[QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)]->getTag() == "brick"
                && ((abs(enemyPos.x() - playerPos.x()) < UNIT * 3 && shootDir < 2)
                || ((abs(enemyPos.y() - playerPos.y()) < UNIT * 3) && shootDir > 1)))
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
            }
            return;
        }
        switch (shootDir)
        {
        case Up:
            if (enemyPos.y() < playerPos.y())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        case Down:
            if (enemyPos.y() > playerPos.y())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        case Left:
            if (enemyPos.x() < playerPos.x())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        case Right:
            if (enemyPos.x() > playerPos.x())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        default:
            break;
        }
        enemyPos += unitPoint;
    }
}

void MainWidget::ifShootHome(Tank *enemy)
{
    if (enemy->getRemainBulletNum() == 0)
    {
        return;
    }
    QPoint enemyPos = enemy->getPos();
    QPoint homePos = home->getPos();
    if (manhattanDistance(enemyPos, homePos) > 15 * UNIT)
    {
        return;
    }
    Direction shootDir = Stop;
    QPoint unitPoint;
    if (enemyPos.x() < homePos.x() && abs(enemyPos.y() - homePos.y()) < UNIT)
    {
        shootDir = Right;
        unitPoint = QPoint(UNIT, 0);
    }
    else if (enemyPos.x() > homePos.x() && abs(enemyPos.y() - homePos.y()) < UNIT)
    {
        shootDir = Left;
        unitPoint = QPoint(-UNIT, 0);
    }
    else if (enemyPos.y() < homePos.y() && abs(enemyPos.x() - homePos.x()) < UNIT)
    {
        shootDir = Down;
        unitPoint = QPoint(0, UNIT);
    }
    else
    {
        return;
    }

    // 沿射击区域前进，判断是否有障碍物遮挡
    while (true)
    {
        if (unitOccupied.contains(QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)) && unitOccupied[QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)]->getTag() != "river")
        {
            // 如果障碍物为墙且距离基地近，则射击破墙
            if (unitOccupied[QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)]->getTag() == "brick"
                && ((abs(enemyPos.x() - homePos.x()) < UNIT * 3 && shootDir < 2)
                || ((abs(enemyPos.y() - homePos.y()) < UNIT * 3) && shootDir > 1)))
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
            }
            return;
        }
        switch (shootDir)
        {
        case Down:
            if (enemyPos.y() > homePos.y())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        case Left:
            if (enemyPos.x() < homePos.x())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        case Right:
            if (enemyPos.x() > homePos.x())
            {
                enemy->changeDirection(shootDir);
                shoot(enemy);
                return;
            }
            break;
        default:
            break;
        }
        enemyPos += unitPoint;
    }
}

bool MainWidget::ifInShootRect(Tank *tank, Bullet *bullet)
{
    return bullet->getShootRect().intersects(QRect(tank->getPos().x(), tank->getPos().y(), 40, 40));
}

void MainWidget::dealpickFood(FoodType foodtype)
{
    disconnect(this, &MainWidget::pickFood, this, &MainWidget::dealpickFood);

    switch (foodtype)
    {
    case Fix:
        playerTank->setUnbeatable(false); // 修复玩家耐久
        ui->healthLabel->setText(getWhiteString(QString::number(playerTank->getHealth()))); // 更新标签

        if (fixTimer)
        {
            if (fixTimer->isActive())
            {
                fixTimer->stop();
            }
            delete fixTimer;
            fixTimer = NULL;
        }
        for (int j = 57; j < 60; j++)
        {
            if (unitOccupied.contains(QPoint(28,j)))
            {
                brickGroup.removeOne(unitOccupied[QPoint(28,j)]);
            }
            if (unitOccupied.contains(QPoint(32,j)))
            {
                brickGroup.removeOne(unitOccupied[QPoint(32,j)]);
            }

            ironGroup.append(new Obstacle(28 * UNIT, j * UNIT, Iron));
            unitOccupied[QPoint(28,j)] = ironGroup.last();
            ironGroup.append(new Obstacle(31 * UNIT, j * UNIT, Iron));
            unitOccupied[QPoint(31,j)] = ironGroup.last();
        }
        if (unitOccupied.contains(QPoint(29,57)))
        {
            brickGroup.removeOne(unitOccupied[QPoint(29,57)]);
        }
        if (unitOccupied.contains(QPoint(30,57)))
        {
            brickGroup.removeOne(unitOccupied[QPoint(30,57)]);
        }
        ironGroup.append(new Obstacle(29 * UNIT, 57 * UNIT, Iron));
        unitOccupied[QPoint(29,57)] = ironGroup.last();
        ironGroup.append(new Obstacle(30 * UNIT, 57 * UNIT, Iron));
        unitOccupied[QPoint(30,57)] = ironGroup.last();

        fixTimer = new QTimer(this);
        fixTimer->start(5000);
        connect(fixTimer, &QTimer::timeout, this, [=](){
            fixTimer->stop();
            for (int j = 57; j < 60; j++)
            {
                ironGroup.removeOne(unitOccupied[QPoint(28,j)]);
                ironGroup.removeOne(unitOccupied[QPoint(31,j)]);
                brickGroup.append(new Obstacle(28 * UNIT, j * UNIT, Brick));
                unitOccupied[QPoint(28,j)] = brickGroup.last();
                brickGroup.append(new Obstacle(31 * UNIT, j * UNIT, Brick));
                unitOccupied[QPoint(31,j)] = brickGroup.last();
            }
            ironGroup.removeOne(unitOccupied[QPoint(29,57)]);
            ironGroup.removeOne(unitOccupied[QPoint(30,57)]);
            brickGroup.append(new Obstacle(29 * UNIT, 57 * UNIT, Brick));
            unitOccupied[QPoint(29,57)] = brickGroup.last();
            brickGroup.append(new Obstacle(30 * UNIT, 57 * UNIT, Brick));
            unitOccupied[QPoint(30,57)] = brickGroup.last();
            this->update();
        });
        break;
    case Heart:
        information.playerlife++; // 玩家生命+1
        ui->playerLifeLabel->setText(getWhiteString(QString::number(information.playerlife))); // 更新标签
        break;
    case Star:
        playerTank->upgrade(); // 玩家升级
        ui->levelLabel->setText(getWhiteString(QString::number(playerTank->getLevel()))); // 更新标签
        ui->healthLabel->setText(getWhiteString(QString::number(playerTank->getHealth()))); // 更新标签
        break;
    case Shell:
        playerTank->setShellInf(true); // 玩家子弹无限
        ui->shellLabel->setText(getWhiteString("inf")); // 更新标签
        if (shellInfTimer)
        {
            if (shellInfTimer->isActive())
            {
                shellInfTimer->stop();
            }
            delete shellInfTimer;
            shellInfTimer = NULL;
        }
        shellInfTimer = new QTimer(this);
        shellInfTimer->start(15000); // 15秒后取消
        connect(shellInfTimer, &QTimer::timeout, this, [=](){
            shellInfTimer->stop();
            playerTank->setShellInf(false);
            ui->shellLabel->setText(getWhiteString("2"));
        });
        break;
    case Laser:
        break;
    default:
        break;
    }
}
