#include "obstacle.h"

Obstacle::Obstacle(int x, int y, ObstacleType obstacletype, int unitSize): Entity(unitSize)
{
    this->obstacletype = obstacletype;
    picPath = ":/obstacle/images/";
    int width = unitSize;
    int height = unitSize;
    switch (obstacletype)
    {
    case Brick:
        picPath += "brick.png";
        tag = "brick";
        break;
    case Iron:
        picPath += "iron.png";
        tag = "iron";
        break;
    case River:
        picPath += "river.png";
        tag = "river";
        break;
    case Tree:
        picPath += "tree.png";
        tag = "tree";
        break;
    case Home:
        picPath += "home.png";
        width = unitSize * 2;
        height = unitSize * 2;
        tag = "home";
        break;
    default:
        break;
    }
    rect = QRect(x, y, width, height);
    setPixmap(picPath);
}

void Obstacle::homeDestroyed() // 设置基地被摧毁
{
    if (obstacletype != Home)
    {
        return;
    }
    setPixmap(":/obstacle/images/home_destroyed.png");
}
