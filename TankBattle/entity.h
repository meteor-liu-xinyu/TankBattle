#ifndef ENTITY_H
#define ENTITY_H

#include <QRect>
#include <QPixmap>
#include <QPainter>
#include <QObject>
#include <QTimer>
#include <QDebug>

enum Direction {
    Up,
    Down,
    Left,
    Right,
    Stop
};

class Entity : public QObject
{
public:
    Entity(){}
    QRect getRect() // 获取实体矩形区域
    {
        return rect;
    }
    void draw(QPainter &painter) // 绘制实体
    {
        painter.drawPixmap(rect, entityPixmap);
    }
    QPoint getPos() // 获取实体位置
    {
        return QPoint(rect.x(), rect.y());
    }
    QString getTag() // 获取实体标签
    {
        return tag;
    }
    void setRect(const QRect &rect) // 设置实体矩形区域
    {
        this->rect = rect;
    }

protected:
    QRect rect;
    QTimer *timer;
    int tick;
    QPixmap entityPixmap; // 实体图片
    QString tag;
};

#endif // ENTITY_H
