#ifndef STOPWIDGET_H
#define STOPWIDGET_H

#include <QWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QGraphicsOpacityEffect>

namespace Ui {
class StopWidget;
}

class StopWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StopWidget(QWidget *parent = 0);
    void keyPressEvent(QKeyEvent *k) override; // 处理键盘输入
    // void resizeEvent(QResizeEvent *event) override; // 处理窗口大小变化
    ~StopWidget();

signals:
    void restart();
    void continuesgn();

private slots:
    void on_continueButton_clicked();

    void on_restartButton_clicked();

private:
    Ui::StopWidget *ui;
};

#endif // STOPWIDGET_H
