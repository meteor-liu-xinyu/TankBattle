#include "mainwidget.h"
#include "ui_mainwidget.h"

#define UNIT 24
#define TANKSPEED 120 / tickrate
#define BULLETSPEED 480 / tickrate
#define INITPLAYERLIFE 2
#define INITENEMYNUM 10

MainWidget::MainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    this->setWindowTitle("坦克大战");
    ui->startButton->clearFocus();

    QPixmap cover(":/background/images/cover.png");
    QPalette palette = this->palette();
    palette.setBrush(QPalette::Window, QBrush(cover));
    // 应用 QPalette 到窗口
    this->setPalette(palette);

    information.playerlife = INITPLAYERLIFE;
    information.enemyleft = INITENEMYNUM;

    stopwidget = new StopWidget(this);
    stopwidget->hide();
}

void MainWidget::on_startButton_clicked()
{
    disconnect(stopwidget, &StopWidget::restart, nullptr, nullptr);
    if (!gameend || timer || gameTimer || foodTimer)
    {
        gameend = true;
        gameEnded();
    }

    ui->startButton->clearFocus();

    information.total++;

    initGame();
}

void MainWidget::initGame()
{
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

    // 初始化随机种子
    qsrand(QTime::currentTime().msec());

    timer = new QTimer(this);
    timer->start(1000 / tickrate); // 设置帧率
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

    gameTimer = new QTimer(this);
    gameTimer->start(10);
    min = 0;
    second = 0;
    msecond = 0;
    connect(gameTimer, &QTimer::timeout, this, [=](){
        msecond++;
        if (msecond == 100)
        {
            msecond = 0;
            second++;
        }
        if (second == 60)
        {
            second = 0;
            min++;
        }
        QString msecondstr = QString::number(msecond);
        if (msecondstr.size() == 1)
        {
            msecondstr.push_front('0');
        }
        QString secondstr = QString::number(second);
        if (secondstr.size() == 1)
        {
            secondstr.push_front('0');
        }
        ui->timeLabbel->setText(getWhiteString(QString::number(min) + ":" + secondstr + ":" + msecondstr));
    });

    generateGameMap();
    generatePlayerTank();
    for (int i = 0; i < 1; i++)
    {
        generateEnemyTank();
    }
    generateFood();

    if (stopwidget)
    {
        stopwidget->close();
        stopwidget = NULL;
    }
    if (gameoverui)
    {
        gameoverui->close();
        gameoverui = NULL;
    }

    information.enemyleft = INITENEMYNUM;
    information.playerlife = INITPLAYERLIFE;

    ui->levelLabel->setText(getWhiteString(QString::number(playerTank->getLevel())));
    ui->healthLabel->setText(getWhiteString("inf"));
    ui->enemyLabel->setText(getWhiteString(QString::number(information.enemyleft)));
    ui->playerLifeLabel->setText(getWhiteString(QString::number(information.playerlife)));
    ui->shellLabel->setText(getWhiteString(QString::number(playerTank->getRemainBulletNum())));
    ui->winLabel->setText(getWhiteString(QString::number(information.win) + "/" + QString::number(information.total)));
}

void MainWidget::updateGame()
{
    if (movingDirection.size() != 0 && !gameend && playerTank)
    {
        playerTank->changeDirection(movingDirection.last());
        if (!checkTankCollision(movingDirection.last()))
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

    connect(this, &MainWidget::playerTankDestroyed, this, &MainWidget::dealplayerTankDestroyed);
    connect(this, &MainWidget::enemyTankDestroyed, this, &MainWidget::dealenemyTankDestroyed);
    connect(this, &MainWidget::pickFood, this, &MainWidget::dealpickFood);

    for (auto boom : booms)
    {
        connect(boom, &Boom::boomFinished, this, [=](){
            booms.removeOne(boom);
        });
    }

    if (playerTank)
    {
        for (auto river : riverGroup)
        {
            if (river->getRect().intersects(playerTank->getRect()))
            {
                emit dealplayerTankDestroyed();
                break;
            }
        }
    }

    if (bullets.size() != 0 && !gameend)
    {
        for (int i = 0; i < bullets.size(); i++)
        {
            if (checkBulletCollision(bullets[i],bullets[i]->getDirection()))
            {
                if (bullets[i]->getTag() == "-1")
                {
                    if (playerTank)
                    {
                        if (playerTank->getRemainBulletNum() < 2)
                        {
                            playerTank->removeBullet(bullets[i]);
                            if (playerTank->getRemainBulletNum() == -1)
                            {
                                ui->shellLabel->setText(getWhiteString("inf"));
                            }
                            else
                            {
                                ui->shellLabel->setText(getWhiteString(QString::number(playerTank->getRemainBulletNum())));
                            }
                        }
                    }
                }
                else
                {
                    if (bullets[i]->getOwner())
                    {
                        if (bullets[i]->getOwner()->getRemainBulletNum() < 1)
                        {
                            bullets[i]->getOwner()->removeBullet(bullets[i]);
                        }
                    }
                }
                bullets.removeAt(i);
                i--;
                continue;
            }
            bullets[i]->move();
        }
    }

    if (enemyTank.size() != 0 && !gameend)
    {
        for (auto enemy : enemyTank)
        {
            if (enemy->getLevel() != 2)
            {
                enemy->setTargetPos(QPoint(playerTank->getPos().x()/UNIT, playerTank->getPos().y()/UNIT));
            }

            enemyMove(enemy);

            for (auto bullet : bullets)
            {
                if (ifInShootRect(enemy, bullet) && bullet->getOwner() != enemy)
                {
                    avoidBullet(bullet, enemy->getNextPos(), enemy);
                }
            }
            Direction nextdir = moveToUnit(enemy);
            ifShootPlayer(enemy);
            ifShootHome(enemy);
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



    if (homeDestroyed && home)
    {
        home->homeDestroyed();
    }

    if (gameend)
    {
        movingDirection.clear();

        if (!gameoverui)
        {
            gameoverui = new GameoverUI(this, win);
            gameoverui->show();
            connect(gameoverui, &GameoverUI::restart, this, &MainWidget::on_startButton_clicked);
        }

        if (booms.empty())
        {
            gameEnded();
        }
    }

    this->update();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::gameEnded()
{
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
        information.win++;
    }
}

bool MainWidget::checkTankCollision(const Direction &dir) // 检测碰撞,返回true表示碰撞
{
    if (!playerTank)
    {
        return false;
    }
    QRect nextrect = playerTank->getRect();
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
    for (auto enemy : enemyTank)
    {
        if (nextrect.intersects(enemy->getRect()))
        {
            return true; // 碰撞
        }
    }

    for (auto food : foods)
    {
        if (nextrect.intersects(food->getRect()))
        {
            emit pickFood(food->getFoodType());
            qDebug() << "food picked";
            foods.removeOne(food);
        }
    }

    if (nextrect.intersects(home->getRect()))
    {
        return true;
    }

    return false;
}

bool MainWidget::checkBulletCollision(Bullet *bullet, const Direction &dir)
{
    QRect nextrect = bullet->getRect();
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
        return true;
    }

    bool flag = false;
    for (int i = nextrect.x() / UNIT - 2; i <= nextrect.x() / UNIT + 2; i++)
    {
        for (int j = nextrect.y() / UNIT - 2; j <= nextrect.y() / UNIT + 2; j++)
        {
            if (unitOccupied.contains(QPoint(i, j)))
            {
                if (nextrect.intersects(unitOccupied[QPoint(i,j)]->getRect()))
                {
                    if (unitOccupied[QPoint(i,j)]->getTag() == "brick")
                    {
                        booms.append(new Boom(unitOccupied[QPoint(i,j)]->getRect()));
                        brickGroup.removeOne(unitOccupied[QPoint(i,j)]);
                        unitOccupied.remove(QPoint(i, j));
                        flag = true;
                    }
                    else if (unitOccupied[QPoint(i,j)]->getTag() == "iron")
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

    if (nextrect.intersects(home->getRect()))
    {
        booms.append(new Boom(home->getRect()));
        gameend = true; // 游戏结束
        homeDestroyed = true;
        win = false;
        return true;
    }

    if (bullet->getTag() == "-1")
    {
        for (int i = 0; i < enemyTank.size(); i++)
        {
            if (nextrect.intersects(enemyTank[i]->getRect()))
            {
                qDebug() << "hit enemy tank";
                emit enemyTankDestroyed(i);
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
            emit playerTankDestroyed();
            booms.append(new Boom(bullet->getRect()));
            return true;
        }
    }

    return false;
}


void MainWidget::keyPressEvent(QKeyEvent *k)
{
    if (k->key() == Qt::Key_R)
    {
        on_startButton_clicked();
    }
    else if (k->key() == Qt::Key_Space && !gameend && playerTank)
    {
        shoot(playerTank);
    }
    else if (k->key() == Qt::Key_Escape)
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
                if (!stopwidget)
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

void MainWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    for (auto brick : brickGroup)
    {
        brick->draw(painter);
    }
    for (auto iron : ironGroup)
    {
        iron->draw(painter);
    }
    for (auto river : riverGroup)
    {
        river->draw(painter);
    }
    for (auto tree : treeGroup)
    {
        tree->draw(painter);
    }
    for (auto bullet : bullets)
    {
        bullet->draw(painter);
    }
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
}

void MainWidget::generatePlayerTank()
{
    // 生成玩家坦克
    playerTank = new Tank(29 * UNIT, 54 * UNIT, PlayerTank, 1, TANKSPEED);
    playerTank->setUnbeatable(true);
    ui->healthLabel->setText(getWhiteString("inf"));
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
    // 随机生成道具位置
    if (x == -1 &&y == -1)
    {
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

    /*
    // 生成河水
    #define riverGroups 4
    for (int i = 0; i < riverGroups; )
    {
        int x = (qrand() % 28 + 1) * 2;
        int y = (qrand() % 24 + 1) * 2;

        if (!unitOccupied.contains(QPoint(x, y)))
        {
            generateRiverGroup(x, y);
            i++;
        }
    }
    */

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

void MainWidget::dealplayerTankDestroyed()
{
    disconnect(this, &MainWidget::playerTankDestroyed, this, &MainWidget::dealplayerTankDestroyed);

    if (playerTank == NULL)
    {
        qDebug() << "playerTank is NULL";
    }

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
            information.playerlife--;
            generatePlayerTank();
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

void MainWidget::dealenemyTankDestroyed(int i)
{
    disconnect(this, &MainWidget::enemyTankDestroyed, this, &MainWidget::dealenemyTankDestroyed);

    if (enemyTank[i]->getHealth() == 1)
    {
        information.enemyleft--;
        booms.append(new Boom(enemyTank[i]->getRect()));
        if (enemyTank[i]->getLevel() == 4)
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
            generateEnemyTank();
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

QPoint MainWidget::findNextDirection(Tank *tank, QPoint start, QPoint target)
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

    tank->setDistance(distance[target]);

    // 回溯路径

    QPoint p = target;
    QPoint q;
    while (p != QPoint(-1, -1))
    {
        q = p;
        p = prePoint[p];
        if (p == start)
        {
            return q;
        }
    }

    return QPoint(-1, -1);
}

void MainWidget::enemyMove(Tank *enemy)
{
    Direction nextdir = moveToUnit(enemy);
    if (nextdir == Stop)
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
                enemy->setTempTargetPos(QPoint(playerTank->getPos().x()/UNIT, playerTank->getPos().y()/UNIT));
            }
            if (enemy->getLevel() == 2)
            {
                int possibility = qrand() % 10;
                if (possibility < 6 && manhattanDistance(enemyPos, enemy->getTargetPos()) < 50)
                {
                    enemy->setTempTargetPos(enemy->getTargetPos());
                }
            }
        }

        temppoint = findNextDirection(enemy, enemyPos, enemy->getTempTargetPos());

        if (temppoint == QPoint(-1,-1))
        {
            return;
        }

        QRect temprect(temppoint.x() * UNIT, temppoint.y() * UNIT, 40, 40);
        if (temprect.intersects(playerTank->getRect()))
        {
            return;
        }

        enemy->setNextPos(QPoint(temppoint.x(), temppoint.y()));
    }
}

void MainWidget::avoidBullet(Bullet *bullet, QPoint tankPos, Tank *enemy)
{
    int distance = 120;
    QPoint hidePoint(-1,-1);
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
        enemy->setNextPos(hidePoint);
    }
}

void MainWidget::ifShootPlayer(Tank *enemy)
{
    if (enemy->getRemainBulletNum() == 0)
    {
        return;
    }
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


    while (true)
    {
        if (unitOccupied.contains(QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)) && unitOccupied[QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)]->getTag() != "river")
        {
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


    while (true)
    {
        if (unitOccupied.contains(QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)) && unitOccupied[QPoint(enemyPos.x() / UNIT, enemyPos.y() / UNIT)]->getTag() != "river")
        {
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
        playerTank->setUnbeatable(false);
        ui->healthLabel->setText(getWhiteString(QString::number(playerTank->getHealth())));

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
        information.playerlife++;
        ui->playerLifeLabel->setText(getWhiteString(QString::number(information.playerlife)));
        break;
    case Star:
        playerTank->upgrade();
        ui->levelLabel->setText(getWhiteString(QString::number(playerTank->getLevel())));
        ui->healthLabel->setText(getWhiteString(QString::number(playerTank->getHealth())));
        break;
    case Shell:
        playerTank->setShellInf(true);
        ui->shellLabel->setText(getWhiteString("inf"));
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
        shellInfTimer->start(15000);
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
