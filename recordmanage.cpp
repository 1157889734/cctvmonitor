#include "recordmanage.h"
#include "ui_recordmanage.h"
#include "debugout/debug.h"
#include "state/state.h"
#include <QDebug>
#include <QMessageBox>
#include "NVRMsgProc.h"
#include "stdio.h"
#include <time.h>
#include "unistd.h"
#include "ftp/ftpApi.h"


#define FTP_SERVER_PORT  21   //FTP服务器默认通信端?

static PFTP_HANDLE g_ftpHandle = 0;

int g_iDateEditNo = 0;      //要显示时间的不同控件的编号

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
    ui->recordFileTableWidget->horizontalHeader()->resizeSection(0,46); ////设置表头第一列的宽度为46
    ui->recordFileTableWidget->horizontalHeader()->resizeSection(1,46);
    ui->recordFileTableWidget->horizontalHeader()->resizeSection(2,219);

    FileSearchTimer = NULL;


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

    timeSetWidget = new timeset(this);
    timeSetWidget->setWindowFlags(timeSetWidget->windowFlags() | Qt::FramelessWindowHint| Qt::Dialog);
    timeSetWidget->hide();

    connect(timeSetWidget, SIGNAL(timeSetSendMsg(QString,QString,QString,QString,QString,QString)), this, SLOT(timeSetRecvMsg(QString,QString,QString,QString,QString,QString)));  //时间设置窗体控件设置信号响应
    connect(ui->startTimeLabel, SIGNAL(clicked()), this, SLOT(openStartTimeSetWidgetSlot()));   //开始时间设置按钮按键信号响应
    connect(ui->endTimeLabel, SIGNAL(clicked()), this, SLOT(openStopTimeSetWidgetSlot()));  //结束时间设置按钮按键信号响应

    getTrainConfig();

    connect(ui->carSeletionComboBox, SIGNAL(currentIndexChanged(int)),this,SLOT(carNoChangeSlot()));
    connect(ui->searchloadButton,SIGNAL(clicked()),this,SLOT(SearchBtnClicked()));
    connect(ui->downloadButton,SIGNAL(clicked()),this,SLOT(DownBtnClicked()));


}

recordManage::~recordManage()
{
    delete ui;
}

void PftpProc(PFTP_HANDLE PHandle, int iPos)
{
    if(100 == iPos)
    {
        SetFileDownProgress(100);//iPos=100,表示下载完毕。
        SetFileDownState(E_FILE_DOWN_SUCC);
    }
    else if(iPos <0)//暂定iPos=-1表示被告知U盘已拔出, iPos=-2表示被告知U盘写入失败,iPos=-3表示被告知数据接收失败失败。 三种情况都隐藏进度条，并在信号处理函数中销毁FTP连接
    {
        SetFileDownProgress(100);
        SetFileDownState(E_FILE_DOWN_FAIL);
    }
    else
    {
        SetFileDownProgress(iPos);
    }
}


void recordManage::DownBtnClicked()
{
    int iRet = 0, row = 0;
    char acFullFileName[256] = {0};
    char acSaveFileName[256] = {0};
    char acIpAddr[32] = {0};

    int iSize = ui->recordFileTableWidget->rowCount();
    int iState = GetFileDownState();

    if ((access(USB_PATH, F_OK) < 0) || (0 == STATE_FindUsbDev()))
    {
        QMessageBox::warning(NULL, "warning", "请插入U盘", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }

    if(E_FILE_DOWNING == iState)
    {
        QMessageBox::warning(NULL, "warning", "正在下载,请稍后!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
//        pMyVidWid->m_pBoxMessage->copy_label("正在下载,请稍后!");
//        pMyVidWid->m_pBoxMessage->redraw();
        return;
    }

    if(m_iNvrNo <0 || iSize <=0)
    {
        QMessageBox::warning(NULL, "warning", "无可下载文件!", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

//        pMyVidWid->m_pBoxMessage->copy_label("无可下载文件");
//        pMyVidWid->m_pBoxMessage->redraw();
        return ;
    }


    if (iSize > 0)
    {
        for (row = 0; row <iSize; row++)	 //先判断一次是否没有录像文件被选中，没有则弹框提示
        {
//            if (pMyVidWid->m_pFilelsTable->GetFileSelectState(row))
//            {
//                break;
//            }
        }
        if (row == iSize)
        {
            QMessageBox::warning(NULL, "warning", "请选择相应下载视频", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
//            pMyVidWid->m_pBoxMessage->copy_label("请选择相应下载视频");
//            pMyVidWid->m_pBoxMessage->redraw();
            return;
        }
        GetNvrIpAddr(m_iNvrNo,acIpAddr);
        g_ftpHandle = FTP_CreateConnect(acIpAddr, FTP_SERVER_PORT, PftpProc);
        if (0 == g_ftpHandle)
        {
            QMessageBox::warning(NULL, "warning", "下载连接失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

//            pMyVidWid->m_pBoxMessage->copy_label("下载连接失败");
//            pMyVidWid->m_pBoxMessage->redraw();
            DebugPrint(DEBUG_FTP_PRINT, "[%s] connect to ftp server:%s error!\n", __FUNCTION__, acIpAddr);
            return;
        }
        for (row = 0; row < iSize; row++)
        {
//            if (pTable->GetFileSelectState(row))
//            {
//                memset(acFullFileName,0,sizeof(acFullFileName));
//                strcpy(acFullFileName,pTable->m_FileString.at(row).c_str());
//                if (parseFileName(acFullFileName) != NULL)
//                {
//                    snprintf(acSaveFileName, sizeof(acSaveFileName),USB_PATH"%s"
//                        , parseFileName(acFullFileName));
//                    DebugPrint(DEBUG_FTP_PRINT, "[%s] add download file:%s!\n", __FUNCTION__, acSaveFileName);
//                    iRet = FTP_AddDownLoadFile(g_ftpHandle,acFullFileName, acSaveFileName);
//                    if (iRet < 0)
//                    {
//                        FTP_DestoryConnect(g_ftpHandle);
//                        pMyVidWid->m_pBoxMessage->copy_label("文件下载失败");
//                        pMyVidWid->m_pBoxMessage->redraw();
//                        return;
//                    }
//                }
//            }
        }
        iRet = FTP_FileDownLoad(g_ftpHandle);
        if (iRet < 0)
        {
            FTP_DestoryConnect(g_ftpHandle);
//            pMyVidWid->m_pBoxMessage->copy_label("文件下载失败");
//            pMyVidWid->m_pBoxMessage->redraw();
            return;
        }
        SetFileDownState(E_FILE_DOWNING);

//        pMyVidWid->m_pBtnOper[1]->deactivate();
//        pMyVidWid->m_pBtnProgress[0]->show();
//        pMyVidWid->m_pBtnProgress[1]->show();
//        Fl::add_timeout(0.2,CheckDownProcessFun,pData);
//        pMyVidWid->m_pBoxMessage->copy_label("文件正在下载");
//        pMyVidWid->m_pBoxMessage->redraw();
    }
    else
    {
//        pMyVidWid->m_pBoxMessage->copy_label("无可下载文件");
//        pMyVidWid->m_pBoxMessage->redraw();
    }

}

void recordManage::recordQueryEndSlot()
{
    int iNvrNo = m_iNvrNo;
    m_iWaitFileCnt ++;
    if(m_iWaitFileCnt < 50)
    {
        int iDataLen = 0;
        char *pBefore = NULL;
        char *pAfter = NULL;
        int iFirst = 1;
        int iFind = 0;
        T_CMD_PACKET tPkt;
        int iLeaveDataLen = 0;
        char acData[2048] = {0};
        char acData_BK[2048] = {0};

        tPkt.iDataLen =0;
        tPkt.pData = NULL;

        while(NVR_GetFileInfo(iNvrNo, &tPkt))
        {
            if(iFirst)
            {
                iFind = 1;
                iFirst = 0;
                sleep(1);
            }

            if(tPkt.iDataLen + iLeaveDataLen < 2048)
            {
                memcpy(acData+iLeaveDataLen,tPkt.pData,tPkt.iDataLen);
                iDataLen = tPkt.iDataLen + iLeaveDataLen;
            }
            else
            {
                memcpy(acData,tPkt.pData,tPkt.iDataLen);
                iDataLen = tPkt.iDataLen + iLeaveDataLen;
            }
            iLeaveDataLen = iDataLen;

            pBefore = acData;
            pAfter = pBefore;
            while (*pAfter != 0 && iLeaveDataLen >0)
            {
//                string strFile;

                pAfter= strstr(pBefore,".avi");
                if(pAfter)
                {
//                    strFile.append(pBefore,pAfter -pBefore +4);
//                    pTable->m_FileString.push_back(strFile);
                    iLeaveDataLen -= pAfter-pBefore +4;
                    pAfter += 4;
                    pBefore = pAfter;
                }
            }
            if (tPkt.pData)
            {
                free(tPkt.pData);
                tPkt.pData = NULL;
                tPkt.iDataLen = 0;
            }
            if(iLeaveDataLen >0)
            {
                while((iLeaveDataLen >0) && (0 == acData[iDataLen-iLeaveDataLen]))
                {
                    iLeaveDataLen--;
                }
                if(iLeaveDataLen > 0)
                {
                    memcpy(acData_BK,&acData[iDataLen-iLeaveDataLen],iLeaveDataLen);
                    memset(acData,0,sizeof(acData));
                    memcpy(acData,acData_BK,iLeaveDataLen);
                }
                else
                {
                    iLeaveDataLen = 0;
                }
            }
            else
            {
                iLeaveDataLen = 0;
            }


        }
        if(iFind)
        {

//            pTable->SetSize(pTable->m_FileString.size(),3);
//            pTable->col_width(0,40);
//            pTable->col_width(1,40);
//            if(pTable->m_FileString.size() >13)
//            {
//                pTable->col_width(2,245);
//            }
//            else
//            {
//                pTable->col_width(2,268);
//            }
//            pTable->redraw();
//            SetFileSearchState(E_FILE_IDLE);
//            pVideoWid->m_pBoxMessage->copy_label("文件搜索成功");
//            pVideoWid->m_pBoxMessage->redraw();
//            Fl::remove_timeout(FileSrhResultTimer,arg);
//            return;


//            if (FileSearchTimer != NULL)
//            {
//                if (FileSearchTimer->isActive())   //判断定时器是否正在运行，是则停止运行
//                {
//                    FileSearchTimer->stop();
//                }
//                delete FileSearchTimer;
//                FileSearchTimer = NULL;
//            }


        }


    }
    else
    {
//        if (FileSearchTimer != NULL)
//        {
//            if (FileSearchTimer->isActive())   //判断定时器是否正在运行，是则停止运行
//            {
//                FileSearchTimer->stop();
//            }
//            delete FileSearchTimer;
//            FileSearchTimer = NULL;
//        }

        QMessageBox::warning(NULL, "warning", "文件搜索失败", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
//        pVideoWid->m_pBoxMessage->copy_label("文件搜索失败");
//        pVideoWid->m_pBoxMessage->redraw();
        SetFileSearchState(E_FILE_IDLE);
//        Fl::remove_timeout(FileSrhResultTimer,arg);
    }



}

void recordManage::SearchBtnClicked()
{
    E_FILE_SEARCH_STATE eState;
    int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;

    int iNvrNo = ui->carSeletionComboBox->currentIndex() +1;
    int iIpcPos    = ui->cameraSelectionComboBox->currentIndex() +1;
    int iVideoType = ui->videoComboBox->currentIndex() +1;

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
        QMessageBox::warning(NULL, "warning", "开始时间不能大于结束时间", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    if(iNvrNo < 0 || iNvrNo > 5)
    {
        QMessageBox::warning(NULL, "warning", "请选择相应的车型号", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    if(iIpcPos < 0)
    {
        QMessageBox::warning(NULL, "warning", "请选择相应相机", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    if(iVideoType < 0)
    {
        QMessageBox::warning(NULL, "warning", "请选择录像类型", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    m_iNvrNo = iNvrNo;
    m_iWaitFileCnt = 0;

    pSchFile = (T_NVR_SEARCH_RECORD *)acCmdData;
    iVideoIdx = GetNvrVideoIdx(m_iNvrNo, iIpcPos);
    pSchFile->iCarriageNo = m_iNvrNo +1;
    pSchFile->cVideoType = iVideoType;
    pSchFile->iIpcPos = iVideoIdx+1;

    SetFileSearchState( E_FILE_SEARCHING);
    NVR_CleanFileInfo(iNvrNo);

    NVR_SendCmdInfo(iNvrNo, iCmd, acCmdData, iDataLen);

//    if(FileSearchTimer == NULL)
//    {
//        FileSearchTimer = new QTimer(this);

//    }
//    FileSearchTimer->start(200);
//    connect(FileSearchTimer,SIGNAL(timeout()), this,SLOT(recordQueryEndSlot()));


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
    }

    for(i = 0; i < 8; i++)
    {
        item = "";
        item = QString::number(i+1);
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
            item += QString::number(i+1);
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
    this->hide();
    emit hideRecSysPage();
}

void recordManage::openStartTimeSetWidgetSlot()
{
    QString timeStr = ui->startTimeLabel->text();
    char acTimeStr[256] = {0};
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

//    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget startTimeSetPushButton pressed!\n");
    strcpy(acTimeStr, timeStr.toLatin1().data());
    if (strlen(acTimeStr) != 0)
    {
//        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] timeStr:%s!\n", __FUNCTION__, acTimeStr);
        sscanf(acTimeStr, "%4d-%02d-%02d %02d:%02d:%02d", &iYear, &iMonth, &iDay, &iHour, &iMin, &iSec);
//        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] %d-%d-%d %d:%d:%d!\n", __FUNCTION__, iYear, iMonth, iDay, iHour, iMin, iSec);
    }
    timeSetWidget->setGeometry(420, 190, timeSetWidget->width(), timeSetWidget->height());
    g_iDateEditNo = 1;
    timeSetWidget->setTimeLabelText(iYear, iMonth, iDay, iHour, iMin, iSec);
    timeSetWidget->show();

}

void recordManage::openStopTimeSetWidgetSlot()
{
    QString timeStr = ui->endTimeLabel->text();
    char acTimeStr[256] = {0};
    int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0;

//    DebugPrint(DEBUG_UI_OPTION_PRINT, "recordPlayWidget stopTimeSetPushButton pressed!\n");
    strcpy(acTimeStr, timeStr.toLatin1().data());
    if (strlen(acTimeStr) != 0)
    {
//        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] timeStr:%s!\n", __FUNCTION__, acTimeStr);
        sscanf(acTimeStr, "%4d-%02d-%02d %02d:%02d:%02d", &iYear, &iMonth, &iDay, &iHour, &iMin, &iSec);
//        DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] %d-%d-%d %d:%d:%d!\n", __FUNCTION__, iYear, iMonth, iDay, iHour, iMin, iSec);
    }
    timeSetWidget->setGeometry(420, 230, timeSetWidget->width(), timeSetWidget->height());
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


