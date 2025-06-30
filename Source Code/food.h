#ifndef FOOD_H
#define FOOD_H

#include <QPixmap>
#include <QRect>
#include <QString>
#include <QPainter>

#include "entity.h"

enum FoodType {
    Fix,
    Heart,
    Shell,
    Star,
    Laser // 暂未制作
};

class Food : public Entity
{
public:
    Food(int x, int y, int foodtype, int unitSize);
    FoodType getFoodType(); // 获取道具类型

private:
    FoodType foodtype; // 道具类型
};

#endif // FOOD_H
