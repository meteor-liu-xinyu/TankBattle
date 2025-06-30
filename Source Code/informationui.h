#ifndef INFORMATIONUI_H
#define INFORMATIONUI_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>

namespace Ui {
class InformationUI;
}

class InformationUI : public QWidget
{
    Q_OBJECT

public:
    explicit InformationUI(QWidget *parent = nullptr, int enemyleft = 10, int playerlife = 2, int shell = 2);
    ~InformationUI();
    void setLevel(int level);
    void setWin(bool ifwin);
    void setHealth(int health);
    void setEnemyLeft(int enemyleft);
    void setPlayerLife(int playerlife);
    void setShell(int shell);
    void setTime(int min, int second, int millisecond);
    void reset();

signals:
    void resetPressed();


private slots:
    void on_startButton_clicked();

private:
    Ui::InformationUI *ui;

    QString getWhiteString(QString str); // 获取白色html字符串

    int win = 0;
    int total = 1;

    int init_enemyleft = 10; // 初始剩余敌人数量
    int init_playerlife = 2; // 初始玩家生命数
    int init_shell = 2; // 初始子弹数量
};

#endif // INFORMATIONUI_H
