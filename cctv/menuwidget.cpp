#include "./debugout/debug.h"
#include "menuwidget.h"
#include "ui_menuwidget.h"
#include "NVRMsgProc.h"
#include "./state/state.h"
#include "./pmsg/pmsgcli.h"
#include <QDebug>

static int g_ipageValue = 0;

menuwidget::menuwidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::menuwidget)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);

    QPalette palette;
    palette.setBrush(QPalette::Background,QBrush(QPixmap(":/res/bg_manforn.png")));
    this->setPalette(palette);

    g_recordManage = new recordManage(this);
    g_recordManage->setGeometry(0,60,g_recordManage->width(),g_recordManage->height());

    g_sysManage = new sysManage(this);
    g_sysManage->setGeometry(0,60,g_sysManage->width(),g_sysManage->height());

    g_recordManage->hide();
    g_sysManage->hide();

    connect(ui->sysyPushButton,SIGNAL(clicked()),this,SLOT(menuButtonClick()));
    connect(ui->recpushButton,SIGNAL(clicked()),this,SLOT(menuButtonClick()));
    connect(g_recordManage,SIGNAL(hideRecSysPage(int)),this,SLOT(hidePageSlots(int)));
    connect(g_sysManage,SIGNAL(hideSysPage(int )),this,SLOT(hidePageSlots(int)));
    connect(this,SIGNAL(sendDeviceSignal()),g_sysManage,SLOT(getDevStateSignalCtrl()));


    m_PmsgTimer = new QTimer(this);
    m_PmsgTimer->start(500);
    connect(m_PmsgTimer, SIGNAL(timeout()), this, SLOT(pmsgTimerFunc()));

    ui->sysyPushButton->setChecked(true);
    ui->recpushButton->setChecked(false);


}

menuwidget::~menuwidget()
{
    if (m_PmsgTimer != NULL)
    {
        if (m_PmsgTimer ->isActive())
        {
            m_PmsgTimer ->stop();
        }
        delete m_PmsgTimer;
        m_PmsgTimer  = NULL;
    }

    delete g_recordManage;
    g_recordManage =  NULL;

    delete g_sysManage;
    g_sysManage = NULL;

    delete ui;
}

void menuwidget::hidePageSlots(int index)
{
    if(g_iPageNumber == 1)
    {
        g_iPageNumber = 0;
    }
    g_ipageValue = index;
    this->hide();
    emit  sendhidesignal();
}


void menuwidget::showMainfornPage()
{
    if(g_iPageNumber == 0)
    {
        g_iPageNumber = 1;
    }
    if(g_ipageValue == 0)
    {
        ui->sysyPushButton->setStyleSheet("background-color: rgb(252, 233, 79)");
        ui->recpushButton->setStyleSheet("background-color: rgb(23, 119, 244)");
        g_sysManage->show();
    }
    else
    {
        ui->recpushButton->setStyleSheet("background-color: rgb(252, 233, 79)");
        ui->sysyPushButton->setStyleSheet("background-color: rgb(23, 119, 244)");
        g_recordManage->show();
    }
    this->show();
}

void menuwidget::getDevStateSlot()
{
    emit sendDeviceSignal();

}

void menuwidget::menuButtonClick()
{
    QObject* Sender = sender();     //Sender->objectName(),???????????????????????????????????????????????????????????????


    if(Sender==0)
    {
        return ;
    }
    if (Sender->objectName() == "sysyPushButton")     //???????????????????????????????????????????????????????????????
    {
        ui->sysyPushButton->setStyleSheet("background-color: rgb(252, 233, 79)");
        ui->recpushButton->setStyleSheet("background-color: rgb(23, 119, 244)");
        g_recordManage->closePlayWin();
        g_recordManage->hide();
        g_sysManage->show();
    }
    else
    {
        ui->sysyPushButton->setStyleSheet("background-color: rgb(23, 119, 244)");
        ui->recpushButton->setStyleSheet("background-color: rgb(252, 233, 79)");
        g_sysManage->hide();
        g_recordManage->show();
    }

}

void menuwidget::pmsgTimerFunc()
{
    int i = 0, iRet = 0;
    T_PMSG_PACKET tPkt;
    PMSG_HANDLE pmsgHandle = 0;
    for( i = 0 ; i< 6; i++)
    {
        memset(&tPkt, 0, sizeof(T_CMD_PACKET));
        pmsgHandle = STATE_GetNvrServerPmsgHandle(i);
        iRet = PMSG_GetDataFromPmsgQueue(pmsgHandle, &tPkt);
        if (iRet < 0)
        {
            DebugPrint(DEBUG_PMSG_ERROR_PRINT, "get server pmsg data error\n");
            continue;
        }
        recvPmsgCtrl(tPkt.PHandle, tPkt.ucMsgCmd, tPkt.pcMsgData, tPkt.iMsgDataLen);
        if (tPkt.pcMsgData)
        {
            free(tPkt.pcMsgData);
            tPkt.pcMsgData = NULL;
        }

    }


}

void menuwidget::recvPmsgCtrl(PMSG_HANDLE pHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen)
{
//    qDebug()<<"*************ucMsgCmd****"<<ucMsgCmd<<__FUNCTION__<<__LINE__;
    switch(ucMsgCmd)    //?????????????????????????????????????????????????????????
    {
        case SERV_CLI_MSG_TYPE_GET_RECORD_TIME_LEN_RESP:
        case SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP:
        {
            DebugPrint(DEBUG_PMSG_NORMAL_PRINT, "pvmsMenu Widget get pmsg message 0x%x, msgDataLen=%d\n",(int)ucMsgCmd, iMsgDataLen);
            if (pcMsgData == NULL)
            {
                break;
            }

            if (g_recordManage != NULL)
            {
                g_recordManage->pmsgCtrl(pHandle, ucMsgCmd, pcMsgData, iMsgDataLen);
            }
            break;
        }

        case SERV_CLI_MSG_TYPE_GET_NVR_STATUS_RESP:
//        case SERV_CLI_MSG_TYPE_PISMSG_REPORT:
        case SERV_CLI_MSG_TYPE_GET_IPC_STATUS_RESP:
        {
            if (pcMsgData == NULL)
            {
                break;
            }
            else
            {
                if (g_sysManage != NULL)
                {
                    g_sysManage->pmsgCtrl(pHandle, ucMsgCmd, pcMsgData, iMsgDataLen);
                }

            }
        }


        default:
            break;


    }


}
