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
    QRect getRect()
    {
        return rect;
    }
    void draw(QPainter &painter)
    {
        painter.drawPixmap(rect, entityPixmap);
    }
    QPoint getPos()
    {
        return QPoint(rect.x(), rect.y());
    }
    QString getTag()
    {
        return tag;
    }
    void setRect(const QRect &rect)
    {
        this->rect = rect;
    }

protected:
    QRect rect;
    QTimer *timer;
    int tick;
    QPixmap entityPixmap;
    QString tag;
};

#endif // ENTITY_H
