#include "recordmanage.h"
#include "ui_recordmanage.h"
#include "state/state.h"
#include <QDebug>
#include <QMessageBox>
#include "NVRMsgProc.h"
#include "stdio.h"
#include <time.h>
#include "unistd.h"
#include "ftp/ftpApi.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <QCheckBox>
#include <QHBoxLayout>


#define FTP_SERVER_PORT  21   //FTP服务器默认通信端?

static PFTP_HANDLE g_ftpHandle = 0;

int g_iDateEditNo = 0;      //要显示时间的不同控件的编号
pthread_mutex_t g_sliderValueSetMutex;
recordManage *g_recordPlayThis = NULL;

void PftpProc(PFTP_HANDLE PHandle, int iPos)     //回调函数处理接收到的进度条进度, iPos为进度百分比
{
    T_LOG_INFO tLog;
    memset(&tLog,0,sizeof(T_LOG_INFO));
    tLog.iLogType = LOG_TYPE_EVENT;


    if (PHandle != g_recordPlayThis->m_tFtpHandle[g_recordPlayThis->m_iFtpServerIdex])
    {
        return;
    }
    g_recordPlayThis->triggerDownloadProcessBarDisplaySignal(1);   //显示进度条

    g_recordPlayThis->triggerSetDownloadProcessBarValueSignal(iPos);

    if ((100 == iPos) || (-1 == iPos) || (-2 == iPos) || (-3 == iPos))  //iPos=100,表示下载完毕。暂定iPos=-1表示被告知U盘已拔出, iPos=-2表示被告知U盘写入失败,iPos=-3表示被告知数据接收失败失败。 三种情况都隐藏进度条，并在信号处理函数中销毁FTP连接
    {
        if(100 == iPos)
        {
            snprintf(tLog.acLogDesc,sizeof(tLog.acLogDesc)-1,"down succuss");
            LOG_WriteLog(&tLog);

            usleep(2000*1000);
        }

        g_recordPlayThis->triggerDownloadProcessBarDisplaySignal(0);
    }

    return ;
}


recordManage::recordManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::recordManage)
{
    ui->setupUi(this);

    connect(ui->backButton,SIGNAL(clicked()),this,SLOT(hideRecPageSlots()));
    ui->playSpeedlabel->setText("1.00");

    m_playSlider = new mySlider(this);    //创建播放进度条
    m_playSlider->setOrientation(Qt::Horizontal);    //设置水平方向
    m_playSlider->setGeometry(330, 584, 580, 29);
    m_playSlider->show();
    /*定义播放进度条样式*/
    m_playSlider->setStyleSheet("QSlider::groove:horizontal{border: 1px solid #4A708B;background: #C0C0C0;height: 5px;border-radius: 1px;padding-left:-1px;padding-right:-1px;}"
                                "QSlider::sub-page:horizontal{background: qlineargradient(x1:0, y1:0, x2:0, y2:1,stop:0 #B1B1B1,stop:1 #c4c4c4);background:qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,stop: 0 #5DCCFF,stop: 1 #1874CD);border: 1px solid #4A708B;height: 10px;border-radius: 2px;}"
                                "QSlider::add-page:horizontal{background: #575757;border: 0px solid #777;height: 10px;border-radius: 2px;}"
                                "QSlider::handle:horizontal{background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5,stop:0.6 #45ADED, stop:0.778409 rgba(255, 255, 255, 255));width: 11px;margin-top: -3px;margin-bottom: -3px;border-radius: 5px;}"
                                "QSlider::handle:horizontal:hover{background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5, fx:0.5, fy:0.5, stop:0.6 #2A8BDA,stop:0.778409 rgba(255, 255, 255, 255));width: 11px;margin-top: -3px;margin-bottom: -3px;border-radius: 5px;}"
                                "QSlider::sub-page:horizontal:disabled{background: #00009C;border-color: #999;}"
                                "QSlider::add-page:horizontal:disabled{background: #eee;border-color: #999;}"
                                "QSlider::handle:horizontal:disabled{background: #eee;border: 1px solid #aaa;border-radius: 4px;}");
    connect(m_playSlider, SIGNAL(sliderPressSianal(int)), this, SLOT(playSliderPressSlot(int)));   //点击进度条信号响应
    connect(m_playSlider, SIGNAL(sliderMoveSianal(int)), this, SLOT(playSliderMoveSlot(int)));   //拖动进度条信号响应

    m_tableWidgetStyle = QStyleFactory::create("windows");
    ui->recordFileTableWidget->setStyle(m_tableWidgetStyle);   //设置tablewidget显示风格为windows风格，否则里面的checkbox选中默认显示叉而不是勾
    ui->recordFileTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->recordFileTableWidget->setShowGrid(true);

    ui->recordFileTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->recordFileTableWidget->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击
    ui->recordFileTableWidget->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    ui->recordFileTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->recordFileTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置整行选中方式
    ui->recordFileTableWidget->setSelectionMode(QAbstractItemView::NoSelection); //设置只能选择一行，不能多行选中
    ui->recordFileTableWidget->setAlternatingRowColors(true);                        //设置隔一行变一颜色，即：一灰一白
    ui->recordFileTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    ui->recordFileTableWidget->horizontalHeader()->resizeSection(0,46); ////设置表头第一列的宽度为46
    ui->recordFileTableWidget->horizontalHeader()->resizeSection(1,46);
    ui->recordFileTableWidget->horizontalHeader()->resizeSection(2,219);

    //参数初始化
    memset(&m_RealMonitorVideos, 0, sizeof(m_RealMonitorVideos));
    m_RealMonitorVideos.pRenderHandle = NULL;
    FileSearchTimer = NULL;
    m_pcRecordFileBuf = (char *)malloc(MAX_RECORD_SEACH_NUM*MAX_RECFILE_PATH_LEN);
    m_cmpHandle = NULL;
    m_iPlayRange = 0;
    m_iPlayFlag = 0;
    m_iRecordIdex = -1;
//    g_fistSelctFlag = -1;
    recordPlayFlag = 0;
    m_iSliderValue = 0;
    m_threadId = 0;

    for (int i = 0; i < MAX_SERVER_NUM; i++)
    {
        m_Phandle[i] = 0;
        m_tFtpHandle[i] = 0;

    }

    ui->fileDownloadProgressBar->hide();

    struct tm *pLocalTime;
    char acTime[56] = {0};
    time_t tTime;
    time(&tTime);
    pLocalTime = localtime(&tTime);
    m_tEndTime.year  = pLocalTime->tm_year +1900;
    m_tEndTime.mon	= pLocalTime->tm_mon +1;
    m_tEndTime.day	= pLocalTime->tm_mday;
    m_tEndTime.hour	= 23;
    m_tEndTime.min	= 59;
    m_tEndTime.sec	= 59;

    m_tStartTime.year  = m_tEndTime.year;
    m_tStartTime.mon   = m_tEndTime.mon;
    m_tStartTime.day   = m_tEndTime.day;
    m_tStartTime.hour  = 0;
    m_tStartTime.min   = 0;
    m_tStartTime.sec   = 0;

    for(int i =0;i<2;i++)
    {
        memset(acTime,0,sizeof(acTime));
        if(0 == i)
        {
            sprintf(acTime,"%d-%02d-%02d 00:00:00",m_tStartTime.year,m_tStartTime.mon,m_tStartTime.day);
            ui->startTimeLabel->setText(acTime);

        }
        else if(1 == i)
        {
            sprintf(acTime,"%d-%02d-%02d 23:59:59",m_tStartTime.year,m_tStartTime.mon,m_tStartTime.day);
            ui->endTimeLabel->setText(acTime);

        }

    }

    playWidget = new QWidget(this);
    playWidget->setGeometry(340,10,680,570);
    playWidget->setStyleSheet("QWidget{background-color: rgb(0, 0, 0);}");     //设置播放窗口背景色为黑色
    playWidget->show();

    timeSetWidget = new timeset(this);
    timeSetWidget->setWindowFlags(timeSetWidget->windowFlags() | Qt::FramelessWindowHint| Qt::Dialog);
    timeSetWidget->setWindowModality(Qt::ApplicationModal);
    timeSetWidget->hide();

    connect(timeSetWidget, SIGNAL(timeSetSendMsg(QString,QString,QString,QString,QString,QString)), this, SLOT(timeSetRecvMsg(QString,QString,QString,QString,QString,QString)));  //时间设置窗体控件设置信号响应
    connect(ui->startTimeLabel, SIGNAL(clicked()), this, SLOT(openStartTimeSetWidgetSlot()));   //开始时间设置按钮按键信号响应
    connect(ui->endTimeLabel, SIGNAL(clicked()), this, SLOT(openStopTimeSetWidgetSlot()));  //结束时间设置按钮按键信号响应

    getTrainConfig();

    connect(ui->searchloadButton,SIGNAL(clicked()),this,SLOT(SearchBtnClicked()));
    connect(ui->downloadButton,SIGNAL(clicked()),this,SLOT(DownBtnClicked()));
    connect(ui->playLastOnePushButton,SIGNAL(clicked()),this,SLOT(PrevBtnClicked()));
    connect(ui->slowForwardPushButton,SIGNAL(clicked()),this,SLOT(SlowBtnClicked()));
    connect(ui->playPushButton,SIGNAL(clicked()),this,SLOT(PlayBtnClicked()));
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(PauesBtnClicked()));
    connect(ui->stopPushButton,SIGNAL(clicked()),this,SLOT(StopBtnClicked()));
    connect(ui->fastForwardPushButton,SIGNAL(clicked()),this,SLOT(QuickBtnClicked()));
    connect(ui->playNextOnePushButton,SIGNAL(clicked()),this,SLOT(NextBtnClicked()));


    connect(ui->carSeletionComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(carNoChangeSlot()));
    connect(ui->recordFileTableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(recordSelectionSlot(QTableWidgetItem*)));  //单击录像文件列表某行触发信号连接相应槽函数
    connect(ui->recordFileTableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(recordPlaySlot(QTableWidgetItem*)));  //双击录像文件列表某行触发信号连接相应槽函数


    connect(this, SIGNAL(recordTableWidgetFillSignal()), this, SLOT(recordTableWidgetFillSlot()));
    connect(this, SIGNAL(setSliderValueSignal(int)), this, SLOT(setPlaySliderValueSlot(int)));
    connect(this, SIGNAL(closeRecordPlaySignal()), this, SLOT(closeRecordPlaySlot()));
    connect(this, SIGNAL(recordSeletPlay(QTableWidgetItem *)), this, SLOT(recordPlaySlot(QTableWidgetItem*)));
    connect(this, SIGNAL(setRangeLabelSignal()), this, SLOT(setRangeLabelSlot()));

    connect(this, SIGNAL(downloadProcessBarDisplaySignal(int)), this, SLOT(downloadProcessBarDisplaySlot(int)));
    connect(this, SIGNAL(setDownloadProcessBarValueSignal(int)), this, SLOT(setDownloadProcessBarValueSlot(int)));

    ui->searchloadButton->setFocusPolicy(Qt::NoFocus);
    ui->downloadButton->setFocusPolicy(Qt::NoFocus);
    ui->playPushButton->setFocusPolicy(Qt::NoFocus);
    ui->stopPushButton->setFocusPolicy(Qt::NoFocus);
    ui->pushButton->setFocusPolicy(Qt::NoFocus);
    ui->playLastOnePushButton->setFocusPolicy(Qt::NoFocus);
    ui->playNextOnePushButton->setFocusPolicy(Qt::NoFocus);
    ui->fastForwardPushButton->setFocusPolicy(Qt::NoFocus);
    ui->slowForwardPushButton->setFocusPolicy(Qt::NoFocus);

    g_recordPlayThis = this;

}

recordManage::~recordManage()
{

    pthread_mutex_destroy(&g_sliderValueSetMutex);
    closePlayWin();

    if (m_tableWidgetStyle != NULL)
    {
        delete m_tableWidgetStyle;
        m_tableWidgetStyle = NULL;
    }

    delete timeSetWidget;
    timeSetWidget = NULL;

    delete  playWidget;
    playWidget = NULL;

    free(m_pcRecordFileBuf);
    m_pcRecordFileBuf = NULL;

    delete ui;
}

void recordManage::cmplaybackInit()
{
    if( m_RealMonitorVideos.pRenderHandle)
        return;
    QRect rt;
    QPoint pt;
    QWidget *pWnd = playWidget; //
    rt = pWnd->geometry();
    pt = pWnd->mapToGlobal(QPoint(0, 0));
//    m_RealMonitorVideos.nVideoWidth = 0;
//    m_RealMonitorVideos.nVideoHeight = 0;
//    m_RealMonitorVideos.nX = rt.x();
//    m_RealMonitorVideos.nY = rt.y();
//    m_RealMonitorVideos.nWidth = rt.width();
//    m_RealMonitorVideos.nHeight = rt.height();
    m_RealMonitorVideos.hWnd = (HWND)pWnd;
    m_RealMonitorVideos.pRenderHandle = NULL;

}


static char *parseFileName(char *pcSrcStr)     //根据录像文件路径全名解析得到单纯的录像文件名
{
    char *pcTmp = NULL;
    if (NULL == pcSrcStr)
    {
        return NULL;
    }

    pcTmp = strrchr(pcSrcStr, '/');
    if (NULL == pcTmp)
    {
        return pcSrcStr;
    }

    if (0 == *(pcTmp+1))
    {
        return NULL;
    }
    return pcTmp+1;
}

int recordManage::pmsgCtrl(PMSG_HANDLE pHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen)
{
    char *pcToken = NULL;

    if (0 == pHandle)
    {
        return 0;
    }
    switch(ucMsgCmd)
    {
        case SERV_CLI_MSG_TYPE_GET_RECORD_TIME_LEN_RESP:
        {
            if (pcMsgData == NULL || iMsgDataLen != 2)
            {
                break;
            }
            else
            {
                short *iDuration = (short *)pcMsgData;
                DebugPrint(DEBUG_PMSG_DATA_PRINT, "recordPlay Widget get pmsg response cmd 0x%x data:%d\n", ucMsgCmd,*iDuration);
                m_iPlayRange = htons(*iDuration);
                break;
            }
        }
        case SERV_CLI_MSG_TYPE_GET_RECORD_FILE_RESP:
        {
            pcToken = &pcMsgData[2];
            iMsgDataLen = iMsgDataLen-2;  //先得到真正录像文件信息的内容长度，前两个字节表示分包序号
            DebugPrint(DEBUG_PMSG_DATA_PRINT, "recordPlay Widget get pmsg response cmd 0x%x data:\n%s\n", ucMsgCmd,pcToken);

            if (NULL == pcToken || iMsgDataLen <= 0)
            {
                break;
            }
            recordQueryCtrl(pcToken, iMsgDataLen);    //触发录像查询处理信号，在UI主线程中处理，而不在这里直接处理

            break;
        }
        default:
            break;
    }


    return 0;

}

void recordManage::triggerSetRangeLabelSignal()
{
    emit setRangeLabelSignal();

}
void recordManage::triggerSetSliderValueSignal(int iValue)   //触发设置播放进度条值的信号
{
    emit setSliderValueSignal(iValue);

}
void recordManage::triggerCloseRecordPlaySignal()
{
    emit closeRecordPlaySignal();
}

void recordManage::triggerDownloadProcessBarDisplaySignal(int iEnableFlag)   //触发是否显示文件下载进度条的信号，iEnableFlag为1，显示，为0不显示
{
    emit downloadProcessBarDisplaySignal(iEnableFlag);

}

void recordManage::triggerSetDownloadProcessBarValueSignal(int iValue)	//触发设置文件下载进度条的值的信号
{
    emit setDownloadProcessBarValueSignal(iValue);
}

void recordManage::setRangeLabelSlot()
{
    char acStr[32] = {0};
    int iMin = 0, iSec = 0;

    iMin = m_iPlayRange / 60;
    iSec = m_iPlayRange % 60;

    snprintf(acStr, sizeof(acStr), "%02d", iMin);
    ui->rangeMinLabel->setText(QString(QLatin1String(acStr)));

    memset(acStr, 0, sizeof(acStr));
    snprintf(acStr, sizeof(acStr), "%02d", iSec);
    ui->rangeSecLabel->setText(QString(QLatin1String(acStr)));

}
void recordManage::setPlaySliderValueSlot(int iValue)       //刷新进度条
{
    char acStr[32] = {0};
    int iMin = 0, iSec = 0;

    iMin = iValue / 60;
    iSec = iValue % 60;

    snprintf(acStr, sizeof(acStr), "%02d", iMin);
    ui->playMinLabel->setText(QString(QLatin1String(acStr)));

    memset(acStr, 0, sizeof(acStr));
    snprintf(acStr, sizeof(acStr), "%02d", iSec);
    ui->playSecLabel->setText(QString(QLatin1String(acStr)));

    m_playSlider->setValue(iValue);
}
void recordManage::closeRecordPlaySlot()
{
    closePlayWin();
}

void recordManage::playSliderMoveSlot(int iPosTime)
{
    QString playSpeedStr = "";

    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget  push play slider!\n");

    if (iPosTime < 0)
    {
        return;
    }
    else if (iPosTime == m_playSlider->value())    //时间值没有变化不进行处理
    {
        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] value is not change, do not set!\n", __FUNCTION__);
        return;
    }
    else if (0 == iPosTime) //防止pos值为0而服务器不处理，所有值最小为1
    {
        iPosTime = 1;
    }

    DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s]  push play slider value=%d!\n", __FUNCTION__, iPosTime);

    if (m_cmpHandle != NULL)
    {
        if (CMP_STATE_PAUSE == CMP_GetPlayStatus(m_cmpHandle))
        {
            return;
        }

        m_iPlayFlag = 1;
        m_dPlaySpeed = 1.00;
        playSpeedStr = "1.00";
        ui->playSpeedlabel->setText(playSpeedStr);

        pthread_mutex_lock(&g_sliderValueSetMutex);
        //m_iSliderValue = iPosTime;
        m_playSlider->setValue(iPosTime);
        CMP_SetPosition(m_cmpHandle, iPosTime);
        pthread_mutex_unlock(&g_sliderValueSetMutex);
    }



}
void recordManage::playSliderPressSlot(int iPosTime)
{
    QString playSpeedStr = "";

    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget  press play slider!\n");

    if (iPosTime < 0)
    {
        return;
    }
    else if (iPosTime == m_playSlider->value())    //时间值没有变化不进行处理
    {
        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] value is not change, do not set!\n", __FUNCTION__);
        return;
    }
    else if (0 == iPosTime) //防止pos值为0而服务器不处理，所有值最小为1
    {
        iPosTime = 1;
    }

    DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s]  push play slider value=%d!\n", __FUNCTION__, iPosTime);

    if (m_cmpHandle != NULL)
    {
        if (CMP_STATE_PAUSE == CMP_GetPlayStatus(m_cmpHandle))
        {
            return;
        }

        m_iPlayFlag = 1;
        m_dPlaySpeed = 1.00;
        playSpeedStr = "1.00";
        ui->playSpeedlabel->setText(playSpeedStr);

        pthread_mutex_lock(&g_sliderValueSetMutex);
        //m_iSliderValue = iPosTime;
        m_playSlider->setValue(iPosTime);
        CMP_SetPosition(m_cmpHandle, iPosTime);
        pthread_mutex_unlock(&g_sliderValueSetMutex);
    }


}

void recordManage::manualSwitchVideoEndSlot()
{
    ui->playLastOnePushButton->setEnabled(true);
    ui->playNextOnePushButton->setEnabled(true);
    if (m_VideoSwitchTimer != NULL)
    {
        if (m_VideoSwitchTimer->isActive())
        {
            m_VideoSwitchTimer->stop();
        }
        delete m_VideoSwitchTimer;
        m_VideoSwitchTimer = NULL;
    }

}

void recordManage::manualtableSwitchVideoEndSlot()
{
    ui->recordFileTableWidget->setEnabled(true);

    if (m_tableVideoSwitchTimer != NULL)
    {
        if (m_tableVideoSwitchTimer->isActive())
        {
            m_tableVideoSwitchTimer->stop();
        }
        delete m_tableVideoSwitchTimer;
        m_tableVideoSwitchTimer = NULL;
    }

}




void recordManage::downloadProcessBarDisplaySlot(int iEnableFlag)   //是否显示文件下载进度条，iEnableFlag为1，显示，为0不显示
{
    if ((0 == iEnableFlag)/* && (0 == ui->fileDownloadProgressBar->isHidden())*/)
    {

        if(ui->fileDownloadProgressBar->isVisible() == true)
        {
            ui->fileDownloadProgressBar->hide();
        }
        if(ui->searchloadButton->isEnabled() == false)
        {
            ui->searchloadButton->setEnabled(true);
        }
        if(ui->downloadButton->isEnabled() == false)
        {
            ui->downloadButton->setEnabled(true);
        }

//        g_downloadFlag = 0;


    }
    else if ((1 == iEnableFlag) /*&& (1 == ui->fileDownloadProgressBar->isHidden())*/)
    {

        if(ui->fileDownloadProgressBar->isVisible() == false)
        {
            ui->fileDownloadProgressBar->show();
        }
        if(ui->searchloadButton->isEnabled() == true)
        {
            ui->searchloadButton->setEnabled(false);
        }
        if(ui->downloadButton->isEnabled() == true)
        {
            ui->downloadButton->setEnabled(false);
        }

//        g_downloadFlag = 1;
    }



}


void recordManage::setDownloadProcessBarValueSlot(int iValue)   //是否显示文件下载进度条，iEnableFlag为1，显示，为0不显示
{
    if (-1 == iValue) //iValue=-1时,表示被告知U盘已拔出,销毁FTP连接并弹框提示
    {
        ui->fileDownloadProgressBar->hide();

        FTP_DestoryConnect(m_tFtpHandle[m_iFtpServerIdex]);
        m_tFtpHandle[m_iFtpServerIdex] = 0;

        static QMessageBox box(QMessageBox::Warning,QString::fromUtf8("错误"),QString::fromUtf8("下载失败，U盘已被拔出!"));
        box.setWindowFlags(Qt::FramelessWindowHint);
        box.setStandardButtons (QMessageBox::Ok);
        box.setButtonText (QMessageBox::Ok,QString::fromUtf8("OK"));
        box.exec();
        return;
    }

    if (-2 == iValue) //iValue=-2时,表示被告知U盘写入失败,销毁FTP连接并弹框提示
    {
        ui->fileDownloadProgressBar->hide();

        FTP_DestoryConnect(m_tFtpHandle[m_iFtpServerIdex]);
        m_tFtpHandle[m_iFtpServerIdex] = 0;

        static QMessageBox box(QMessageBox::Warning,QString::fromUtf8("错误"),QString::fromUtf8("下载失败，U盘写入失败!"));
        box.setWindowFlags(Qt::FramelessWindowHint);
        box.setStandardButtons (QMessageBox::Ok);
        box.setButtonText (QMessageBox::Ok,QString::fromUtf8("OK"));
        box.exec();
        return;
    }

    if (-3 == iValue) //iValue=-3时,表示被告知数据接收失败,销毁FTP连接并弹框提示
    {
        ui->fileDownloadProgressBar->hide();

        FTP_DestoryConnect2(m_tFtpHandle[m_iFtpServerIdex]);
        m_tFtpHandle[m_iFtpServerIdex] = 0;


        static QMessageBox box(QMessageBox::Warning,QString::fromUtf8("错误"),QString::fromUtf8("下载失败，数据接收失败!"));
        box.setWindowFlags(Qt::FramelessWindowHint);
        box.setStandardButtons (QMessageBox::Ok);
        box.setButtonText (QMessageBox::Ok,QString::fromUtf8("OK"));
        box.exec();
        return;
    }

    ui->fileDownloadProgressBar->setValue(iValue);

    if (100 == iValue)   //iValue=100,下载结束，销毁ftp连接
    {
        ui->fileDownloadProgressBar->hide();
        FTP_DestoryConnect(m_tFtpHandle[m_iFtpServerIdex]);
        m_tFtpHandle[m_iFtpServerIdex] = 0;
        return;

    }


}


void *slideValueSetThread(void *param)    //播放进度条刷新线程
{
    int iDuration = 0, iTryGetPlayRangeNum = 5;
      recordManage *recordPlaypage = (recordManage *)param;
      if (NULL == recordPlaypage)
      {
          return NULL;
      }
//      pthread_detach(pthread_self());

      while (1 == recordPlaypage->m_iThreadRunFlag)
      {
          if (0 == recordPlaypage->m_iPlayRange)
          {
              while (1 == recordPlaypage->m_iThreadRunFlag && iTryGetPlayRangeNum > 0)     //尝试5次获取播放时长，每次间隔1000MS
              {
//                  iDuration = CMP_GetPlayRange(recordPlaypage->m_cmpHandle);
                  iDuration =  CMP_GetPlayRange(recordPlaypage->m_cmpHandle);

                  if (iDuration > 0)
                  {
                      break;
                  }
                  iTryGetPlayRangeNum--;
                  usleep(1000*1000);
              }
              if (iDuration > 0)
              {
                  recordPlaypage->m_iPlayRange = iDuration;
//                  DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s-%d] m_iPlayRange=%d!\n",__FUNCTION__, __LINE__, recordPlaypage->m_iPlayRange);
              }
              else
              {
                  recordPlaypage->m_iPlayRange = 600;
//                  DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s-%d] m_iPlayRange=%d!\n",__FUNCTION__, __LINE__, recordPlaypage->m_iPlayRange);
              }
              recordPlaypage->m_playSlider->setRange(0, recordPlaypage->m_iPlayRange);

              recordPlaypage->triggerSetRangeLabelSignal();
          }
//            qDebug()<<"***********recordPlaypage->m_iPlayRange="<<recordPlaypage->m_iPlayRange<<__LINE__;
          if ((recordPlaypage->m_iPlayRange > 0) && (recordPlaypage->m_iPlayFlag != 0))   //只有获取到了进度条范围值,并且不处于暂停状态才会刷新进度条，否则不做刷新处理
          {
              pthread_mutex_lock(&g_sliderValueSetMutex);
//              recordPlaypage->m_iSliderValue = CMP_GetCurrentPlayTime(recordPlaypage->m_cmpHandle);
              recordPlaypage->m_iSliderValue = CMP_GetPlayTime(recordPlaypage->m_cmpHandle);
              recordPlaypage->triggerSetSliderValueSignal(recordPlaypage->m_iSliderValue);
              pthread_mutex_unlock(&g_sliderValueSetMutex);
              if (recordPlaypage->m_iSliderValue >= recordPlaypage->m_iPlayRange)   //进度到100%，表示该段录像回放完毕，关闭播放窗口
              {
//                  DebugPrint(DEBUG_UI_NOMAL_PRINT, "recordPlayWidget record play end!close play window\n");
                  recordPlaypage->triggerCloseRecordPlaySignal();
              }
//              qDebug()<<"***********recordPlaypage->m_iSliderValue="<<recordPlaypage->m_iSliderValue<<__LINE__;

          }
          usleep(500*1000);
      }


    return NULL;

}

void recordManage::recordPlayCtrl(int iRow, int iDex)
{
    int iRet = 0;
    char acRtspAddr[128] = {0};
//    T_TRAIN_CONFIG tTrainConfigInfo;
    T_LOG_INFO tLogInfo;
    QString playSpeedStr;


    /*每次播放开始时播放时长清0，设置播放进度条范围值为0，使播放进度条复位*/
    m_iPlayRange = 0;
    m_playSlider->setRange(0, m_iPlayRange);
    m_iSliderValue = 0;

    m_iPlayFlag = 1;
    m_dPlaySpeed = 1.00;
    playSpeedStr = QString::number(m_dPlaySpeed);
    if (m_dPlaySpeed == (int)m_dPlaySpeed)
    {
        playSpeedStr += ".00";
    }
    else
    {
        playSpeedStr += "";
    }
    ui->playSpeedlabel->setText(playSpeedStr);

    snprintf(acRtspAddr, sizeof(acRtspAddr), "rtsp://192.168.%d.81:554%s",4+100, m_acFilePath[iRow]);
    if (NULL == m_cmpHandle)
    {
        m_RealMonitorVideos.hWnd = playWidget;
        m_cmpHandle = CMP_Init(&m_RealMonitorVideos,CMP_VDEC_NORMAL);
        CMP_OpenMediaFile(m_cmpHandle, acRtspAddr, CMP_TCP);
        if(NULL == m_cmpHandle)
        {
            return;
        }

    }
    CMP_PlayMedia(m_cmpHandle);
    CMP_SetPlayEnable(m_cmpHandle,1);
    if(iRet < 0)
    {
        return;
    }
    m_iRecordIdex = iRow;
    for (int i = 0; i < ui->recordFileTableWidget->rowCount(); i++)
    {
        if (0 == ui->recordFileTableWidget->item(i, 2)->text().contains("tmp") && i == m_iRecordIdex)
        {
            ui->recordFileTableWidget->item(i, 1)->setTextColor(Qt::blue);
            ui->recordFileTableWidget->item(i, 2)->setForeground(Qt::blue);
        }
        else
        {
            ui->recordFileTableWidget->item(i, 1)->setTextColor(Qt::black);
            ui->recordFileTableWidget->item(i, 2)->setForeground(Qt::black);

        }
    }

    usleep(200*1000);
    if (0 == m_threadId)    //保证播放进度条刷新线程只创建一次
    {
        m_iThreadRunFlag = 1;
//        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] create slideValueSet thread begin!\n",__FUNCTION__);
        pthread_create(&m_threadId, NULL, slideValueSetThread, (void *)this);    //创建线程

        if (0 == m_threadId)
        {
//            DebugPrint(DEBUG_UI_ERROR_PRINT, "[%s] create slideValueSet thread error\n", __FUNCTION__);
        }
//        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] create slideValueSet thread end!\n",__FUNCTION__);
    }

//    memset(&tLogInfo, 0, sizeof(T_LOG_INFO));
//    tLogInfo.iLogType = 0;
//    snprintf(tLogInfo.acLogDesc, sizeof(tLogInfo.acLogDesc), "play %s of nvr server %d", m_acFilePath[iRow], tTrainConfigInfo.tNvrServerInfo[iDex].iCarriageNO+100);
//    LOG_WriteLog(&tLogInfo);

}

void recordManage::recordQueryCtrl(char *pcMsgData, int iMsgDataLen)
{
    if (m_iTotalLen > (MAX_RECORD_SEACH_NUM*MAX_RECFILE_PATH_LEN - 1))
    {
        return;
    }
    memcpy(m_pcRecordFileBuf+m_iTotalLen, pcMsgData, iMsgDataLen);
    m_iTotalLen += iMsgDataLen;

}

void recordManage::NextBtnClicked()
{
    int iDex = 0, iRow = 0;

//       DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget nextOne play PushButton pressed!\n");

    if (ui->recordFileTableWidget->rowCount() <= 0 || NULL == m_cmpHandle)
    {
       return;
    }

    iDex = ui->carSeletionComboBox->currentIndex();
    if (iDex < 0)
    {
       return;
    }

    iRow = m_iRecordIdex + 1;
    if (iRow > (ui->recordFileTableWidget->rowCount() - 1))
    {
       return;
    }


    closePlayWin();   //先关闭之前的
//       setPlayButtonStyleSheet();
//    emit setRecordPlayFlagSignal(1);


    recordPlayCtrl(iRow, iDex);

    ui->playNextOnePushButton->setEnabled(false);
    ui->playLastOnePushButton->setEnabled(false);

    if(m_VideoSwitchTimer == NULL)
    {
       m_VideoSwitchTimer = new QTimer(this);
       m_VideoSwitchTimer->start(1*1000);
       connect(m_VideoSwitchTimer,SIGNAL(timeout()), this,SLOT(manualSwitchVideoEndSlot()));
    }



}

void recordManage::QuickBtnClicked()
{
    QString playSpeedStr;
    if (NULL == m_cmpHandle)
    {
       printf("*****NULL == m_cmpHandle****line1036\n");
       return;
    }

    if (m_dPlaySpeed >= 4.00)
    {
       return;
    }
    m_iPlayFlag = 1;
    m_dPlaySpeed = m_dPlaySpeed*2;

    playSpeedStr = QString::number(m_dPlaySpeed);
    if (m_dPlaySpeed == (int)m_dPlaySpeed)
    {
        playSpeedStr += ".00";
    }
    else
    {
        playSpeedStr += "";
    }
    ui->playSpeedlabel->setText(playSpeedStr);
    CMP_SetPlaySpeed(m_cmpHandle,m_dPlaySpeed);
//    setPlayButtonStyleSheet();

}

void recordManage::StopBtnClicked()
{
    if (m_cmpHandle != NULL)    //如果播放窗口已经有打开了码流播放，关闭码流播放
    {
        closePlayWin();
//        setPlayButtonStyleSheet();

    }

}

void recordManage::PauesBtnClicked()
{
    if (m_cmpHandle != NULL)
    {
        if(m_iPlayFlag == 1)
        {
            m_iPlayFlag = 0;
            CMP_PauseMedia(m_cmpHandle);
        }
    }

}

void recordManage::PlayBtnClicked()
{
    recordPlayFlag = 1;
    if (m_cmpHandle != NULL)
    {
       if (0 == m_iPlayFlag)
       {
           m_iPlayFlag = 1;
           CMP_PlayMedia(m_cmpHandle);
    //            CMP_SetPlayRate(m_cmpHandle,m_dPlaySpeed);
       }
    //        else
    //        {
    //            m_iPlayFlag = 0;
    //            CMP_PauseMedia(m_cmpHandle);
    //        }
    //           setPlayButtonStyleSheet();
    }
    else
    {
       if(ui->recordFileTableWidget->currentItem() != NULL)
       {
           emit recordSeletPlay(ui->recordFileTableWidget->currentItem());
       }
    }

}

void recordManage::SlowBtnClicked()
{
    QString playSpeedStr;
    if (NULL == m_cmpHandle)
    {
       return;
    }

    if (m_dPlaySpeed <= 0.25)
    {
       return;
    }
    m_iPlayFlag = 1;
    m_dPlaySpeed = m_dPlaySpeed / 2;
    playSpeedStr = QString::number(m_dPlaySpeed);
    if (m_dPlaySpeed == (int)m_dPlaySpeed)
    {
        playSpeedStr += ".00";
    }
    else
    {
        playSpeedStr += "";
    }
    ui->playSpeedlabel->setText(playSpeedStr);
    CMP_SetPlaySpeed(m_cmpHandle,m_dPlaySpeed);

//    setPlayButtonStyleSheet();


}

void recordManage::PrevBtnClicked()
{
    int iRow = 0, iDex = 0;

    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget lastOne play PushButton pressed!\n");

    if (ui->recordFileTableWidget->rowCount() <= 0 || NULL == m_cmpHandle)
    {
       return;
    }

    iDex = ui->carSeletionComboBox->currentIndex();
    if (iDex < 0)
    {
       return;
    }

    iRow = m_iRecordIdex - 1;
    if (iRow < 0)
    {
       return;
    }

    closePlayWin();   //先关闭之前的
//    setPlayButtonStyleSheet();
//    emit setRecordPlayFlagSignal(1);

    recordPlayCtrl(iRow, iDex);
    ui->playNextOnePushButton->setEnabled(false);
    ui->playLastOnePushButton->setEnabled(false);

    if(m_VideoSwitchTimer == NULL)
    {
       m_VideoSwitchTimer = new QTimer(this);
       m_VideoSwitchTimer->start(1*1000);
       connect(m_VideoSwitchTimer,SIGNAL(timeout()), this,SLOT(manualSwitchVideoEndSlot()));
    }

}

void recordManage::DownBtnClicked()
{
    int iRet = 0, idex = 0, row = 0;
    QString filename = "";
    QString fileSavePath = "/mnt/usb/u/";
    char acIpAddr[32] = {0};
    char acSaveFileName[128] = {0};

    if (ui->recordFileTableWidget->rowCount() > 0)
    {
       for (row = 0; row < ui->recordFileTableWidget->rowCount(); row++)    //先判断一次是否没有录像文件被选中，没有则弹框提示
       {

           if(QWidget *w = ui->recordFileTableWidget->cellWidget(row,0))
           {
               QCheckBox *checkBox = (QCheckBox*)(w->children().at(0));

               if(checkBox->checkState() == Qt::Checked)
               {
                  break;
               }
           }
       }

       if (row == ui->recordFileTableWidget->rowCount())
       {
           DebugPrint(DEBUG_UI_MESSAGE_PRINT, "recordPlayWidget not select record file to download!\n");
           static QMessageBox msgBox(QMessageBox::Question,QString(tr("注意")),QString(tr("请选择您要下载的录像文件!")));
           msgBox.setWindowFlags(Qt::FramelessWindowHint);
           msgBox.setStandardButtons(QMessageBox::Yes);
           msgBox.button(QMessageBox::Yes)->setText("OK");
           msgBox.exec();

           return;
       }

       if (access("/home/data/u/", F_OK) < 0)
       {
           DebugPrint(DEBUG_UI_MESSAGE_PRINT, "recordPlayWidget not get USB device!\n");
           static QMessageBox msgBox(QMessageBox::Warning,QString(tr("注意")),QString(tr("未检测到U盘,请插入!")));
           msgBox.setWindowFlags(Qt::FramelessWindowHint);
           msgBox.setStandardButtons(QMessageBox::Yes);
           msgBox.button(QMessageBox::Yes)->setText("OK");
           msgBox.exec();

           return;
       }
       else
       {
           if (0 == STATE_FindUsbDev())   //这里处理一个特殊情况:U盘拔掉是umount失败，/mnt/usb/u/路径还存在，但是实际U盘是没有再插上的
           {
               DebugPrint(DEBUG_UI_MESSAGE_PRINT, "recordPlayWidget not get USB device!\n");
               static QMessageBox msgBox(QMessageBox::Warning,QString(tr("注意")),QString(tr("未检测到U盘,请插入!")));
               msgBox.setWindowFlags(Qt::FramelessWindowHint);
               msgBox.setStandardButtons(QMessageBox::Yes);
               msgBox.button(QMessageBox::Yes)->setText("OK");
               msgBox.exec();

               return;
           }
       }

       qDebug()<<"****************"<<__FUNCTION__<<__LINE__;
       idex = ui->carSeletionComboBox->currentIndex();

       if (idex < 0)
       {
           return;
       }
       qDebug()<<"****************"<<__FUNCTION__<<__LINE__;

       m_iFtpServerIdex = idex;

    //           memset(&tTrainConfigInfo, 0, sizeof(T_TRAIN_CONFIG));
    //           STATE_GetCurrentTrainConfigInfo(&tTrainConfigInfo);
    //           snprintf(acIpAddr, sizeof(acIpAddr), "192.168.%d.81", 100+tTrainConfigInfo.tNvrServerInfo[idex].iCarriageNO);

        snprintf(acIpAddr, sizeof(acIpAddr), "192.168.%d.81", 104);
       if(m_tFtpHandle[idex] == 0)
       {
           m_tFtpHandle[idex] = FTP_CreateConnect(acIpAddr, FTP_SERVER_PORT, PftpProc);
       }
       else
       {
           return;
       }
       qDebug()<<"****************"<<__FUNCTION__<<__LINE__;

       if (0 == m_tFtpHandle[idex])
       {
    //               DebugPrint(DEBUG_UI_ERROR_PRINT, "[%s] connect to ftp server:%s error!\n", __FUNCTION__, acIpAddr);
           return;
       }
       qDebug()<<"****************"<<__FUNCTION__<<__LINE__;

       qDebug()<<"*****************m_tFtpHandle[idex]="<<m_tFtpHandle[idex]<<"************idex="<<idex<<__LINE__;
       memset(acSaveFileName,0,sizeof (acSaveFileName));
       qDebug()<<"**************acSaveFileName"<<acSaveFileName<<__FUNCTION__<<__LINE__;
       for (row = 0; row < ui->recordFileTableWidget->rowCount(); row++)
       {

           if(QWidget *w = ui->recordFileTableWidget->cellWidget(row,0))
           {
               QCheckBox *checkBox = (QCheckBox*)(w->children().at(0));
               if(checkBox->checkState() == Qt::Checked)
               {
                   if (parseFileName(m_acFilePath[row]) != NULL)
                   {
                       snprintf(acSaveFileName, sizeof(acSaveFileName), "%s%s", "/home/data/u/", parseFileName(m_acFilePath[row]));
                   }

                   DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] add download file:%s!\n", __FUNCTION__, m_acFilePath[row]);
                   iRet = FTP_AddDownLoadFile(m_tFtpHandle[idex], m_acFilePath[row], acSaveFileName);
                   if (iRet < 0)
                   {
                       FTP_DestoryConnect(m_tFtpHandle[m_iFtpServerIdex]);
                       m_tFtpHandle[m_iFtpServerIdex] = 0;
                       DebugPrint(DEBUG_UI_MESSAGE_PRINT, "recordPlayWidget not get USB device!\n");
                       static QMessageBox msgBox(QMessageBox::Warning,QString(tr("提示")),QString(tr("文件下载失败!")));
                       msgBox.setWindowFlags(Qt::FramelessWindowHint);
                       msgBox.setStandardButtons(QMessageBox::Yes);
                       msgBox.button(QMessageBox::Yes)->setText("OK");
                       msgBox.exec();

                       return;
                   }

               }

           }
       }

       iRet = FTP_FileDownLoad(m_tFtpHandle[idex]);
       if (iRet < 0)
       {
           FTP_DestoryConnect(m_tFtpHandle[m_iFtpServerIdex]);
           m_tFtpHandle[m_iFtpServerIdex] = 0;
           DebugPrint(DEBUG_UI_MESSAGE_PRINT, "recordPlayWidget record file download failed!\n");
           static QMessageBox msgBox(QMessageBox::Warning,QString(tr("提示")),QString(tr("文件下载失败")));
           msgBox.setWindowFlags(Qt::FramelessWindowHint);
           msgBox.setStandardButtons(QMessageBox::Yes);
           msgBox.button(QMessageBox::Yes)->setText("OK");
           msgBox.exec();

           return;
       }
    }

}


void recordManage::recordPlaySlot(QTableWidgetItem *item)
{
    int iRow = 0, iDex = 0;
    closePlayWin();   //先关闭之前的

//    emit setRecordPlayFlagSignal(1);
    iRow = item->row();

    ui->recordFileTableWidget->setEnabled(false);
    if(m_tableVideoSwitchTimer == NULL)
    {
        m_tableVideoSwitchTimer = new QTimer(this);
        m_tableVideoSwitchTimer->start(1*1000);
        connect(m_tableVideoSwitchTimer,SIGNAL(timeout()), this,SLOT(manualtableSwitchVideoEndSlot()));
    }

    recordPlayFlag = 0;

    iDex = ui->carSeletionComboBox->currentIndex();

    recordPlayCtrl(iRow, iDex);


}

void recordManage::recordSelectionSlot(QTableWidgetItem *item)
{
    int i = 0;
    for (i = 0; i < ui->recordFileTableWidget->rowCount(); i++)
    {
        if (0 == ui->recordFileTableWidget->item(i, 2)->text().contains("tmp") && i != m_iRecordIdex)
        {
            if (i == item->row())
            {
//                g_fistSelctFlag = item->row();

                ui->recordFileTableWidget->item(i, 1)->setTextColor(Qt::green);
                ui->recordFileTableWidget->item(i, 2)->setForeground(Qt::green);
            }
            else
            {
                ui->recordFileTableWidget->item(i, 1)->setTextColor(Qt::black);
                ui->recordFileTableWidget->item(i, 2)->setForeground(Qt::black);
            }
//            setPlayButtonStyleSheet();

        }
    }


}
void recordManage::closePlayWin()
{

    if (m_threadId != 0)
    {
        m_iThreadRunFlag = 0;
        pthread_join(m_threadId, NULL);
        m_threadId = 0;
    }

    m_playSlider->setRange(0, 0);
    m_playSlider->setValue(0);

    if (m_cmpHandle != NULL)    //关闭已打开的回放
    {
        CMP_SetPlayEnable(m_cmpHandle, 0);
        CMP_CloseMedia(m_cmpHandle);
        CMP_UnInit(m_cmpHandle);

        m_cmpHandle= NULL;
//        emit setRecordPlayFlagSignal(0);
    }

    if (m_iRecordIdex >= 0 && ui->recordFileTableWidget->item(m_iRecordIdex, 2) != NULL && 0 == ui->recordFileTableWidget->item(m_iRecordIdex, 2)->text().contains("tmp"))
    {
        ui->recordFileTableWidget->item(m_iRecordIdex, 1)->setForeground(Qt::cyan);
        ui->recordFileTableWidget->item(m_iRecordIdex, 2)->setForeground(Qt::cyan);
    }
    m_iRecordIdex = -1;
    m_iPlayFlag = 0;

    ui->playMinLabel->setText("00");
    ui->playSecLabel->setText("00");
    ui->rangeMinLabel->setText("00");
    ui->rangeSecLabel->setText("00");


    playWidget->hide();
    playWidget->show();
}


void recordManage::recordTableWidgetFillFunc()
{
    char *pcfileName = NULL;
    char *pcToken = m_pcRecordFileBuf, *pcBufTmp = NULL;
    char acFilePath[MAX_RECFILE_PATH_LEN] = {0};
    int iParseIdex = 0;
    QString item = "";
    qDebug()<<"****************"<<__FUNCTION__<<__LINE__;
    pcBufTmp = strstr(pcToken,".MP4");
    while (pcBufTmp != NULL)
    {
        memset(acFilePath, 0, MAX_RECFILE_PATH_LEN);
        memcpy(acFilePath, pcToken, pcBufTmp-pcToken);
        strcat(acFilePath, ".MP4");
        memcpy(m_acFilePath[iParseIdex], acFilePath, strlen(acFilePath));
        iParseIdex++;
        if (iParseIdex > MAX_RECORD_SEACH_NUM)
        {
            break;
        }

        ui->recordFileTableWidget->insertRow(iParseIdex-1);//添加新的一行


        QWidget *checkWidget= new QWidget(this); //创建一个widget

        QCheckBox *checkBox = new QCheckBox(checkWidget);
        checkBox->setChecked(Qt::Unchecked);
        QHBoxLayout *hLayout = new QHBoxLayout(); //创建布局
        hLayout->addWidget(checkBox); //添加checkbox
        hLayout->setMargin(0); //设置边缘距离 否则会很难看
        hLayout->setAlignment(checkBox, Qt::AlignCenter); //居中
        checkWidget->setLayout(hLayout); //设置widget的布局

        checkBox->setStyleSheet(QString("QCheckBox {margin:3px;border:3px; border-color: rgb(170, 170, 170);border-width: 2px;border-style: solid}QCheckBox::indicator {width: %1px; height: %1px;}").arg(30));
        ui->recordFileTableWidget->setCellWidget(iParseIdex-1, 0, checkWidget);


        item = QString::number(iParseIdex);
        ui->recordFileTableWidget->setItem(iParseIdex-1, 1, new QTableWidgetItem(item));
        ui->recordFileTableWidget->item(iParseIdex-1, 1)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        pcfileName = parseFileName(acFilePath);
        if (pcfileName != NULL)
        {
            item = QString(QLatin1String(pcfileName));
        }
        ui->recordFileTableWidget->setItem(iParseIdex-1, 2, new QTableWidgetItem(item));
        ui->recordFileTableWidget->item(iParseIdex-1, 2)->setTextAlignment(Qt::AlignHCenter|Qt::AlignVCenter);

        if (pcfileName != NULL && strstr(pcfileName, "_tmp") != NULL)
        {
            ui->recordFileTableWidget->item(iParseIdex-1, 1)->setForeground(Qt::red);
            ui->recordFileTableWidget->item(iParseIdex-1, 2)->setForeground(Qt::red);
        }
        else
        {
            ui->recordFileTableWidget->item(iParseIdex-1, 1)->setForeground(Qt::black);
            ui->recordFileTableWidget->item(iParseIdex-1, 2)->setForeground(Qt::black);
        }

        pcToken = pcBufTmp + strlen(".MP4");
        pcBufTmp = strstr(pcToken,".MP4");
    }

    ui->searchloadButton->setEnabled(true);
    ui->carSeletionComboBox->setEnabled(true);
    ui->cameraSelectionComboBox->setEnabled(true);
    ui->videoComboBox->setEnabled(true);

    if (m_recordTabelWidgetFillTimer != NULL)
    {
        if (m_recordTabelWidgetFillTimer->isActive())   //判断定时器是否正在运行，是则停止运行
        {
            m_recordTabelWidgetFillTimer->stop();
        }
        delete m_recordTabelWidgetFillTimer;
        m_recordTabelWidgetFillTimer = NULL;
    }


}

void recordManage::recordTableWidgetFillSlot()
{
    if (NULL == m_recordTabelWidgetFillTimer)
    {
        m_recordTabelWidgetFillTimer = new QTimer(this);
    }
    m_recordTabelWidgetFillTimer->start(1000);   //收到第一包后等待1秒，确保录像文件消息包能全部接收完再一起填充文件列表处理
    connect(m_recordTabelWidgetFillTimer,SIGNAL(timeout()), this, SLOT(recordTableWidgetFillFunc()));

}

void recordManage::recordQueryEndSlot()
{

    static int iRecordNum = 0;

    iRecordNum++;

    if (m_iTotalLen> 0)  /*收到第一个包触发录像文件列表填充信号来填充列表*/
    {
        emit recordTableWidgetFillSignal();

        if (FileSearchTimer != NULL)
        {
            if (FileSearchTimer->isActive())   //判断定时器是否正在运行，是则停止运行
            {
                FileSearchTimer->stop();
            }
            delete FileSearchTimer;
            FileSearchTimer = NULL;
        }
    }
    else
    {
        if (iRecordNum > 10)    //5秒没查询即恢复查询按键可按*/
        {
            iRecordNum = 0;

            ui->searchloadButton->setEnabled(true);
            ui->carSeletionComboBox->setEnabled(true);
            ui->cameraSelectionComboBox->setEnabled(true);
            ui->videoComboBox->setEnabled(true);

            if (FileSearchTimer != NULL)
            {
                if (FileSearchTimer->isActive())   //判断定时器是否正在运行，是则停止运行
                {
                    FileSearchTimer->stop();
                }
                delete FileSearchTimer;
                FileSearchTimer = NULL;
            }


            DebugPrint(DEBUG_UI_MESSAGE_PRINT, "[%s-%d] recordQuery fail!\n",__FUNCTION__, __LINE__);
            static QMessageBox box(QMessageBox::Warning,QString::fromUtf8("错误"),QString::fromUtf8("未查询到录像数据!"));
            box.setWindowFlags(Qt::FramelessWindowHint);
            box.setStandardButtons (QMessageBox::Ok);
            box.setButtonText (QMessageBox::Ok,QString::fromUtf8("OK"));
            box.exec();
        }
    }


}

void recordManage::SearchBtnClicked()
{
    E_FILE_SEARCH_STATE eState;
    int iRet = 0, row = 0, iServerIdex = 0, iCameraIdex = 0, i = 0;
    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;
    memset(m_pcRecordFileBuf, 0, MAX_RECORD_SEACH_NUM*MAX_RECFILE_PATH_LEN);
    m_iTotalLen = 0;

    for (i = 0; i < MAX_RECORD_SEACH_NUM; i++)
    {
        memset(m_acFilePath[i], 0, MAX_RECFILE_PATH_LEN);
    }

    int iNvrNo = ui->carSeletionComboBox->currentIndex();
    int iIpcPos    = ui->cameraSelectionComboBox->currentIndex();
    int iVideoType = ui->videoComboBox->currentIndex() +1;
    T_LOG_INFO tLogInfo;


    int iCmd = CLI_SERV_MSG_TYPE_GET_RECORD_FILE;
    int iDataLen = sizeof(T_NVR_SEARCH_RECORD);
    int iVideoIdx = -1;
    char acCmdData[96]={0};
    T_NVR_SEARCH_RECORD *pSchFile = NULL;
    int iDiscTime = 0;

    sscanf(ui->startTimeLabel->text().toLatin1().data(), "%4d-%2d-%2d %2d:%2d:%2d", &year, &mon, &day, &hour, &min, &sec);
    m_tStartTime.year = year;
    m_tStartTime.mon = mon;
    m_tStartTime.day = day;
    m_tStartTime.hour = hour;
    m_tStartTime.min = min;
    m_tStartTime.sec = sec;


    sscanf(ui->endTimeLabel->text().toLatin1().data(), "%4d-%2d-%2d %2d:%2d:%2d", &year, &mon, &day, &hour, &min, &sec);
    m_tEndTime.year = year;
    m_tEndTime.mon = mon;
    m_tEndTime.day = day;
    m_tEndTime.hour = hour;
    m_tEndTime.min = min;
    m_tEndTime.sec = sec;

    iDiscTime = (m_tEndTime.year - m_tStartTime.year)*366*24*3600
        +(m_tEndTime.mon - m_tStartTime.mon)*30*24*3600
        +(m_tEndTime.day - m_tStartTime.day)*24*3600
        +(m_tEndTime.hour - m_tStartTime.hour)*3600
        +(m_tEndTime.min - m_tStartTime.min)*3600
        +(m_tEndTime.sec - m_tStartTime.sec)*3600;


    if(iDiscTime <= 0)
    {
        static QMessageBox box(QMessageBox::Warning,QString::fromUtf8("warning"),QString::fromUtf8("开始时间不能大于结束时间!"));
        box.setWindowFlags(Qt::FramelessWindowHint);
        box.setStandardButtons (QMessageBox::Ok);
        box.setButtonText (QMessageBox::Ok,QString::fromUtf8("OK"));
        box.exec();
        return;
    }
    m_Phandle[iNvrNo] = STATE_GetNvrServerPmsgHandle(iNvrNo);
    qDebug()<<"********m_Phandle[iNvrNo]="<<m_Phandle[iNvrNo]<<"****iNvrNo="<<iNvrNo<<"iIpcPos"<<iIpcPos<<__LINE__;
    if (m_Phandle[iNvrNo])
    {
        T_NVR_SEARCH_RECORD tRecordSeach;
        memset(&tRecordSeach, 0, sizeof(T_NVR_SEARCH_RECORD));

        tRecordSeach.tStartTime.i16Year =  htons(m_tStartTime.year);
        tRecordSeach.tStartTime.i8Mon   =  m_tStartTime.mon;
        tRecordSeach.tStartTime.i8day   =  m_tStartTime.day;
        tRecordSeach.tStartTime.i8Hour  =  m_tStartTime.hour;
        tRecordSeach.tStartTime.i8Min   =  m_tStartTime.min;
        tRecordSeach.tStartTime.i8Sec   =  m_tStartTime.sec;

        tRecordSeach.tEndTime.i16Year   =  htons(m_tEndTime.year);
        tRecordSeach.tEndTime.i8Mon     =  m_tEndTime.mon;
        tRecordSeach.tEndTime.i8day     =  m_tEndTime.day;
        tRecordSeach.tEndTime.i8Hour    =  m_tEndTime.hour;
        tRecordSeach.tEndTime.i8Min     =  m_tEndTime.min;
        tRecordSeach.tEndTime.i8Sec     =  m_tEndTime.sec;

        tRecordSeach.iCarriageNo =iNvrNo;
        tRecordSeach.iIpcPos = 8+iIpcPos;


        iRet = PMSG_SendPmsgData(m_Phandle[iNvrNo], CLI_SERV_MSG_TYPE_GET_RECORD_FILE, (char *)&tRecordSeach, sizeof(T_NVR_SEARCH_RECORD));
        if (iRet < 0)
        {
            qDebug()<<"PMSG_SendPmsgData CLI_SERV_MSG_TYPE_GET_RECORD_FILE error!"<<__FUNCTION__<<__LINE__<<endl;
        }
        else
        {

        }

        ui->searchloadButton->setEnabled(false);
        ui->carSeletionComboBox->setEnabled(false);
        ui->cameraSelectionComboBox->setEnabled(false);
        ui->videoComboBox->setEnabled(false);

        if(FileSearchTimer == NULL)
        {
            FileSearchTimer = new QTimer(this);

        }
        FileSearchTimer->start(500);
        connect(FileSearchTimer,SIGNAL(timeout()), this,SLOT(recordQueryEndSlot()));

    }


}


void recordManage::carNoChangeSlot()
{
    int i = 0, j =0,idex = ui->carSeletionComboBox->currentIndex();    //获取当前车厢选择下拉框的索引
    QString item = "";
    ui->cameraSelectionComboBox->setCurrentIndex(-1);
    ui->cameraSelectionComboBox->clear();

    if(idex ==0 || idex == 5)
    {
        for(i = 0; i < 8; i++)
        {
            item = "";
            item = QString::number(idex+1);
            if(i == 0)
                item += tr("F");
            else if(i == 1)
                item += tr("G");
            else if(i == 2)
                item += tr("E");
            else if(i == 3)
            {
                item = "";
                item += tr("G");
                item += QString::number(idex+1);
            }
            else if(i == 4)
                item += tr("A");
            else if(i == 5)
                item += tr("B");
            else if(i == 6)
                item += tr("C");
            else
                item += tr("D");
            ui->cameraSelectionComboBox->addItem(item);
        }

    }
    else if ((idex > 0) && ( idex < 5))
    {
        for(i = 0; i < 4; i++)
        {
            item = "";
            item = QString::number(idex+1);

            if(i == 0)
                item += tr("A");
            else if(i == 1)
                item += tr("B");
            else if(i == 2)
                item += tr("C");
            else
                item += tr("D");
            ui->cameraSelectionComboBox->addItem(item);
        }

    }

}

void recordManage::getTrainConfig()
{
    int i = 0, j = 0;
    QString item = "";

    for(i = 0; i < 6; i++)
    {
        item = "";
        item = QString::number(i+1);
        item +=tr("车");
        ui->carSeletionComboBox->addItem(item);
//        m_Phandle[i] = STATE_GetNvrServerPmsgHandle(i);
//        qDebug()<<"**********m_Phandle[i]"<<m_Phandle[i]<<__func__<<__LINE__;
    }

    for(i = 0; i < 8; i++)
    {
        item = "";
        item = QString::number(j+1);
        if(i == 0)
            item += tr("F");
        else if(i == 1)
            item += tr("G");
        else if(i == 2)
            item += tr("E");
        else if(i == 3)
        {
            item = "";
            item += tr("G");
            item += QString::number(j+1);
        }
        else if(i == 4)
            item += tr("A");
        else if(i == 5)
            item += tr("B");
        else if(i == 6)
            item += tr("C");
        else
            item += tr("D");
        ui->cameraSelectionComboBox->addItem(item);
    }

    for(i = 0; i < 4; i++)
    {
        item = "";
        if(i == 0)
        {
            item += tr("所有录像");
            ui->videoComboBox->addItem(item);
        }
        else if(i == 1)
        {
            item += tr("普通录像");
            ui->videoComboBox->addItem(item);
        }
        else if(i == 2)
        {
            item += tr("报警录像");
            ui->videoComboBox->addItem(item);
        }
        else
        {
            item += tr("紧急录像");
            ui->videoComboBox->addItem(item);
        }


    }


}

void recordManage::hideRecPageSlots()
{
    closePlayWin();   //先关闭之前的
    this->hide();
    emit hideRecSysPage();
}

void recordManage::openStartTimeSetWidgetSlot()
{
    QString timeStr = ui->startTimeLabel->text();
    char acTimeStr[256] = {0};
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget startTimeSetPushButton pressed!\n");
    strcpy(acTimeStr, timeStr.toLatin1().data());
    if (strlen(acTimeStr) != 0)
    {
        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] timeStr:%s!\n", __FUNCTION__, acTimeStr);
        sscanf(acTimeStr, "%4d-%02d-%02d %02d:%02d:%02d", &iYear, &iMonth, &iDay, &iHour, &iMin, &iSec);
        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] %d-%d-%d %d:%d:%d!\n", __FUNCTION__, iYear, iMonth, iDay, iHour, iMin, iSec);
    }
    timeSetWidget->setGeometry(312, 112, timeSetWidget->width(), timeSetWidget->height());
    g_iDateEditNo = 1;
    timeSetWidget->setTimeLabelText(iYear, iMonth, iDay, iHour, iMin, iSec);
    timeSetWidget->show();

}

void recordManage::openStopTimeSetWidgetSlot()
{
    QString timeStr = ui->endTimeLabel->text();
    char acTimeStr[256] = {0};
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget stopTimeSetPushButton pressed!\n");
    strcpy(acTimeStr, timeStr.toLatin1().data());
    if (strlen(acTimeStr) != 0)
    {
        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] timeStr:%s!\n", __FUNCTION__, acTimeStr);
        sscanf(acTimeStr, "%4d-%02d-%02d %02d:%02d:%02d", &iYear, &iMonth, &iDay, &iHour, &iMin, &iSec);
        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] %d-%d-%d %d:%d:%d!\n", __FUNCTION__, iYear, iMonth, iDay, iHour, iMin, iSec);
    }
    timeSetWidget->setGeometry(312, 150, timeSetWidget->width(), timeSetWidget->height());
    g_iDateEditNo = 2;
    timeSetWidget->setTimeLabelText(iYear, iMonth, iDay, iHour, iMin, iSec);
    timeSetWidget->show();

}

void recordManage::timeSetRecvMsg(QString year, QString month, QString day, QString hour, QString min, QString sec)
{
    char timestr[128] = {0};
    snprintf(timestr, sizeof(timestr), "%s-%s-%s %s:%s:%s", year.toStdString().data(), month.toStdString().data(), day.toStdString().data(),
            hour.toStdString().data(), min.toStdString().data(), sec.toStdString().data());
    QString string = QString(QLatin1String(timestr)) ;
    if (1 == g_iDateEditNo)
    {
        ui->startTimeLabel->setText(string);
    }
    else if (2 == g_iDateEditNo)
    {
        ui->endTimeLabel->setText(string);
    }

}


