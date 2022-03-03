#include "sysmanage.h"
#include "ui_sysmanage.h"
#include <QAbstractItemView>
#include <QTableWidget>
#include <QDebug>
#include "comm.h"
#include "log/log.h"
#include "NVRMsgProc.h"
#include "unistd.h"
#include <sys/time.h>
#include "stdio.h"
#include <QTime>
#include "pmsg/pmsgcli.h"
#include "debugout/debug.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
static int g_statusflag = 0;

void sysManage::getDevStateSignalCtrl()
{
    int iRet = 0, i = 0, j = 0;
    for(i = 0; i< 6; i++)
    {
        if(g_statusflag == 0)
        {
            m_NvrServerPhandle[i] = STATE_GetNvrServerPmsgHandle(i);
        }
        if (m_aiServerIdex[i] >= 1)
        {
            if (E_SERV_STATUS_CONNECT == PMSG_GetConnectStatus(m_NvrServerPhandle[i]))    //获取到服务器状态为在线
            {
                DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] server %d status is online\n", __FUNCTION__, i+1);
                 ui->devStatusTableWidget->setItem(m_aiServerIdex[i]-1, 5, new QTableWidgetItem(tr("在线")));
                 ui->devStatusTableWidget->item(m_aiServerIdex[i]-1, 5)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                 iRet = PMSG_SendPmsgData(m_NvrServerPhandle[i], CLI_SERV_MSG_TYPE_GET_NVR_STATUS, NULL, 0);
                 if (iRet < 0)
                 {
                     DebugPrint(DEBUG_UI_ERROR_PRINT, "[%s] PMSG_SendPmsgData CLI_SERV_MSG_TYPE_GET_NVR_STATUS error!iRet=%d,server=%d\n", __FUNCTION__, iRet,i+1);
                 }

                 iRet = PMSG_SendPmsgData(m_NvrServerPhandle[i], CLI_SERV_MSG_TYPE_GET_IPC_STATUS, NULL, 0);
                 if (iRet < 0)
                 {
                     DebugPrint(DEBUG_UI_ERROR_PRINT, "[%s] PMSG_SendPmsgData CLI_SERV_MSG_TYPE_GET_IPC_STATUS error!iRet=%d,server=%d\n", __FUNCTION__, iRet,i+1);
                 }

                 m_aiNvrOnlineFlag[i] = 1;

            }
            else
            {
                DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] server %d status is offline\n", __FUNCTION__, i+1);
                ui->devStatusTableWidget->setItem(m_aiServerIdex[i]-1, 5, new QTableWidgetItem(tr("离线")));
                ui->devStatusTableWidget->item(m_aiServerIdex[i]-1, 5)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                ui->devStatusTableWidget->setItem(m_aiServerIdex[i]-1, 4, new QTableWidgetItem(""));

            }

        }


    }

    g_statusflag = 1;


}



sysManage::sysManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sysManage)
{
    ui->setupUi(this);
//    this->setAutoFillBackground(true);
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


    m_GetDevStatethreadId = 0;

    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QPixmap(":/res/bg_system.png")));
    this->setPalette(palette);

    connect(ui->backButton,SIGNAL(clicked()),this,SLOT(hideSysPageSlots()));



    ui->devStatusTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->devStatusTableWidget->setColumnCount(6);
    ui->devStatusTableWidget->setRowCount(6);
    ui->devStatusTableWidget->setShowGrid(true);
//    ui->devStatusTableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(252, 233, 79) }");

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
    ui->devStatusTableWidget->verticalHeader()->setVisible(false);

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
//    ui->devLogTableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(252, 233, 79) }");

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
    ui->devLogTableWidget->verticalHeader()->setVisible(false);

    ui->devLogTableWidget->horizontalHeader()->resizeSection(1,40);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(2,70);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(3,170);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(4,430);


    for (int i = 0; i < MAX_SERVER_NUM; i++)
    {
        m_aiServerIdex[i] = 0;
        m_NvrServerPhandle[i] = 0;
        m_iNoCheckDiskErrNum[i] = 0;
        m_iCheckDiskErrFlag[i] = 0;
        for (int j = 0; j < MAX_CAMERA_OFSERVER; j++)
        {
            m_aiNvrOnlineFlag[i] = 0;
//            m_aiCameraIdex[i][j] = 0;
//            m_aiCameraOnlineFlag[i][j] = 0;
        }
    }


    g_buttonGroup = new QButtonGroup();
    g_buttonGroup->addButton(ui->LastYearButton,1);
    g_buttonGroup->addButton(ui->NextYearButton,2);
    g_buttonGroup->addButton(ui->LastMonButton,3);
    g_buttonGroup->addButton(ui->NextMonButton,4);
    g_buttonGroup->addButton(ui->LastDayButton,5);
    g_buttonGroup->addButton(ui->NextDayButton,6);

    connect(g_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(GroupButtonClickSlot(int)));

    connect(ui->searchSystermLogButton,SIGNAL(clicked()),this,SLOT(searchSystermLog()));
    connect(ui->searchWorkLogButton,SIGNAL(clicked()),this,SLOT(searchSystermLog()));
    connect(ui->LastPageButton,SIGNAL(clicked()),this,SLOT(lastpageSlot()));
    connect(ui->NextPageButton,SIGNAL(clicked()),this,SLOT(nextPageSlot()));


    getTrainConfig();

}

sysManage::~sysManage()
{

    delete  g_buttonGroup;
    g_buttonGroup = NULL;


    delete ui;
}

void sysManage::getNvrStatusCtrl(PMSG_HANDLE pHandle, char *pcMsgData)
{
    int i = 0;
//    char actmp[16] = {0}, acVersion[32] = {0};
    char acDiskFull[16] = {0}, acDiskUsed[16] = {0};
    QString acDeviceTp =  "";
    T_NVR_STATUS *ptNvrstaus = (T_NVR_STATUS *)pcMsgData;
    snprintf(acDiskFull, sizeof(acDiskFull), "%dG", htons(ptNvrstaus->i16HdiskTotalSize));
    snprintf(acDiskUsed, sizeof(acDiskUsed), "%dG", htons(ptNvrstaus->i16HdiskUsedSize));

    for (i =0; i < MAX_SERVER_NUM; i++)
    {
        if ((pHandle == m_NvrServerPhandle[i]) && (m_aiServerIdex[i] >= 1))
        {

            /*第一次连上服务器的3分钟之内不检测硬盘是否异常*/
//            if (0 == m_iCheckDiskErrFlag[i])
//            {
//                m_iNoCheckDiskErrNum[i]++;
//                if (18 == m_iNoCheckDiskErrNum[i])
//                {
//                    m_iCheckDiskErrFlag[i] = 1;
//                    m_iNoCheckDiskErrNum[i] = 0;
//                }
//            }
            m_iCheckDiskErrFlag[i] = 1;


            if (htons(ptNvrstaus->i16HdiskTotalSize) <= 0)
            {
                if (1 == m_iCheckDiskErrFlag[i])
                {
                    ui->devStatusTableWidget->setItem(i, 2, new QTableWidgetItem("0G"));
                    ui->devStatusTableWidget->item(i, 2)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    ui->devStatusTableWidget->setItem(i, 3, new QTableWidgetItem("0G"));
                    ui->devStatusTableWidget->item(i, 3)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);
                    ui->devStatusTableWidget->setItem(i, 4, new QTableWidgetItem(QString(tr("硬盘异常"))));
                    ui->devStatusTableWidget->item(i, 4)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

                }
            }
            else
            {
                ui->devStatusTableWidget->setItem(i, 2, new QTableWidgetItem(QString(QLatin1String(acDiskFull))));
                ui->devStatusTableWidget->item(i, 2)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

                ui->devStatusTableWidget->setItem(i, 3, new QTableWidgetItem(QString(QLatin1String(acDiskUsed))));
                ui->devStatusTableWidget->item(i, 3)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

                ui->devStatusTableWidget->setItem(i, 4, new QTableWidgetItem(QString(tr("正常"))));
                ui->devStatusTableWidget->item(i, 4)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

            }
            break;

        }

    }

}


int sysManage::pmsgCtrl(PMSG_HANDLE pHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen)     //与服务器通信消息处理
{
    switch(ucMsgCmd)
    {
        case SERV_CLI_MSG_TYPE_GET_NVR_STATUS_RESP:
        {
            if (pcMsgData == NULL || iMsgDataLen != 18)
            {
                break;
            }
            else
            {
                getNvrStatusCtrl(pHandle, pcMsgData);
                break;
            }
        }
        case SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP:
        {
            break;
        }

        default:
            break;
    }

    return 0;
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
    int i = 0, row = 0;
    QString item = "";
    QString devStatus = tr("离线");     //设备状态初始默认值为离线

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
        m_NvrServerPhandle[i] = STATE_GetNvrServerPmsgHandle(i);
        row = ui->devStatusTableWidget->rowCount();//获取表格中当前总行数
        ui->devStatusTableWidget->insertRow(row);//添加一行
        m_aiServerIdex[i] = row+1;
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
        const QColor color = QColor(255,0,0);
        ui->devStatusTableWidget->item(row, 5)->setTextColor(color);

    }


}

