#include "informationui.h"
#include "ui_informationui.h"
#include <qevent.h>

InformationUI::InformationUI(QWidget *parent, int enemyleft, int playerlife, int shell)
    : QWidget(parent)
    , ui(new Ui::InformationUI)
{
    ui->setupUi(this);

    init_enemyleft = enemyleft;
    init_playerlife = playerlife;
    init_shell = shell;

    ui->startButton->clearFocus(); // 清除按钮焦点
}

InformationUI::~InformationUI()
{
    delete ui;
}

QString InformationUI::getWhiteString(QString str)
{
    return "<span style='color: white;'>" + str + "</span>";
}

void InformationUI::setLevel(int level)
{
    ui->levelLabel->setText(getWhiteString(QString::number(level)));
}

void InformationUI::setWin(bool ifwin)
{
    total++;
    if (ifwin)
    {
        win++;
    }
    ui->winLabel->setText(getWhiteString(QString::number(win) + "/" + QString::number(total)));
}

void InformationUI::setHealth(int health)
{
    if (health == -1)
    {
        ui->healthLabel->setText(getWhiteString("inf"));
    }
    else
    {
        ui->healthLabel->setText(getWhiteString(QString::number(health)));
    }
}

void InformationUI::setEnemyLeft(int enemyleft)
{
    ui->enemyLabel->setText(getWhiteString(QString::number(enemyleft)));
}

void InformationUI::setPlayerLife(int playerlife)
{
    ui->playerLifeLabel->setText(getWhiteString(QString::number(playerlife)));
}

void InformationUI::setShell(int shell)
{
    if (shell == -1)
    {
        ui->shellLabel->setText(getWhiteString("inf"));
    }
    else
    {
        ui->shellLabel->setText(getWhiteString(QString::number(shell)));
    }
}

void InformationUI::setTime(int min, int second, int millisecond)
{
    QString millisecondstr = QString::number(millisecond);
    if (millisecondstr.size() == 1)
    {
        millisecondstr.push_front('0');
    }
    QString secondstr = QString::number(second);
    if (secondstr.size() == 1)
    {
        secondstr.push_front('0');
    }
    ui->timeLabbel->setText(getWhiteString(QString::number(min) + ":" + secondstr + ":" + millisecondstr));
}

void InformationUI::reset()
{
    ui->winLabel->setText(getWhiteString(QString::number(win) + "/" + QString::number(total)));
    ui->levelLabel->setText(getWhiteString(QString::number(1)));
    ui->healthLabel->setText(getWhiteString("inf"));
    ui->enemyLabel->setText(getWhiteString(QString::number(init_enemyleft)));
    ui->playerLifeLabel->setText(getWhiteString(QString::number(init_playerlife)));
    if (init_shell == -1)
    {
        ui->shellLabel->setText(getWhiteString("inf"));
    }
    else
    {
        ui->shellLabel->setText(getWhiteString(QString::number(init_shell)));
    }
    ui->timeLabbel->setText(getWhiteString("00:00:00"));
}

void InformationUI::on_startButton_clicked()
{
    emit resetPressed();
    reset(); // 重置信息
}

