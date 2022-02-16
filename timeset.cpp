#include "timeset.h"
#include "ui_timeset.h"
#include <QDateTime>
#include <stdio.h>
#include <QDebug>

int idayNum[12]={31,28,31,30,31,30,31,31,30,31,30,31};

timeset::timeset(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::timeset)
{
    ui->setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint);

    connect(ui->pushButton_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_1_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_2_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_3, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_3_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_4, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_4_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_5, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_5_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_6, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_6_1, SIGNAL(clicked()), this, SLOT(stButtonClick()));
    connect(ui->pushButton_7, SIGNAL(clicked()), this, SLOT(okButtonClick()));
    connect(ui->pushButton_8, SIGNAL(clicked()), this, SLOT(cancleButtonClick()));


}

timeset::~timeset()
{
    delete ui;
}



void timeset::cancleButtonClick()
{
    this->hide();
    emit cancleMsg();
}

void timeset::okButtonClick()
{
    this->hide();
    emit timeSetSendMsg(ui->label_2->text(), ui->label_3->text(), ui->label_4->text(), ui->label_5->text(), ui->label_6->text(), ui->label_7->text());
}

void timeset::stButtonClick()
{
    //printf("timeSet::stButtonClick (%d,%d),(%d,%d)\n",cursor().pos().x(),cursor().pos().y(),this->pos().x(),this->pos().y());
    int num = 0, mon = 0;
    QString string = "";
    QPoint pt=cursor().pos()-((QWidget *)(this->parent()))->pos()-this->pos();
    QObject* Object=sender();
    if (Object == ui->pushButton_1 || Object == ui->pushButton_1_1)
    {
        num = ui->label_2->text().toInt();

        if(Object == ui->pushButton_1)
        {
            num +=1;
        }
        else
        {
            num -=1;
        }
        if ((num%4==0&&num%100!=0)||(num%400==0))
        {
            idayNum[1] = 29;
        }
        else
        {
            idayNum[1] = 28;
        }
        ui->label_2->setText(QString::number(num));
    }
    if (Object == ui->pushButton_2 || Object ==ui->pushButton_2_1)
    {
        string = "";
        num = ui->label_3->text().toInt();

        if(Object == ui->pushButton_2)
        {
            num +=1;
            if (num > 12)
            {
                num = 1;
            }
        }
        else
        {
            num -=1;
            if (num < 1)
            {
                num = 12;
            }
        }

        if (num < 10)
        {
            string += "0";
        }

        string += QString::number(num);
        ui->label_3->setText(string);
    }
    if (Object == ui->pushButton_3 || Object == ui->pushButton_3_1)
    {
        string = "";
        mon = ui->label_3->text().toInt();
        num = ui->label_4->text().toInt();

        if(Object == ui->pushButton_3)
        {
            num +=1;
            if (num > idayNum[mon-1])
            {
                num = 1;
            }
        }
        else
        {
            num -=1;
            if (num < 1)
            {
                num = idayNum[mon-1];
            }
        }

        if (num < 10)
        {
            string += "0";
        }
        string += QString::number(num);
        ui->label_4->setText(string);
    }
    if (Object == ui->pushButton_4 || Object == ui->pushButton_4_1)
    {
        string = "";
        num = ui->label_5->text().toInt();

        if(Object == ui->pushButton_4)
        {
            num +=1;
            if (num > 23)
            {
                num = 0;
            }
        }
        else
        {
            num -=1;
            if (num < 0)
            {
                num = 23;
            }
        }

        if (num < 10)
        {
            string += "0";
        }
        string += QString::number(num);
        ui->label_5->setText(string);
    }
    if (Object == ui->pushButton_5 || Object == ui->pushButton_5_1)
    {
        string = "";
        num = ui->label_6->text().toInt();

        if(Object == ui->pushButton_5)
        {
            num +=1;
            if (num > 59)
            {
                num = 0;
            }
        }
        else
        {
            num -=1;
            if (num < 0)
            {
                num = 59;
            }
        }

        if (num < 10)
        {
            string += "0";
        }
        string += QString::number(num);
        ui->label_6->setText(string);
    }
    if (Object == ui->pushButton_6 || Object == ui->pushButton_6_1)
    {
        string = "";
        num = ui->label_7->text().toInt();

        if(Object == ui->pushButton_6)
        {
            num +=1;
            if (num > 59)
            {
                num = 0;
            }
        }
        else
        {
            num -=1;
            if (num < 0)
            {
                num = 59;
            }
        }


        if (num < 10)
        {
            string += "0";
        }
        string += QString::number(num);
        ui->label_7->setText(string);
    }
}

void timeset::setTimeLabelText(int year, int month, int day, int hour, int min, int sec)
{
    QString string = "";
    ui->label_2->setText(QString::number(year));
    if (month < 10)
    {
        string = "0";
    }
    else
    {
        string = "";
    }
    string += QString::number(month);
    ui->label_3->setText(string);
    if (day < 10)
    {
        string = "0";
    }
    else
    {
        string = "";
    }
    string += QString::number(day);
    ui->label_4->setText(string);
    if (hour < 10)
    {
        string = "0";
    }
    else
    {
        string = "";
    }
    string += QString::number(hour);
    ui->label_5->setText(string);
    if (min < 10)
    {
        string = "0";
    }
    else
    {
        string = "";
    }
    string += QString::number(min);
    ui->label_6->setText(string);
    if (sec < 10)
    {
        string = "0";
    }
    else
    {
        string = "";
    }
    string += QString::number(sec);
    ui->label_7->setText(string);
}
