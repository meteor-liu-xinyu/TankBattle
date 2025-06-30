#ifndef ENTITY_H
#define ENTITY_H

#include <QRect>
#include <QPixmap>
#include <QPainter>
#include <QDebug>

enum Direction {
    Up,
    Down,
    Left,
    Right,
    Stop
};

class Entity
{
public:
    Entity(int unitSize)
    {
        this->unitSize = unitSize;
    }
    ~Entity()
    {
        rect = QRect(0,0,0,0);
    }
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
        if (!picPath.isEmpty())
        {
            entityPixmap = QPixmap(picPath).scaled(rect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        }
    }
    void setPixmap(const QString& picpath)
    {
        this->picPath = picpath;
        entityPixmap = QPixmap(picPath).scaled(rect.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }
    void setUnitSize(int size)
    {
        if (!rect.isEmpty())
        {
            rect = QRect(rect.x() * size / unitSize, rect.y() * size / unitSize, rect.width() * size / unitSize, rect.height() * size / unitSize);
            entityPixmap = QPixmap(picPath).scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

        this->unitSize = size;
    }

protected:
    QRect rect;
    QPixmap entityPixmap; // 实体图片
    QString picRootPath; // 图片路径
    QString picPath; // 图片路径
    QString tag;

    int unitSize = 24;
};

#endif // ENTITY_H
