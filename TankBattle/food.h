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
    Laser
};

class Food : public Entity
{
public:
    Food(int x, int y, int foodtype);
    FoodType foodtype;
    ~Food();
    FoodType getFoodType();
};

#endif // FOOD_H
