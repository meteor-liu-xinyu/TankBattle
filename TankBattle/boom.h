#ifndef BOOM_H
#define BOOM_H

#include <QDialog>
#include <QPixmap>
#include <QRect>
#include <QString>
#include <QPainter>
#include <QTimer>
#include <QDebug>

namespace Ui {
class Boom;
}

class Boom : public QDialog
{
    Q_OBJECT

public:
    explicit Boom(QRect rect, QWidget *parent = 0);

    ~Boom();
    void draw(QPainter &painter)
    {
        painter.drawPixmap(rect, entityPixmap);
    }

signals:
   void boomFinished();

private slots:
    void destroyBoom();

private:
    Ui::Boom *ui;

    QString picpath;
    QRect rect;
    int tick;
    QTimer *timer;
    QPixmap entityPixmap;
};

#endif // BOOM_H
