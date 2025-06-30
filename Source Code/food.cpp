#include "food.h"

Food::Food(int x, int y, int foodtype, int unitSize): Entity(unitSize)
{
    picPath = ":/food/images/";
    switch (foodtype)
    {
    case 0:
        this->foodtype = Fix;
        picPath += "fix.png";
        tag = "fix";
        break;
    case 1:
        picPath += "heart.png";
        tag = "heart";
        this->foodtype = Heart;
        break;
    case 2:
        picPath += "shell.png";
        tag = "shell";
        this->foodtype = Shell;
        break;
    case 3:
        picPath += "star.png";
        tag = "star";
        this->foodtype = Star;
        break;
    case 4:
        picPath += "laser.png";
        tag = "laser";
        this->foodtype = Laser;
        break;

    default:
        break;
    }
    rect = QRect(x + unitSize / 6, y + unitSize / 6, unitSize * 5 / 3, unitSize * 5 / 3); // 设置食物矩形区域, 修正到中心
    setPixmap(picPath);
}

FoodType Food::getFoodType()
{
    return foodtype;
}
