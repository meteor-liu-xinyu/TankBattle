#ifndef BOOM_H
#define BOOM_H

#include <QObject>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QPainter>
#include <QTimer>
#include <QDebug>

#include "entity.h"

class Boom : public QObject, public Entity
{
    Q_OBJECT
public:
    explicit Boom(QRect rect, int unitSize, QObject *parent = nullptr);

signals:
    void boomFinished();

private slots:
    void destroyBoom();

private:
    int tick;
    QTimer *timer;
};

#endif // BOOM_H
