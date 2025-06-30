#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <QPixmap>
#include <QRect>
#include <QString>
#include <QPainter>

#include "entity.h"

enum ObstacleType {
    Brick,
    Iron,
    River,
    Tree,
    Home
};


class Obstacle : public Entity
{
public:
    Obstacle(int x, int y, ObstacleType obstacletype, int unitSize);
    void homeDestroyed(); // 设置基地被摧毁

private:
    ObstacleType obstacletype; // 障碍物类型
};

#endif // OBSTACLE_H
