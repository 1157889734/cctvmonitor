#include "cctvtest.h"
#include "ui_cctvtest.h"
#include <QDate>
#include <QDateTime>
#include "state/state.h"
#include <pthread.h>
#include <stdio.h>
#include "unistd.h"
#include "NVRMsgProc.h"
#include <sys/time.h>
#include <QIcon>
#include "CMPlayer.h"


static E_PLAY_STYLE g_eCurPlayStyle = E_FOUR_VPLAY;   //当前播放风格
static E_PLAY_STYLE g_eNextPlayStyle = g_eCurPlayStyle;
static int	g_iWarn = 0;			//是否有报警
static int  g_iVideoCycleFlag = 1;  //轮询标志
static int  g_iNextSingleVideoIdx = -1;
static int g_iCycTime = 30;
static int  g_iWarnFreshed = 0;  //避免报警信息画面还未刷新，就被别的指令破坏

static CMPHandle	g_hSinglePlay = 0;

static int  g_aiCurFourVideoIdx[4] = {-1,-1,-1,-1};
static int  g_aiNextFourVideoIdx[4] = {-1,-1,-1,-1};
static int	g_iNeedUpdateWarnIcon = 0; //是否需要更新报警图标



void cctvTest::UpdateCamState()
{
    E_PLAY_STYLE eStyle = g_eCurPlayStyle;
    int iNVRState =0;
    int iNvrNo = 0;
    int iVideoNum = GetVideoNum();
    for(int i=0;i<iVideoNum;i++)
    {
        int iGroup= -1,iPos =-1;
        int iOnline = GetVideoOnlineState(i);
        int iShelterWarn = GetVideoWarnState(i);
        int iImgIndex = GetVideoImgIdx(i);
        int iNowPlay = 0;

        if(iImgIndex <1 || iImgIndex > 32)
        {
            continue;
        }
        GetBtnPoseAccordVideoIdx(i, &iGroup, &iPos);
        iNvrNo = GetVideoNvrNo(i);
        iNVRState = NVR_GetConnectStatus((iNvrNo/2)*2);
        if(E_SERV_STATUS_CONNECT != iNVRState)
        {
            iNVRState = NVR_GetConnectStatus((iNvrNo/2)*2+1);
        }
        if(E_SERV_STATUS_CONNECT != iNVRState )
        {
            iOnline = 0;
        }

        QIcon Nowicon = videoGroupBtn[iGroup][iPos]->icon();
        QIcon icon = Nowicon;

        if(E_SINGLE_VPLAY == eStyle )
        {
            if(i == g_iNextSingleVideoIdx)
            {
                iNowPlay = 1;
            }
        }

        else if(E_FOUR_VPLAY == eStyle)
        {
            if((g_iNextSingleVideoIdx >=0) && (i == g_iNextSingleVideoIdx))
            {
                iNowPlay = 1;
            }
            else if(g_iNextSingleVideoIdx <0)
            {
                for(int iNum=0;iNum<4;iNum++)
                {
                    if(i == g_aiNextFourVideoIdx[iNum])
                    {
                        iNowPlay = 1;
                        break;
                    }
                }
            }

        }

        if(iNowPlay)
        {
            icon = iOnline ? pImageBtn[iImgIndex-1][0]:pImageBtn[iImgIndex-1][1];
        }
        else
        {
            icon = iOnline ? pImageBtn[iImgIndex-1][2]:pImageBtn[iImgIndex-1][3];
        }
        if(iOnline && iShelterWarn)
        {
            icon = pImageBtn[iImgIndex-1][4];
        }
        if(icon.name() != Nowicon.name())
        {
            videoGroupBtn[iGroup][iPos]->setIcon(icon);
            videoGroupBtn[iGroup][iPos]->hide();
            videoGroupBtn[iGroup][iPos]->show();
        }

    }

}
void cctvTest::PlayStyleChanged()
{




}


void cctvTest::PlayCtrlFun()
{
    if(g_eCurPlayStyle != g_eNextPlayStyle)
    {
        PlayStyleChanged();
        g_eCurPlayStyle = g_eNextPlayStyle;
        g_iNeedUpdateWarnIcon = 1;
    }




}


cctvTest::cctvTest(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::cctvTest)
{
    ui->setupUi(this);
    m_tLastTime = 0;
    gettimeofday(&m_tPrevClickTime, NULL);
    m_iPecuInfo = GetPecuWarnInfo();
    GetAllDoorWarnInfo(m_acDoorWarnInfo,sizeof(m_acDoorWarnInfo));
    GetAllDoorClipInfo(m_acDoorClipWarnInfo, sizeof(m_acDoorClipWarnInfo));
    GetAllFireWarnInfo(m_acFireWarnInfo, sizeof(m_acFireWarnInfo));
    m_iFireCount = 0;
    m_iDoorCount =0;
    m_iPecuCount = 0;
    m_iDoorClipCount = 0;

    setUi();

    g_iCycTime = GetCycTime();

    connect(ui->monitorpushButton,SIGNAL(clicked()),this,SLOT(showMonitorPage()));
    connect(ui->signalBUtton,SIGNAL(clicked()),this,SLOT(sigalePageSlot()));    connect(ui->fourpushButton,SIGNAL(clicked()),this,SLOT(fourPageSlot()));
    connect(ui->startpushButton,SIGNAL(clicked()),this,SLOT(cycleSlot()));
    connect(ui->stoppushButton,SIGNAL(clicked()),this,SLOT(cycleSlot()));

    UpdateTimer = new QTimer(this);
    UpdateTimer->start(1000);
    connect(UpdateTimer,SIGNAL(timeout()),this,SLOT(timeupdateSlot()));

    updatePlayTimer = new QTimer(this);
    updatePlayTimer->start(150);
    connect(updatePlayTimer,SIGNAL(timeout()),this,SLOT(updatePlaySlot()));

}

cctvTest::~cctvTest()
{
    if (UpdateTimer != NULL)
    {
        if (UpdateTimer ->isActive())
        {
            UpdateTimer ->stop();
        }
        delete UpdateTimer;
        UpdateTimer  = NULL;
    }

    if (updatePlayTimer != NULL)
    {
        if (updatePlayTimer ->isActive())
        {
            updatePlayTimer ->stop();
        }
        delete updatePlayTimer;
        updatePlayTimer  = NULL;
    }

    delete ui;
}


int cctvTest::FindCameBtnInfo(QAbstractButton* pbtn,int &iGroup,int &iNo)
{

    for(int i=0;i<8;i++)
    {
        for(int j=0;j<4;j++)
        {
            if(videoGroupBtn[i][j] == pbtn)
            {
                iGroup = i;
                iNo = j;
                return 1;
            }
        }
    }
    return 0;
}

void cctvTest::PlayWidCicked(QPoint pos)
{
    static struct timeval tNow ;
    gettimeofday(&tNow, NULL);
    int idiff = (tNow.tv_sec - m_tPrevClickTime.tv_sec)*1000000
        +tNow.tv_usec - m_tPrevClickTime.tv_usec;
    if(g_iWarnFreshed)
    {
        return;
    }
    if((idiff < 600000 && idiff > -600000) && (E_FOUR_VPLAY == g_eCurPlayStyle))
    {



    }


}

void cctvTest::GroupButtonClickSlot(QAbstractButton* pbtn)
{
    int iGroup =0, iNo=0;
    time_t tTime;
    time(&tTime);
    if(tTime == m_tLastTime ||  g_iWarnFreshed)
    {
        return;
    }
    m_tLastTime  = tTime;
    if(FindCameBtnInfo(pbtn,iGroup,iNo))
    {
        int iVideoIndex = GetVideoIdxAccordBtnPose(iGroup, iNo);
        if(E_SINGLE_VPLAY == g_eCurPlayStyle)  //如果此时为单画面模式
        {
            g_iNextSingleVideoIdx = iVideoIndex;
        }
        else
        {
            if(g_iWarn)
            {
                g_iNextSingleVideoIdx = iVideoIndex;
            }
            else
            {
                //当只有28个相机的时候 要将最后一组的相机数和第一组的相机数合起来
                //所以点击第一组和点击最后一组要显示的四个画面一样
                //这里是将选中第八组也等同于选中第一组
                if(7 == iGroup && GetVideoNum() <=28)
                {
                    iGroup = 0;
                }
                for(int i=0;i<4;i++)
                {
                    g_aiNextFourVideoIdx[i] = GetVideoIdxAccordBtnPose(iGroup,i);
                    if(-1 == g_aiNextFourVideoIdx[i] && (0 == iGroup && GetVideoNum() <=28))
                    {
                        g_aiNextFourVideoIdx[i] = GetVideoIdxAccordBtnPose(7,i);
                    }
                }
                g_iNextSingleVideoIdx = -1;
            }
        }

    }
    if(g_iVideoCycleFlag)
    {



    }




}

void cctvTest::GroupButtonFireSlot(int index)
{
    if(g_iWarnFreshed)
    {
        return ;
    }

    if(index >=0 )
    {
        g_iNextSingleVideoIdx = m_aiFireIdx[index];
    }

}


void cctvTest::GroupButtonDoorSlot(int index)
{
    if(g_iWarnFreshed)
    {
        return ;
    }
    if(index >=0 )
    {
        g_iNextSingleVideoIdx = m_aiDoorIdx[index];

    }
}

void cctvTest::GroupButtonDoorclipSlot(int index)
{
    if(g_iWarnFreshed)
    {
        return ;
    }
    if(index >=0 )
    {
        g_iNextSingleVideoIdx = m_aiDoorClipIdx[index];

    }

}

void cctvTest::GroupButtonPecuSlot(int index)
{
    if(g_iWarnFreshed)
    {
        return ;
    }
    if(index >=0 )
    {
        g_iNextSingleVideoIdx = m_aiPecuIdx[index];

    }


}

void cctvTest::setUi()
{
    char szData[5][256];
    char acImgFullName[56] = {0};
    char acDoorClipImage[56] = {0};
    char acPecuImage[56] = {0};
    QIcon icon;
//    QIcon playIcon[32][5];

//    pImageBtn[32][5]= (char *)malloc(sizeof(32*5));


    for(int i=0;i<32;i++)
    {
        memset(szData,0,sizeof(szData));
        snprintf(szData[0],sizeof(szData[0])-1,":/res/%d_act.png",i+1);
        snprintf(szData[1],sizeof(szData[0])-1,":/res/%d_act_nocon.png",i+1);
        snprintf(szData[2],sizeof(szData[0])-1,":/res/%d_noact.png",i+1);
        snprintf(szData[3],sizeof(szData[0])-1,":/res/%d_no.png",i+1);
        snprintf(szData[4],sizeof(szData[0])-1,":/res/%d_dis.png",i+1);

//        pImageBtn[i][0]=szData[0];
//        pImageBtn[i][1]=szData[1];
//        pImageBtn[i][2]=szData[2];
//        pImageBtn[i][3]=szData[3];
//        pImageBtn[i][4]=szData[4];

        pImageBtn[i][0].addFile(QString::fromUtf8(szData[0]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][1].addFile(QString::fromUtf8(szData[1]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][2].addFile(QString::fromUtf8(szData[2]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][3].addFile(QString::fromUtf8(szData[3]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][4].addFile(QString::fromUtf8(szData[4]),QSize(),QIcon::Normal,QIcon::Off);


    }

    videoGroupBtn[0][0] = ui->pushButton_1_1;
    videoGroupBtn[0][1] = ui->pushButton_1_2;
    videoGroupBtn[0][2] = ui->pushButton_1_3;
    videoGroupBtn[0][3] = ui->pushButton_1_4;
    videoGroupBtn[1][0] = ui->pushButton_1_5;
    videoGroupBtn[1][1] = ui->pushButton_1_6;
    videoGroupBtn[1][2] = ui->pushButton_1_7;
    videoGroupBtn[1][3] = ui->pushButton_1_8;

    videoGroupBtn[2][0] = ui->pushButton_2_1;
    videoGroupBtn[2][1] = ui->pushButton_2_2;
    videoGroupBtn[2][2] = ui->pushButton_2_3;
    videoGroupBtn[2][3] = ui->pushButton_2_4;

    videoGroupBtn[3][0] = ui->pushButton_3_1;
    videoGroupBtn[3][1] = ui->pushButton_3_2;
    videoGroupBtn[3][2] = ui->pushButton_3_3;
    videoGroupBtn[3][3] = ui->pushButton_3_4;

    videoGroupBtn[4][0] = ui->pushButton_4_1;
    videoGroupBtn[4][1] = ui->pushButton_4_2;
    videoGroupBtn[4][2] = ui->pushButton_4_3;
    videoGroupBtn[4][3] = ui->pushButton_4_4;

    videoGroupBtn[5][0] = ui->pushButton_5_1;
    videoGroupBtn[5][1] = ui->pushButton_5_2;
    videoGroupBtn[5][2] = ui->pushButton_5_3;
    videoGroupBtn[5][3] = ui->pushButton_5_4;

    videoGroupBtn[6][0] = ui->pushButton_6_1;
    videoGroupBtn[6][1] = ui->pushButton_6_2;
    videoGroupBtn[6][2] = ui->pushButton_6_3;
    videoGroupBtn[6][3] = ui->pushButton_6_4;
    videoGroupBtn[7][0] = ui->pushButton_6_5;
    videoGroupBtn[7][1] = ui->pushButton_6_6;
    videoGroupBtn[7][2] = ui->pushButton_6_6;
    videoGroupBtn[7][3] = ui->pushButton_6_7;

    g_buttonGroup = new QButtonGroup;


    for (int i = 0; i < 8; i++)
    {
        int iLeft = 15;
        if(0 == i)
        {
            iLeft = 17;
        }
        for(int j= 0;j<4;j++)
        {
            g_buttonGroup->addButton(videoGroupBtn[i][j]);

            int iIndex = GetVideoIdxAccordBtnPose(i, j);
            int iImgIndex = GetVideoImgIdx(iIndex);
            if(iIndex >= 0 && iImgIndex <=32 && iImgIndex >=1)
            {

//                icon.addFile(QString::fromUtf8(pImageBtn[iImgIndex-1][3]),QSize(),QIcon::Normal,QIcon::Off);
                videoGroupBtn[i][j]->setIcon(pImageBtn[iImgIndex-1][3]);

            }
            else
            {
                videoGroupBtn[i][j]->hide();

            }

        }

        for(int j = 0; j < 4; j++)
        {
            videoGroupBtn[i][j]->installEventFilter(this);
            videoGroupBtn[i][j]->setMouseTracking(true);
        }

    }

    m_playWidget[0] = new playwidget(this);
    m_playWidget[0]->setGeometry(0,0,416,312);

    m_playWidget[1] = new playwidget(this);
    m_playWidget[1]->setGeometry(0,312,416,312);

    m_playWidget[2] = new playwidget(this);
    m_playWidget[2]->setGeometry(416,0,416,312);

    m_playWidget[3] = new playwidget(this);
    m_playWidget[3]->setGeometry(416,312,416,312);

    m_playSingleWidget =new playwidget(this);
    m_playSingleWidget->setGeometry(0,0,832,624);


    for(int i=0;i<4;i++)
    {
       connect(m_playWidget[i],SIGNAL(sendClickSignal(QPoint)),this,SLOT(PlayWidCicked(QPoint)));

    }
    connect(m_playSingleWidget,SIGNAL(sendClickSignal(QPoint)),this,SLOT(PlayWidCicked(QPoint)));



    for(int i = 0; i < 4; i++)
    {
        m_playWidget[i]->setStyleSheet("QWidget{background-color: rgb(0, 0, 0);}");     //设置播放窗口背景色为黑色
        m_playWidget[i]->installEventFilter(this);     //播放窗体注册进事件过滤器
        m_playWidget[i]->setMouseTracking(true);
    }
    m_playSingleWidget->setStyleSheet("QWidget{background-color: rgb(0, 0, 0);}");
    m_playSingleWidget->installEventFilter(this);     //播放窗体注册进事件过滤器
    m_playSingleWidget->setMouseTracking(true);
    m_playSingleWidget->hide();

    memset(acImgFullName,0,sizeof(acImgFullName));
    snprintf(acImgFullName,sizeof(acImgFullName)-1,":/res/fire.bmp");

    g_fileButtonGroup = new QButtonGroup(this);

    for (int i = 0; i < 6; i++)
    {
        m_buttonFire[i]= new QPushButton(this);
        m_buttonFire[i]->setGeometry(20,10+50+i,100,45);
        icon.addFile(QString::fromUtf8(acImgFullName),QSize(),QIcon::Normal,QIcon::Off);
        m_buttonFire[i]->setIcon(icon);
        m_buttonFire[i]->hide();

        g_fileButtonGroup->addButton(m_buttonFire[i]);
    }



    memset(acImgFullName,0,sizeof(acImgFullName));
    snprintf(acImgFullName,sizeof(acImgFullName)-1,":/res/door.bmp");

    memset(acDoorClipImage,0,sizeof(acDoorClipImage));
    snprintf(acDoorClipImage,sizeof(acDoorClipImage)-1,":/res/clip.bmp");

    memset(acPecuImage,0,sizeof(acPecuImage));
    snprintf(acPecuImage,sizeof(acPecuImage)-1,":/res/pecu.bmp");


    g_doorButtonGroup = new QButtonGroup(this);
    g_doorclipButtonGroup = new QButtonGroup(this);
    g_PecuButtonGroup = new QButtonGroup(this);

    for(int i=0;i<2;i++)
    {
        for(int j=0;j<12;j++)
        {
            m_buttonDoor[i*12+j] = new QPushButton(this);
            m_buttonDoor[i*12+j]->setGeometry(130+i*115,10+50*j,100,45);
            icon.addFile(QString::fromUtf8(acImgFullName),QSize(),QIcon::Normal,QIcon::Off);
            m_buttonDoor[i*12+j]->setIcon(icon);
            m_buttonDoor[i*12+j]->hide();

            m_pBoxDoorClip[i*12+j] = new QPushButton(this);
            m_pBoxDoorClip[i*12+j]->setGeometry(130+i*115,10+50*j,100,45);
            icon.addFile(QString::fromUtf8(acDoorClipImage),QSize(),QIcon::Normal,QIcon::Off);
            m_pBoxDoorClip[i*12+j]->setIcon(icon);
            m_pBoxDoorClip[i*12+j]->hide();

            m_pBoxPecu[i*12+j] = new QPushButton(this);
            m_pBoxPecu[i*12+j]->setGeometry(130+i*115,10+50*j,100,45);
            icon.addFile(QString::fromUtf8(acPecuImage),QSize(),QIcon::Normal,QIcon::Off);
            m_pBoxPecu[i*12+j]->setIcon(icon);
            m_pBoxPecu[i*12+j]->hide();


            g_doorButtonGroup->addButton(m_buttonDoor[i*12+j]);
            g_doorclipButtonGroup->addButton(m_pBoxDoorClip[i*12+j]);
            g_PecuButtonGroup->addButton(m_pBoxPecu[i*12+j]);
        }
    }

    connect(g_fileButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonFireSlot(int)));
    connect(g_buttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(GroupButtonClickSlot(QAbstractButton *)));     //预置点按钮组按键信号连接响应槽函数
    connect(g_doorButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonDoorSlot(int)));
    connect(g_doorclipButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonDoorclipSlot(int)));
    connect(g_PecuButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonPecuSlot(int)));



}

void cctvTest::updatePlaySlot()
{






}


void cctvTest::timeupdateSlot()
{
    struct tm tLocalTime;
    time_t tTime;
    time(&tTime);
    char acTime[56] = {0};


    if (NULL == localtime_r(&tTime, &tLocalTime))
    {
         return;
    }

    m_u16Year = tLocalTime.tm_year +1900;
    m_u8Mon = tLocalTime.tm_mon +1;
    m_u8Day = tLocalTime.tm_mday;
    m_iWeek = tLocalTime.tm_wday;

    for(int i=0;i<3;i++)
    {
        if(0 == i)
        {
            memset(acTime,0,sizeof(acTime));
            sprintf(acTime,"%d-%02d-%02d",tLocalTime.tm_year +1900,tLocalTime.tm_mon +1,tLocalTime.tm_mday);
            ui->datelabel->setText(QString(QLatin1String(acTime)));

        }
        else if(1 == i)
        {
            switch (tLocalTime.tm_wday)
            {
                case 0:
                    ui->weeklabel->setText(QString("星期天"));
                    break;
                case 1:
                    ui->weeklabel->setText(QString("星期一"));
                    break;
                case 2:
                    ui->weeklabel->setText(QString("星期二"));
                    break;
                case 3:
                    ui->weeklabel->setText(QString("星期三"));
                    break;
                case 4:
                    ui->weeklabel->setText(QString("星期四"));
                    break;
                case 5:
                    ui->weeklabel->setText(QString("星期五"));
                    break;
                case 6:
                    ui->weeklabel->setText(QString("星期六"));
                    break;
            }
        }
        else if(2 == i)
        {
            memset(acTime,0,sizeof(acTime));
            sprintf(acTime,"%02d:%02d:%02d",tLocalTime.tm_hour,tLocalTime.tm_min,tLocalTime.tm_sec);
            ui->timelabel->setText(QString(QLatin1String(acTime)));

        }

    }
    return;

}


void cctvTest::showMonitorPage()
{
    this->hide();
    emit showMonitorSignal();
}

void cctvTest::sigalePageSlot()
{
    time_t tTime;
    time(&tTime);
    if((tTime == m_tLastTime || g_iWarn))
    {
        return;
    }
    m_tLastTime  = tTime;
    if(E_SINGLE_VPLAY == g_eCurPlayStyle )
    {
        return ;
    }
    else if(E_FOUR_VPLAY == g_eCurPlayStyle)
    {
        ui->fourpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_01_hig.png)}");
        ui->signalBUtton->setStyleSheet("QPushButton{border-image: url(:/res/btn_02_nor.png)}");
        g_eNextPlayStyle = E_SINGLE_VPLAY;

    }

}

void cctvTest::fourPageSlot()
{

    time_t tTime;
    time(&tTime);
    if((tTime == m_tLastTime || g_iWarn))
    {
        return;
    }
    m_tLastTime  = tTime;
    if(E_FOUR_VPLAY == g_eCurPlayStyle )
    {
        return ;
    }
    else if(E_SINGLE_VPLAY == g_eCurPlayStyle)
    {
        ui->fourpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_01_nor.png)}");
        ui->signalBUtton->setStyleSheet("QPushButton{border-image: url(:/res/btn_02_hig.png)}");
        g_eNextPlayStyle = E_FOUR_VPLAY;

    }


}

void cctvTest::closeVideoCyc()
{
    g_iVideoCycleFlag =0;
    ui->startpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_03_hig.png)}");
    ui->stoppushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_04_nor.png)}");


}


void cctvTest::cycleSlot()
{
    QObject* Sender = sender();     //Sender->objectName(),可区分不同的信号来源，也就是不同的按钮按键
    time_t tTime;

    time(&tTime);
    if(tTime == m_tLastTime || g_iWarn)
    {
        return;
    }
    m_tLastTime  = tTime;


    if(Sender==0)
    {
        return ;
    }
    if(Sender->objectName() == "startpushButton")
    {
        if(!g_iVideoCycleFlag)
        {
            g_iVideoCycleFlag = 1;
            if(E_FOUR_VPLAY == g_eCurPlayStyle)
            {
                g_iNextSingleVideoIdx = -1; //此时不能单画面显示了
            }
            ui->startpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_03_nor.png)}");
            ui->stoppushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_04_hig.png)}");

        }

    }
    else
    {
        closeVideoCyc();

    }

}

