#ifndef GAMEOVERUI_H
#define GAMEOVERUI_H

#include <QWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsOpacityEffect>

namespace Ui {
class GameoverUI;
}

class GameoverUI : public QWidget
{
    Q_OBJECT

public:
    explicit GameoverUI(QWidget *parent = 0, bool ifwin = false);
    void keyPressEvent(QKeyEvent *k) override; // 处理键盘输入
    ~GameoverUI();

signals:
    void restart();

private:
    Ui::GameoverUI *ui;
};

#endif // GAMEOVERUI_H
