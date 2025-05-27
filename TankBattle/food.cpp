#include "food.h"

Food::Food(int x, int y, int foodtype)
{
    QString picpath = ":/food/images/";
    switch (foodtype)
    {
    case 0:
        this->foodtype = Fix;
        picpath += "fix.png";
        tag = "fix";
        break;
    case 1:
        picpath += "heart.png";
        tag = "heart";
        this->foodtype = Heart;
        break;
    case 2:
        picpath += "shell.png";
        tag = "shell";
        this->foodtype = Shell;
        break;
    case 3:
        picpath += "star.png";
        tag = "star";
        this->foodtype = Star;
        break;
    case 4:
        picpath += "laser.png";
        tag = "laser";
        this->foodtype = Laser;
        break;

    default:
        break;
    }
    rect = QRect(x + 4, y + 4, 40, 40); // 设置食物矩形区域, 修正到中心
    entityPixmap = QPixmap(picpath);
}

Food::~Food()
{
    rect = QRect(0, 0, 0, 0);
}

FoodType Food::getFoodType()
{
    return foodtype;
}
