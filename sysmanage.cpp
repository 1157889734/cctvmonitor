#include "sysmanage.h"
#include "ui_sysmanage.h"
#include <QAbstractItemView>
#include <QTableWidget>
#include <QDebug>
#include "comm.h"

sysManage::sysManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sysManage)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);
    struct tm *pLocalTime;
    time_t tTime;
    time(&tTime);
    pLocalTime = localtime(&tTime);
    m_iYear = pLocalTime->tm_year +1900;
    m_iMonth = pLocalTime->tm_mon +1;
    m_iDay = pLocalTime->tm_mday;

    ui->yearLabel->setText(QString::number(m_iYear,10));
    ui->monthLabel->setText(QString::number(m_iMonth,10));
    ui->dayLabel->setText(QString::number(m_iDay,10));


    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QPixmap(":/res/bg_system.png")));
    this->setPalette(palette);
//    this->setStyleSheet("QWidget{background-image: url(:/res/bg_system.png)}");

    connect(ui->backButton,SIGNAL(clicked()),this,SLOT(hideSysPageSlots()));

    ui->devStatusTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->devStatusTableWidget->setColumnCount(6);
    ui->devStatusTableWidget->setRowCount(6);
    ui->devStatusTableWidget->setShowGrid(true);
    ui->devStatusTableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(252, 233, 79) }");

//    ui->devStorageTableWidget->setStyleSheet("QTableWidget{ gridline-color : rgb(255, 255, 255)}");
    QStringList header;
    header<<tr("设备位置")<<tr("硬盘个数")<<tr("硬盘容量")<<tr("使用量")<<tr("硬盘状态")<<tr("在线状态");
    ui->devStatusTableWidget->horizontalHeader()->setStyleSheet("background-color:white");
    ui->devStatusTableWidget->setHorizontalHeaderLabels(header);
    ui->devStatusTableWidget->horizontalHeader()->setVisible(true);//temp
    ui->devStatusTableWidget->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击
    ui->devStatusTableWidget->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    ui->devStatusTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->devStatusTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    ui->devStatusTableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中

    ui->devStatusTableWidget->horizontalHeader()->resizeSection(1,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(2,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(3,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(4,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(5,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(6,70);


    ui->devLogTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->devLogTableWidget->setColumnCount(4);
//    ui->devLogTableWidget->setRowCount(6);
    ui->devLogTableWidget->setShowGrid(true);
    ui->devLogTableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(252, 233, 79) }");

    QStringList logheader;
    logheader<<tr("序号")<<tr("类型")<<tr("时间")<<tr("日志信息");
    ui->devLogTableWidget->horizontalHeader()->setStyleSheet("background-color:white");
    ui->devLogTableWidget->setHorizontalHeaderLabels(logheader);
    ui->devLogTableWidget->horizontalHeader()->setVisible(true);//temp
    ui->devLogTableWidget->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击
    ui->devLogTableWidget->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    ui->devLogTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->devLogTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    ui->devLogTableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中

    ui->devLogTableWidget->horizontalHeader()->resizeSection(1,40);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(2,70);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(3,170);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(4,430);


    g_buttonGroup = new QButtonGroup();
    g_buttonGroup->addButton(ui->LastYearButton,1);
    g_buttonGroup->addButton(ui->NextYearButton,2);
    g_buttonGroup->addButton(ui->LastMonButton,3);
    g_buttonGroup->addButton(ui->NextMonButton,4);
    g_buttonGroup->addButton(ui->LastDayButton,5);
    g_buttonGroup->addButton(ui->NextDayButton,6);

    connect(g_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(GroupButtonClickSlot(int)));     //预置点按钮组按键信号连接响应槽函数

    connect(ui->searchSystermLogButton,SIGNAL(clicked()),this,SLOT(searchSystermLog()));
    connect(ui->searchWorkLogButton,SIGNAL(clicked()),this,SLOT(searchSystermLog()));
    connect(ui->LastPageButton,SIGNAL(clicked()),this,SLOT(lastpageSlot()));
    connect(ui->NextPageButton,SIGNAL(clicked()),this,SLOT(nextPageSlot()));

    getTrainConfig();

}

sysManage::~sysManage()
{
    delete ui;
}

void sysManage::searchSystermLog()
{





}

void sysManage::searchWorkLog()
{




}

void sysManage::lastpageSlot()
{





}

void sysManage::nextPageSlot()
{



}




void sysManage::GroupButtonClickSlot(int index)
{
    int buttonIndex = index;
    int iYear = getYear();
    int iMonth = getMonth();
    int iDay = getDay();
    int iMaxDay = 28;

    qDebug()<<"*******buttonIndex="<<buttonIndex<<__LINE__;
    if(buttonIndex == 1)
    {
        iYear--;
        if(iYear <1970)
        {
            return ;
        }
        setYear(iYear);
        iMaxDay = GetMaxDay(iYear,iMonth);
        if(iDay > iMaxDay)
        {
            setDay(iMaxDay);
        }

    }
    else if(buttonIndex == 2)
    {
        iYear++;
        if(iYear >2100)
        {
            return ;
        }
        setYear(iYear);
        iMaxDay = GetMaxDay(iYear,iMonth);
        if(iDay > iMaxDay)
        {
            setDay(iMaxDay);
        }

    }
    else if(buttonIndex == 3)
    {
        iMonth--;
        if(iMonth <1)
        {
            iMonth = 12;
        }
        setMonth(iMonth);
        iMaxDay = GetMaxDay(iYear,iMonth);
        if(iDay > iMaxDay)
        {
            setDay(iMaxDay);
        }
    }
    else if(buttonIndex == 4)
    {
        iMonth ++;
        if(iMonth >12)
        {
            iMonth = 1;
        }
        setMonth(iMonth);
        iMaxDay = GetMaxDay(iYear,iMonth);
        if(iDay > iMaxDay)
        {
            setDay(iMaxDay);
        }

    }
    else if(buttonIndex == 5 || buttonIndex == 6)
    {
        iMaxDay =GetMaxDay(iYear,iMonth);
        if(buttonIndex == 6)
        {
            iDay ++;
        }
        else
        {
            iDay --;
        }
        if(iDay >iMaxDay)
        {
            iDay = 1;
        }
        if(iDay <1)
        {
            iDay = iMaxDay;
        }
        setDay(iDay);
    }

}


void sysManage::hideSysPageSlots()
{
    this->hide();
    emit hideSysPage();
}

void sysManage::setYear(int iYear)
{
    if(m_iYear != iYear)
    {
        char acData[12] = {0};
        sprintf(acData,"%d",iYear);
        m_iYear = iYear;
        ui->yearLabel->setText(QString::number(m_iYear,10));

    }

}

void sysManage::setMonth(int iMonth)
{
    if(m_iMonth != iMonth)
    {
        char acData[12] = {0};
        sprintf(acData,"%d",iMonth);
        m_iMonth = iMonth;
        ui->monthLabel->setText(QString::number(m_iMonth,10));

    }

}

void sysManage::setDay(int iDay)
{
    if(m_iDay != iDay)
    {
        char acData[12] = {0};
        sprintf(acData,"%d",iDay);
        m_iDay = iDay;
        ui->dayLabel->setText(QString::number(m_iDay,10));

    }

}


int sysManage::getYear()
{
    return m_iYear;
}

int sysManage::getMonth()
{
    return m_iMonth;
}
int sysManage::getDay()
{
    return m_iDay;
}



void sysManage::getTrainConfig()     //获取车型配置信息
{
    int i = 0, j = 0, row = 0;
    QString item = "";
    QString devStatus = tr("离线");     //设备状态初始默认值为离线
//    T_TRAIN_CONFIG tTrainConfigInfo;
    char tranNum[32] = {0};


    /*设备状态和设备存储列表清空*/
    row = ui->devStatusTableWidget->rowCount();
    for (i = 0; i < row; i++)
    {
        ui->devStatusTableWidget->removeRow(i);
    }
    ui->devStatusTableWidget->setRowCount(0);


    /*获取各服务器即摄像机信息，填充相应的列表控件*/
    for (i = 0; i < 6; i++)
    {
//        m_NvrServerPhandle[i] = STATE_GetNvrServerPmsgHandle(i);
        row = ui->devStatusTableWidget->rowCount();//获取表格中当前总行数
        ui->devStatusTableWidget->insertRow(row);//添加一行
//        m_aiServerIdex[i] = row+1;
        item = "";
        item = QString::number(row+1);
        item += "车NVR";
        ui->devStatusTableWidget->setItem(row, 0, new QTableWidgetItem(item));  //新建一个文本列并插入到列表中
        ui->devStatusTableWidget->item(row, 0)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);    //设置列文件对齐方式为居中对齐

        item = "0";
        item += "个";
        ui->devStatusTableWidget->setItem(row, 1, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 1)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

        item = "";
        item = "0";
        item += tr("G");
        ui->devStatusTableWidget->setItem(row, 2, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 2)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);


        item = "";
        item = "0";
        item += tr("G");
        ui->devStatusTableWidget->setItem(row, 3, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 3)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);


        item = "";
        item = "离线";
        ui->devStatusTableWidget->setItem(row, 5, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 5)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);


    }


}

