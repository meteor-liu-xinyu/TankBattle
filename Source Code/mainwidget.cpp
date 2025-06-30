#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    this->setWindowTitle("坦克大战");

    int height = this->size().height();
    background = new Entity(height / unitNum);
    background->setRect(QRect(0, 0, height, height));
    background->setPixmap(":/background/images/cover.png");

    stopwidget = new StopWidget(this); // 创建暂停窗口
    stopwidget->resize(this->size().height() / 3, stopwidget->size().height() * this->size().height() / 3 / stopwidget->size().width());
    stopwidget->move(this->size().height() / 2 - stopwidget->size().width() / 2, this->size().height() / 2 - stopwidget->size().height() / 2);
    stopwidget->hide();

    information.playerlife = initPlayerLives;
    information.enemyleft = initEnemyNum;
    informationui = new InformationUI(this, initEnemyNum, initPlayerLives, 2);
    informationui->resize(265, this->size().height());
    informationui->move(this->size().height(), 0);
    informationui->show();
    connect(informationui, &InformationUI::resetPressed, this, &MainWidget::startGame);
}

void MainWidget::resizeEvent(QResizeEvent *event)
{
    int height = event->size().height();

    this->setMinimumWidth(height + 265);

    if (timer)
    {
        if (timer->isActive())
        {
            stopGame(); // 暂停游戏
        }

        // 设置unitsize
        unitSize = height / unitNum;

        // 更新实体位置大小

        // lambda表达式简化代码
        auto obstacleSetSize = [&](QList<Obstacle*> entityGroup)
        {
            for (auto entity : entityGroup)
            {
                entity->setUnitSize(unitSize);
            }
        };
        obstacleSetSize(brickGroup);
        obstacleSetSize(ironGroup);
        obstacleSetSize(riverGroup);
        obstacleSetSize(treeGroup);

        if (home)
        {
            home->setUnitSize(unitSize);
        }

        tankSpeed = unitSize * 5 / tickrate;
        if (playerTank)
        {
            playerTank->setUnitSize(unitSize);
            playerTank->setTankSpeed(tankSpeed); // 更新玩家坦克速度
        }
        for (int i = 0; i < enemyTank.size(); i++)
        {
            enemyTank[i]->setUnitSize(unitSize);
            enemyTank[i]->setTankSpeed(tankSpeed); // 更新敌人坦克速度
        }
        for (int i = 0; i < booms.size(); i++)
        {
            booms[i]->setUnitSize(unitSize);
        }
        for (int i = 0; i < foods.size(); i++)
        {
            foods[i]->setUnitSize(unitSize);
        }
        for (int i = 0; i < bullets.size(); i++)
        {
            bullets[i]->setUnitSize(unitSize);
        }

        tankSpeed = unitSize * 5 / tickrate;
        if (playerTank)
        {
            playerTank->setTankSpeed(tankSpeed); // 更新玩家坦克速度
        }
        for (int i = 0; i < enemyTank.size(); i++)
        {
            enemyTank[i]->setTankSpeed(tankSpeed); // 更新敌人坦克速度
        }

        update();
    }
    else
    {
        if (background)
        {
            background->setRect(QRect(0, 0, height, height));
            background->setPixmap(":/background/images/cover.png");
        }
    }

    if (informationui)
    {
        informationui->resize(265, event->size().height());
        informationui->move(event->size().height(), 0);
    }
    if (stopwidget)
    {
        stopwidget->resize(event->size().height() / 3, stopwidget->size().height() * event->size().height() / 3 / stopwidget->size().width());
        stopwidget->move(event->size().height() / 2 - stopwidget->size().width() / 2, event->size().height() / 2 - stopwidget->size().height() / 2);
    }
    if (gameoverui)
    {
        gameoverui->resize(this->size().width() / 2, this->size().height() / 2);
        gameoverui->move(this->size().height() / 2 - gameoverui->size().width() / 2, this->size().height() / 2 - gameoverui->size().height() / 2);
    }

    QWidget::resizeEvent(event);
}

void MainWidget::startGame()
{
    disconnect(stopwidget, &StopWidget::restart, nullptr, nullptr); // 断开信号槽连接
    if (!gameend || timer) // 如果游戏未结束或计时器未清除
    {
        gameend = true;
        gameEnded();
    }

    initGame(); // 初始化游戏
}

void MainWidget::initGame()
{
    // 清除之前的游戏数据
    gameend = false;
    win = false;
    unitOccupied.clear();
    for (int i = 0; i < enemyTank.size(); i++)
    {
        enemyTank[i]->deleteLater();
    }
    enemyTank.clear();
    delete playerTank;
    playerTank = NULL;
    delete home;
    home = NULL;
    for (int i = 0; i < brickGroup.size(); i++)
    {
        delete brickGroup[i];
    }
    brickGroup.clear();
    for (int i = 0; i < ironGroup.size(); i++)
    {
        delete ironGroup[i];
    }
    ironGroup.clear();
    for (int i = 0; i < riverGroup.size(); i++)
    {
        delete riverGroup[i];
    }
    riverGroup.clear();
    for (int i = 0; i < treeGroup.size(); i++)
    {
        delete treeGroup[i];
    }
    treeGroup.clear();
    homeDestroyed = false;
    for (int i = 0; i < foods.size(); i++)
    {
        delete foods[i];
    }
    foods.clear();
    countSimpleEnemy = 0;
    movingDirection.clear();
    for (int i = 0; i < bullets.size(); i++)
    {
        delete bullets[i];
    }
    bullets.clear();
    for (int i = 0; i < booms.size(); i++)
    {
        delete booms[i];
    }
    booms.clear();

    // 获取一个全局随机数生成器
    randgenerate = QRandomGenerator::global();

    unitSize = qMin(this->size().width(), this->size().height()) / unitNum;
    tankSpeed = unitSize * 5 / tickrate;

    generateGameMap(); // 生成游戏地图
    generatePlayerTank(); // 生成玩家坦克
    for (int i = 0; i < 1; i++) // 生成敌人坦克
    {
        generateEnemyTank();
    }
    generateFood(); // 生成道具


    if (stopwidget)
    {
        stopwidget->hide(); // 隐藏暂停窗口
        delete stopwidget; // 删除暂停窗口
        stopwidget = NULL;
    }
    if (gameoverui)
    {
        gameoverui->hide(); // 隐藏游戏结束界面
        delete gameoverui; // 删除游戏结束界面
        gameoverui = NULL;
    }

    information.enemyleft = initEnemyNum;
    information.playerlife = initPlayerLives;
    informationui->reset(); // 重置信息界面

    // 初始化计时器
    timer = new QTimer(this);
    timer->start(1000 / tickrate); // 设置每帧时间
    connect(timer, &QTimer::timeout, this, &MainWidget::updateGame);
}

void MainWidget::updateGame() // 更新游戏
{
    // 更新计时器
    millisecond += 1000.0 / tickrate;
    if (millisecond >= 1000)
    {
        millisecond -= 1000;
        second++;
        if (second == 60)
        {
            second = 0;
            min++;
        }
    }
    informationui->setTime(min, second, (int)millisecond);

    // 道具定时生成
    if (second % 20 == 0 && foods.size() == 0)
    {
        if (randgenerate->bounded(10) < 8)
        {
            generateFood();
        }
    }

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
        for (const auto river : riverGroup)
        {
            if (river->getRect().intersects(playerTank->getRect()))
            {
                emit playerTankHit();
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
                            informationui->setShell(-1);
                        }
                        else
                        {
                            informationui->setShell(playerTank->getRemainBulletNum());
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
        for (int i = 0; i < enemyTank.size(); i++)
        {
            if (enemyTank[i]->getLevel() != 2)
            {
                // 更新玩家位置作为目标
                enemyTank[i]->setTargetPos(QPoint(playerTank->getPos().x()/unitSize, playerTank->getPos().y()/unitSize));
            }

            enemyMove(enemyTank[i]); // 敌人移动

            // 检测是否要躲子弹
            for (const auto bullet : bullets)
            {
                if (ifInShootRect(enemyTank[i], bullet) && bullet->getOwner() != enemyTank[i])
                {
                    avoidBullet(bullet, enemyTank[i]->getNextPos(), enemyTank[i]);
                }
            }
            Direction nextdir = moveToUnit(enemyTank[i]); // 获取下一刻运动方向
            ifShootPlayer(enemyTank[i]); // 判断是否射击玩家
            ifShootHome(enemyTank[i]); // 判断是否射击基地
            // 移动
            switch (nextdir)
            {
            case Up:
                enemyTank[i]->moveUp();
                break;
            case Down:
                enemyTank[i]->moveDown();
                break;
            case Left:
                enemyTank[i]->moveLeft();
                break;
            case Right:
                enemyTank[i]->moveRight();
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
            gameoverui->resize(this->size().width() / 2, this->size().height() / 2);
            gameoverui->move(this->size().height() / 2 - gameoverui->size().width() / 2, this->size().height() / 2 - gameoverui->size().height() / 2);
            gameoverui->show();
            connect(gameoverui, &GameoverUI::restart, this, &MainWidget::startGame);
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

    if (unbeatableTimer)
    {
        unbeatableTimer->stop();
        disconnect(unbeatableTimer, &QTimer::timeout, nullptr, nullptr);
        delete unbeatableTimer;
    }
    unbeatableTimer = NULL;

    if (fixTimer)
    {
        fixTimer->stop();
        disconnect(fixTimer, &QTimer::timeout, nullptr, nullptr);
        delete fixTimer;
    }
    fixTimer = NULL;

    if (shellInfTimer)
    {
        shellInfTimer->stop();
        disconnect(shellInfTimer, &QTimer::timeout, nullptr, nullptr);
        delete shellInfTimer;
    }
    shellInfTimer = NULL;

    informationui->setWin(win);
}

void MainWidget::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_R) // 按下R键重新开始
    {
        startGame();
    }
    else if (k->key() == Qt::Key_Space && !gameend && playerTank) // 按下空格键射击
    {
        shoot(playerTank);
    }
    else if (k->key() == Qt::Key_Escape) // 按下Esc键暂停
    {
        stopGame();
    }
    if (timer && timer->isActive())
    {
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
}

void MainWidget::stopGame()
{
    if (gameend)
    {
        return;
    }
    if (timer)
    {
        if (timer->isActive())
        {
            timer->stop();
            movingDirection.clear(); // 清空移动方向
            if (!stopwidget) // 没有暂停窗口
            {
                stopwidget = new StopWidget(this);
                stopwidget->resize(this->size().height() / 3, stopwidget->size().height() * this->size().height() / 3 / stopwidget->size().width());
                stopwidget->move(this->size().height() / 2 - stopwidget->size().width() / 2, this->size().height() / 2 - stopwidget->size().height() / 2);
            }
            stopwidget->show();
            connect(stopwidget, &StopWidget::restart, this, &MainWidget::startGame);
            connect(stopwidget, &StopWidget::continuesgn, this, [=](){
                disconnect(stopwidget, &StopWidget::continuesgn, nullptr, nullptr);
                timer->start();
                if (stopwidget)
                {
                    stopwidget->hide();
                }
            });
        }
        else
        {
            timer->start();
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

    if (background)
    {
        background->draw(painter);
    }

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
    for (int i = 0; i < enemyTank.size(); i++)
    {
        enemyTank[i]->draw(painter);
    }
    for (int i = 0; i < booms.size(); i++)
    {
        booms[i]->draw(painter);
    }
    for (int i = 0; i < foods.size(); i++)
    {
        foods[i]->draw(painter);
    }
    for (int i = 0; i < bullets.size(); i++)
    {
        bullets[i]->draw(painter);
    }
}

void MainWidget::generatePlayerTank()
{
    // 生成玩家坦克
    playerTank = new Tank((unitNum / 2 - 1) * unitSize, (unitNum - 6) * unitSize, PlayerTank, 1, unitSize);
    playerTank->setTankSpeed(tankSpeed); // 设置玩家坦克速度
    playerTank->setUnbeatable(true);

    informationui->setHealth(-1); // 设置玩家生命数为无限

    // 设置无敌状态
    unbeatableTimer = new QTimer(this);
    unbeatableTimer->start(3000);
    connect(unbeatableTimer, &QTimer::timeout, this, [=](){
        playerTank->setUnbeatable(false);
        informationui->setHealth(playerTank->getHealth()); // 设置玩家生命数
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
        x = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
        y = (randgenerate->bounded(unitNum / 6) + 1) * 2;
        if (!unitOccupied.contains(QPoint(x, y)))
        {
            // 不生成在玩家附近
            if (playerTank && (abs(playerTank->getPos().x()/unitSize - x) > 6 && abs(playerTank->getPos().y()/unitSize - y) > 3))
            {
                break;
            }
        }
    }
    int enemyLevel = randgenerate->bounded(5) + 1; // 1-4
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

    enemyTank.append(new Tank(x * unitSize, y * unitSize, EnemyTank, enemyLevel, unitSize, enemyTank.size()));
    enemyTank.last()->setTankSpeed(tankSpeed); // 设置敌人坦克速度

    // 设置目标
    QPoint targetPos;
    if (enemyTank.last()->getLevel() == 2)
    {
        int possibility = randgenerate->bounded(5);
        if (possibility < 2)
        {
            targetPos = QPoint(unitNum / 2 - 4, unitNum - 2);
        }
        else if (possibility == 2)
        {
            targetPos = QPoint(unitNum / 2 + 2, unitNum - 2);
        }
        else
        {
            targetPos = QPoint(unitNum / 2 - 1, unitNum - 6);
        }
    }
    else
    {
        targetPos = QPoint(playerTank->getPos().x()/unitSize, playerTank->getPos().y()/unitSize);
    }
}

void MainWidget::generateFood(int x, int y)
{
    if (x == -1 &&y == -1) // 如果没有指定位置
    {
        // 随机生成道具位置
        while (true)
        {
            x = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
            y = (randgenerate->bounded(unitNum / 3) + 1) * 2;
            if (!(unitOccupied.contains(QPoint(x, y)) || unitOccupied.contains(QPoint(x + 1, y)) || unitOccupied.contains(QPoint(x, y + 1))))
            {
                break;
            }
        }
    }

    int foodType = randgenerate->bounded(4); // 0-3
    Food *food = new Food(x * unitSize, y * unitSize, foodType, unitSize);
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
    if (background)
    {
        delete background;
        background = NULL;
    }

    // 生成基地（固定在底部中央位置）
    home = new Obstacle((unitNum / 2 - 1) * unitSize, (unitNum - 2) * unitSize, Home, unitSize);

    // 生成基地保护墙（左/右/上三个方向，每个方向生成unitSizexunitSize单块砖）
    for (int j = unitNum - 3; j < unitNum; j++)
    {
        brickGroup.append(new Obstacle((unitNum / 2 - 2) * unitSize, j * unitSize, Brick, unitSize));
        unitOccupied[QPoint(unitNum / 2 - 2,j)] = brickGroup.last();
        brickGroup.append(new Obstacle((unitNum / 2 + 1) * unitSize, j * unitSize, Brick, unitSize));
        unitOccupied[QPoint(unitNum / 2 + 1,j)] = brickGroup.last();
    }
    brickGroup.append(new Obstacle((unitNum / 2 - 1) * unitSize, (unitNum - 3) * unitSize, Brick, unitSize));
    unitOccupied[QPoint(unitNum / 2 - 1,unitNum - 3)] = brickGroup.last();
    brickGroup.append(new Obstacle((unitNum / 2) * unitSize, (unitNum - 3) * unitSize, Brick, unitSize));
    unitOccupied[QPoint(unitNum / 2,unitNum - 3)] = brickGroup.last();

    // 生成河水
    if (randgenerate->bounded(10) < 7) // 70%概率生成
    {
        for (int x = unitNum / 4 - unitNum / 12; x < unitNum / 4 + unitNum / 12; x++)
        {
            generateRiverGroup(x * 2, unitNum / 5 * 2);
        }
    }

    // 固定生成基地上方的铁块
    int row = unitNum - 12;
    generateIronGroup(unitNum / 2 - 4, row);
    generateIronGroup(unitNum / 2 - 2, row);
    generateIronGroup(unitNum / 2, row);
    generateIronGroup(unitNum / 2 + 2, row);

    // 半固定生成砖墙
    for (int x = unitNum / 2 - 2; x > 0; x -= 4)
    {
        for (int j = 0; j < unitNum / 6;)
        {
            int y = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
            // 检查未被占用
            if (!unitOccupied.contains(QPoint(x, y)) && !nearHome(x, y))
            {
                generateBrickGroup(x , y); // 生成2x2砖块组
                j++;
            }
        }
        for (int j = 0; j < unitNum / 6;)
        {
            int y = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
            // 检查未被占用
            if (!unitOccupied.contains(QPoint(unitNum - 2 - x, y)) && !nearHome(unitNum - 2 - x, y))
            {
                generateBrickGroup(58 - x , y); // 生成2x2砖块组
                j++;
            }
        }
    }

    // 生成铁墙
    for (int i = 0; i < unitNum * unitNum / 180; )
    {
        int x = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
        int y = randgenerate->bounded(unitNum / 2 - 4) * 2;

        if (unitOccupied.contains(QPoint(x, y)) || unitOccupied.contains(QPoint(x + 1, y)) || unitOccupied.contains(QPoint(x, y + 1)) || unitOccupied.contains(QPoint(x + 1, y + 1)))
        {
            continue;
        }
        else
        {
            int type = randgenerate->bounded(3);
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
    int minRandomBricks = unitNum * unitNum / 180;
    int maxRandomBricks = unitNum * unitNum / 72;
    int randomBrickGroups = minRandomBricks + (randgenerate->bounded(maxRandomBricks - minRandomBricks + 1));

    for (int i = 0; i < randomBrickGroups; )
    {
        int x = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
        int y = (randgenerate->bounded(unitNum / 2 - 3) + 1) * 2;

        // 检查未被占用
        if (!unitOccupied.contains(QPoint(x, y)) && !nearHome(x, y))
        {
            generateBrickGroup(x, y); // 生成2x2砖块组
            i++;
        }
    }

    // 生成树叶
    for (int i = 0; i < unitNum * unitNum / 225; )
    {
        int x = (randgenerate->bounded(unitNum / 2 - 2) + 1) * 2;
        int y = (randgenerate->bounded(unitNum / 2 - 3) + 1) * 2;  // 避免最底部行

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
    // 生成4个砖块（2x2布局覆盖48x48 unitSize）
    brickGroup.append(new Obstacle(x * unitSize, y * unitSize, Brick, unitSize));
    unitOccupied[QPoint(x,y)] = brickGroup.last();
    brickGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize, Brick, unitSize));
    unitOccupied[QPoint(x+1,y)] = brickGroup.last();
    brickGroup.append(new Obstacle(x * unitSize, y * unitSize + unitSize, Brick, unitSize));
    unitOccupied[QPoint(x,y+1)] = brickGroup.last();
    brickGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize + unitSize, Brick, unitSize));
    unitOccupied[QPoint(x+1,y+1)] = brickGroup.last();
}

void MainWidget::generateIronGroup(int x, int y)
{
    // 生成2个铁块（水平排列）
    ironGroup.append(new Obstacle(x * unitSize, y * unitSize, Iron, unitSize));
    unitOccupied[QPoint(x,y)] = ironGroup.last();
    ironGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize, Iron, unitSize));
    unitOccupied[QPoint(x+1,y)] = ironGroup.last();
}

void MainWidget::generateRiverGroup(int x, int y)
{
    // 生成4个河水（4x4布局）
    riverGroup.append(new Obstacle(x * unitSize, y * unitSize, River, unitSize));
    unitOccupied[QPoint(x,y)] = riverGroup.last();
    riverGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize, River, unitSize));
    unitOccupied[QPoint(x+1,y)] = riverGroup.last();
    riverGroup.append(new Obstacle(x * unitSize, y * unitSize + unitSize, River, unitSize));
    unitOccupied[QPoint(x,y+1)] = riverGroup.last();
    riverGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize + unitSize, River, unitSize));
    unitOccupied[QPoint(x+1,y+1)] = riverGroup.last();
}

void MainWidget::generateTreeGroup(int x, int y)
{
    // 生成16个树叶（4x4布局）
    treeGroup.append(new Obstacle(x * unitSize, y * unitSize, Tree, unitSize));
    treeGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize, Tree, unitSize));
    treeGroup.append(new Obstacle(x * unitSize, y * unitSize + unitSize, Tree, unitSize));
    treeGroup.append(new Obstacle(x * unitSize + unitSize, y * unitSize + unitSize, Tree, unitSize));
}

bool MainWidget::nearHome(int x, int y)
{
    if (x >= unitNum / 2 - 5 && x < unitNum / 2 + 5 && y >= unitNum - 6 && y < unitNum)
    {
        return true; // 在基地附近
    }
    else
    {
        return false; // 不在基地附近
    }
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
    for (const auto river : riverGroup)
    {
        if (river->getRect().intersects(playerTank->getRect()))
        {
            inRiver = true;
            break;
        }
    }

    if (playerTank->getHealth() == 1 || inRiver)
    {
        booms.append(new Boom(playerTank->getRect(), unitSize));

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
            informationui->setHealth(-1); // 设置玩家生命数为无限
            informationui->setPlayerLife(information.playerlife);
            informationui->setLevel(playerTank->getLevel());
            informationui->setShell(playerTank->getRemainBulletNum());
        }
    }
    else if (playerTank->getHealth() != -1)
    {
        playerTank->healthDecrease();
        informationui->setHealth(playerTank->getHealth()); // 更新生命数标签
    }
}

void MainWidget::dealenemyTankHit(int i)
{
    disconnect(this, &MainWidget::enemyTankHit, this, &MainWidget::dealenemyTankHit);

    if (enemyTank[i]->getHealth() == 1)
    {
        information.enemyleft--;
        booms.append(new Boom(enemyTank[i]->getRect(), unitSize));
        if (enemyTank[i]->getLevel() == 4) // 目标坦克击败后生成道具
        {
            generateFood(enemyTank[i]->getPos().x()/unitSize, enemyTank[i]->getPos().y()/unitSize);
        }
        enemyTank.removeAt(i);
        informationui->setEnemyLeft(information.enemyleft);
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
        Bullet *bullet = new Bullet(tank, unitSize);
        bullets.append(bullet);
        tank->setBullet(bullet);
        if (tank == playerTank)
        {
            informationui->setShell(playerTank->getRemainBulletNum());
        }
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
        nextrect.moveTo(nextrect.x(), nextrect.y() - tankSpeed);
        break;
    case Down:
        nextrect.moveTo(nextrect.x(), nextrect.y() + tankSpeed);
        break;
    case Left:
        nextrect.moveTo(nextrect.x() - tankSpeed, nextrect.y());
        break;
    case Right:
        nextrect.moveTo(nextrect.x() + tankSpeed, nextrect.y());
        break;
    default:
        break;
    }

    if (nextrect.x() < 0 || nextrect.x() > (unitNum - 2) * unitSize + unitSize / 2 || nextrect.y() < 0 || nextrect.y() > (unitNum - 2) * unitSize + unitSize / 2)
    {
        return true; // 超出边界
    }

    // 检测附近的障碍物是否有碰撞
    for (int i = nextrect.x() / unitSize - 2; i <= nextrect.x() / unitSize + 2; i++)
    {
        for (int j = nextrect.y() / unitSize - 2; j <= nextrect.y() / unitSize + 2; j++)
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

    nextrect = QRect(nextrect.x(), nextrect.y(), nextrect.width() - unitSize / 4, nextrect.height() - unitSize / 4);

    // 检测敌人是否有碰撞
    for (const auto enemy : enemyTank)
    {
        if (nextrect.intersects(enemy->getRect()))
        {
            return true; // 碰撞
        }
    }

    // 检测是否捡到道具
    for (int i = 0; i < foods.size(); i++)
    {
        if (nextrect.intersects(foods[i]->getRect()))
        {
            emit pickFood(foods[i]->getFoodType()); // 发射信号
            foods.removeAt(i); // 移除道具
            i--; // 避免越界
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

    if (nextrect.x() < 0 || nextrect.x() > (unitNum - 1) * unitSize + unitSize / 2 || nextrect.y() < 0 || nextrect.y() > (unitNum - 1) * unitSize + unitSize / 2)
    {
        return true; // 超出边界
    }

    bool flag = false;
    // 检测附近的障碍物是否有碰撞
    for (int i = nextrect.x() / unitSize - 2; i <= nextrect.x() / unitSize + 2; i++)
    {
        for (int j = nextrect.y() / unitSize - 2; j <= nextrect.y() / unitSize + 2; j++)
        {
            if (unitOccupied.contains(QPoint(i, j)))
            {
                if (nextrect.intersects(unitOccupied[QPoint(i,j)]->getRect()))
                {
                    if (unitOccupied[QPoint(i,j)]->getTag() == "brick") // 砖块
                    {
                        booms.append(new Boom(unitOccupied[QPoint(i,j)]->getRect(), unitSize)); // 爆炸
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
        booms.append(new Boom(home->getRect(), unitSize));
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
                booms.append(new Boom(bullet->getRect(), unitSize));
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
            booms.append(new Boom(bullet->getRect(), unitSize));
            return true;
        }
    }

    return false;
}

bool MainWidget::unitReachable(QPoint pos)
{
    QRect nextrect(pos.x() * unitSize, pos.y() * unitSize, unitSize * 5 / 3, unitSize * 5 / 3);

    if (nextrect.x() < 0 || nextrect.x() > (unitNum - 2) * unitSize + unitSize / 2 || nextrect.y() < 0 || nextrect.y() > (unitNum - 2) * unitSize + unitSize / 2)
    {
        return false; // 超出边界
    }

    for (int i = nextrect.x() / unitSize - 2; i <= nextrect.x() / unitSize + 2; i++)
    {
        for (int j = nextrect.y() / unitSize - 2; j <= nextrect.y() / unitSize + 2; j++)
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
    QPoint target = QPoint(tank->getNextPos().x() * unitSize, tank->getNextPos().y() * unitSize);
    if (start == target)
    {
        return Stop;
    }
    if (start.x() < target.x())
    {
        if (start.y() % unitSize != 0)
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
        if (start.y() % unitSize != 0)
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
        if (start.x() % unitSize != 0)
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
        if (start.x() % unitSize != 0)
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
        QPoint enemyPos(enemy->getPos().x()/unitSize, enemy->getPos().y()/unitSize);

        if (enemy->getTempTargetPos() == enemyPos)
        {
            // 在15格范围附近随机选点
            do
            {
                int randX = randgenerate->bounded(15) - 7;
                int randY = randgenerate->bounded(15) - 7;
                temppoint = QPoint(enemyPos.x() + randX, enemyPos.y() + randY);
            } while (!unitReachable(temppoint));
            enemy->setTempTargetPos(temppoint);

            bool inTree = false;
            for (const auto tree : treeGroup)
            {
                if (playerTank->getRect().intersects(tree->getRect()))
                {
                    inTree = true;
                    break;
                }
            }

            if (!inTree && manhattanDistance(enemyPos, QPoint(playerTank->getPos().x()/unitSize, playerTank->getPos().y()/unitSize)) < 40)
            {
                // 目标设为玩家
                enemy->setTempTargetPos(QPoint(playerTank->getPos().x()/unitSize, playerTank->getPos().y()/unitSize));
            }
            if (enemy->getLevel() == 2) // 目标设为基地
            {
                if (randgenerate->bounded(10) < 6 && manhattanDistance(enemyPos, enemy->getTargetPos()) < 70)
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
        QRect temprect(temppoint.x() * unitSize, temppoint.y() * unitSize, unitSize * 5 / 3, unitSize * 5 / 3);
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
            if (bullet->getShootRect().intersects(QRect(i * unitSize, j * unitSize, unitSize * 5 / 3, unitSize * 5 / 3)) || !unitReachable(QPoint(i, j)))
            {
                continue;
            }
            QPoint temp = findNextDirection(enemy, QPoint(enemy->getPos().x() / unitSize, enemy->getPos().y() / unitSize), QPoint(i, j));
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
    for (const auto tree : treeGroup)
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
    if (enemyPos.x() < playerPos.x() && abs(enemyPos.y() - playerPos.y()) < unitSize)
    {
        shootDir = Right;
        unitPoint = QPoint(unitSize, 0);
    }
    else if (enemyPos.x() > playerPos.x() && abs(enemyPos.y() - playerPos.y()) < unitSize)
    {
        shootDir = Left;
        unitPoint = QPoint(-unitSize, 0);
    }
    else if (enemyPos.y() < playerPos.y() && abs(enemyPos.x() - playerPos.x()) < unitSize)
    {
        shootDir = Down;
        unitPoint = QPoint(0, unitSize);
    }
    else if (enemyPos.y() > playerPos.y() && abs(enemyPos.x() - playerPos.x()) < unitSize)
    {
        shootDir = Up;
        unitPoint = QPoint(0, -unitSize);
    }
    else
    {
        return;
    }

    // 沿射击区域前进，判断是否有障碍物遮挡
    while (true)
    {
        if (unitOccupied.contains(QPoint(enemyPos.x() / unitSize, enemyPos.y() / unitSize)) && unitOccupied[QPoint(enemyPos.x() / unitSize, enemyPos.y() / unitSize)]->getTag() != "river")
        {
            // 如果障碍物为墙且距离玩家近，则射击破墙
            if (unitOccupied[QPoint(enemyPos.x() / unitSize, enemyPos.y() / unitSize)]->getTag() == "brick"
                && ((abs(enemyPos.x() - playerPos.x()) < unitSize * 3 && shootDir < 2)
                || ((abs(enemyPos.y() - playerPos.y()) < unitSize * 3) && shootDir > 1)))
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
    if (manhattanDistance(enemyPos, homePos) > 15 * unitSize)
    {
        return;
    }
    Direction shootDir = Stop;
    QPoint unitPoint;
    if (enemyPos.x() < homePos.x() && abs(enemyPos.y() - homePos.y()) < unitSize)
    {
        shootDir = Right;
        unitPoint = QPoint(unitSize, 0);
    }
    else if (enemyPos.x() > homePos.x() && abs(enemyPos.y() - homePos.y()) < unitSize)
    {
        shootDir = Left;
        unitPoint = QPoint(-unitSize, 0);
    }
    else if (enemyPos.y() < homePos.y() && abs(enemyPos.x() - homePos.x()) < unitSize)
    {
        shootDir = Down;
        unitPoint = QPoint(0, unitSize);
    }
    else
    {
        return;
    }

    // 沿射击区域前进，判断是否有障碍物遮挡
    while (true)
    {
        if (unitOccupied.contains(QPoint(enemyPos.x() / unitSize, enemyPos.y() / unitSize)) && unitOccupied[QPoint(enemyPos.x() / unitSize, enemyPos.y() / unitSize)]->getTag() != "river")
        {
            // 如果障碍物为墙且距离基地近，则射击破墙
            if (unitOccupied[QPoint(enemyPos.x() / unitSize, enemyPos.y() / unitSize)]->getTag() == "brick"
                && ((abs(enemyPos.x() - homePos.x()) < unitSize * 3 && shootDir < 2)
                || ((abs(enemyPos.y() - homePos.y()) < unitSize * 3) && shootDir > 1)))
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
    return bullet->getShootRect().intersects(QRect(tank->getPos().x(), tank->getPos().y(), unitSize * 5 / 3, unitSize * 5 / 3));
}

void MainWidget::dealpickFood(FoodType foodtype)
{
    disconnect(this, &MainWidget::pickFood, this, &MainWidget::dealpickFood);

    switch (foodtype)
    {
    case Fix:
        playerTank->setUnbeatable(false); // 修复玩家耐久
        informationui->setHealth(playerTank->getHealth());

        if (fixTimer)
        {
            if (fixTimer->isActive())
            {
                fixTimer->stop();
            }
            delete fixTimer;
            fixTimer = NULL;
        }

        for (int j = unitNum - 3; j < unitNum; j++)
        {
            if (unitOccupied.contains(QPoint(unitNum / 2 - 2,j)))
            {
                brickGroup.removeOne(unitOccupied[QPoint(unitNum / 2 - 2, j)]);
            }
            if (unitOccupied.contains(QPoint(unitNum / 2 + 1,j)))
            {
                brickGroup.removeOne(unitOccupied[QPoint(unitNum / 2 + 1, j)]);
            }

            ironGroup.append(new Obstacle((unitNum / 2 - 2) * unitSize, j * unitSize, Iron, unitSize));
            unitOccupied[QPoint(unitNum / 2 - 2, j)] = ironGroup.last();
            ironGroup.append(new Obstacle((unitNum / 2 + 1) * unitSize, j * unitSize, Iron, unitSize));
            unitOccupied[QPoint(unitNum / 2 + 1, j)] = ironGroup.last();
        }
        if (unitOccupied.contains(QPoint(unitNum / 2 - 1, unitNum - 3)))
        {
            brickGroup.removeOne(unitOccupied[QPoint(unitNum / 2 - 1, unitNum - 3)]);
        }
        if (unitOccupied.contains(QPoint(unitNum / 2, unitNum - 3)))
        {
            brickGroup.removeOne(unitOccupied[QPoint(unitNum / 2, unitNum - 3)]);
        }
        ironGroup.append(new Obstacle((unitNum / 2 - 1) * unitSize, (unitNum - 3) * unitSize, Iron, unitSize));
        unitOccupied[QPoint(unitNum / 2 - 1, unitNum - 3)] = ironGroup.last();
        ironGroup.append(new Obstacle((unitNum / 2) * unitSize, (unitNum - 3) * unitSize, Iron, unitSize));
        unitOccupied[QPoint(unitNum / 2, unitNum - 3)] = ironGroup.last();

        fixTimer = new QTimer(this);
        fixTimer->start(5000);
        connect(fixTimer, &QTimer::timeout, this, [=](){
            fixTimer->stop();
            for (int j = unitNum - 3; j < unitNum; j++)
            {
                ironGroup.removeOne(unitOccupied[QPoint(unitNum / 2 - 2, j)]);
                ironGroup.removeOne(unitOccupied[QPoint(unitNum / 2 + 1, j)]);
                brickGroup.append(new Obstacle((unitNum / 2 - 2) * unitSize, j * unitSize, Brick, unitSize));
                unitOccupied[QPoint(unitNum / 2 - 2,j)] = brickGroup.last();
                brickGroup.append(new Obstacle((unitNum / 2 + 1) * unitSize, j * unitSize, Brick, unitSize));
                unitOccupied[QPoint(unitNum / 2 + 1,j)] = brickGroup.last();
            }
            ironGroup.removeOne(unitOccupied[QPoint(unitNum / 2 - 1, unitNum - 3)]);
            ironGroup.removeOne(unitOccupied[QPoint(unitNum / 2, unitNum - 3)]);
            brickGroup.append(new Obstacle((unitNum / 2 - 1) * unitSize, (unitNum - 3) * unitSize, Brick, unitSize));
            unitOccupied[QPoint(unitNum / 2 - 1, unitNum - 3)] = brickGroup.last();
            brickGroup.append(new Obstacle((unitNum / 2) * unitSize, (unitNum - 3) * unitSize, Brick, unitSize));
            unitOccupied[QPoint(unitNum / 2, unitNum - 3)] = brickGroup.last();

            this->update();
        });
        break;
    case Heart:
        information.playerlife++; // 玩家生命+1
        informationui->setPlayerLife(information.playerlife);
        break;
    case Star:
        playerTank->upgrade(); // 玩家升级
        informationui->setLevel(playerTank->getLevel());
        informationui->setHealth(playerTank->getHealth());
        break;
    case Shell:
        playerTank->setShellInf(true); // 玩家子弹无限
        informationui->setShell(-1);
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
            informationui->setShell(2);
        });
        break;
    case Laser:
        break;
    default:
        break;
    }
}
