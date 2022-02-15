#include "mainforn.h"
#include "ui_mainforn.h"

mainforn::mainforn(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::mainforn)
{
    ui->setupUi(this);

    g_recordManage = new recordManage(this);
    g_recordManage->setGeometry(0,60,g_recordManage->width(),g_recordManage->height());

    g_sysManage = new sysManage(this);
    g_sysManage->setGeometry(0,60,g_sysManage->width(),g_sysManage->height());

    g_recordManage->hide();
    g_sysManage->hide();

    connect(ui->sysyPushButton,SIGNAL(clicked()),this,SLOT(menuButtonClick()));
    connect(ui->recpushButton,SIGNAL(clicked()),this,SLOT(menuButtonClick()));


    ui->sysyPushButton->setChecked(true);
    ui->recpushButton->setChecked(false);


}

mainforn::~mainforn()
{
    delete ui;
}

void mainforn::showMainfornPage()
{
    this->show();
}

void mainforn::menuButtonClick()
{
    QObject* Sender = sender();     //Sender->objectName(),可区分不同的信号来源，也就是不同的按钮按键


    if(Sender==0)
    {
        return ;
    }
    if (Sender->objectName() == "sysyPushButton")     //受电弓监控按钮被按，则切换到受电弓监控页面
    {
        g_sysManage->hide();
        g_recordManage->show();

    }
    else
    {
        g_sysManage->show();
        g_recordManage->hide();
    }

}

