#include "obstacle.h"

Obstacle::Obstacle(int x, int y, ObstacleType obstacletype)
{
    this->obstacletype = obstacletype;
    picpath = ":/obstacle/images/";
    int width = 24;
    int height = 24;
    switch (obstacletype)
    {
    case Brick:
        picpath += "brick.png";
        tag = "brick";
        break;
    case Iron:
        picpath += "iron.png";
        tag = "iron";
        break;
    case River:
        picpath += "river.png";
        tag = "river";
        break;
    case Tree:
        picpath += "tree.png";
        tag = "tree";
        break;
    case Home:
        picpath += "home.png";
        width = 48;
        height = 48;
        tag = "home";
        break;
    default:
        break;
    }
    rect = QRect(x, y, width, height);
    entityPixmap = QPixmap(picpath);
}

void Obstacle::homeDestroyed() // 设置基地被摧毁
{
    if (obstacletype != Home)
    {
        return;
    }
    entityPixmap = QPixmap(":/obstacle/images/home_destroyed.png");
}

Obstacle::~Obstacle()
{
    rect = QRect(0, 0, 0, 0);
}
