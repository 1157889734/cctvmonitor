#include "cctv.h"
#include "ui_cctv.h"
#include <QDate>
#include <QDateTime>
#include "./state/state.h"
#include <pthread.h>
#include <stdio.h>
#include "unistd.h"
#include "NVRMsgProc.h"
#include <sys/time.h>
#include <QIcon>
#include <QDebug>
#include <QMouseEvent>
#include <QEvent>
#include <sys/sysinfo.h>
#include "pmsg/pmsgcli.h"


static int g_init_flag = 0;
static int g_imult_init_flag[4] ={0,0,0,0};

typedef  void*  CMPHandle;
static   CMPHandle	g_pHplay[8] = {0,0,0,0,0,0,0,0};	  //最多只能同时存在4个播放句柄，多了会出问题
static int	g_iWarnNum = 0;
static int	g_iWarn = 0;			//是否有报警

static int	g_iNeedUpdateWarnIcon = 0; //是否需要更新报警图标

static E_PLAY_STYLE g_eCurPlayStyle = E_FOUR_VPLAY;   //当前播放风格
static E_PLAY_STYLE g_eNextPlayStyle = g_eCurPlayStyle;
static int  g_iCurSingleVideoIdx = -1;
static int  g_iVideoCycleFlag = 1;  //轮询标志

static int  g_iNextSingleVideoIdx = -1;
static int g_iCycTime = 30;
static int  g_iWarnFreshed = 0;  //避免报警信息画面还未刷新，就被别的指令破坏



static CMPHandle	g_hSinglePlay = 0;
static CMPHandle	g_hBackSinglePlay = 0;
static int	g_iBackSingleVideoIdx = 0;

static int  g_aiBackFourVideoIdx[4] = {-1,-1,-1,-1};
static int  g_aiCurFourVideoIdx[4] = {-1,-1,-1,-1};
static int  g_aiNextFourVideoIdx[4] = {-1,-1,-1,-1};



void cctv::UpdateCamStatefunc()
{

    E_PLAY_STYLE eStyle = g_eCurPlayStyle;
    int iNVRState =0;
    int iNvrNo = 0;
    QIcon icon;
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

        m_NvrServerPhandle[(iNvrNo/2)*2] = STATE_GetNvrServerPmsgHandle((iNvrNo/2)*2);
        m_NvrServerPhandle[(iNvrNo/2)*2+1] = STATE_GetNvrServerPmsgHandle((iNvrNo/2)*2+1);

        iNVRState = PMSG_GetConnectStatus(m_NvrServerPhandle[(iNvrNo/2)*2]);

        if(E_SERV_STATUS_CONNECT != iNVRState)
        {
            iNVRState = PMSG_GetConnectStatus(m_NvrServerPhandle[(iNvrNo/2)*2+1]);

        }
        if(E_SERV_STATUS_CONNECT != iNVRState )
        {
            iOnline = 0;
        }


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
            if(iOnline)
            {
                videoGroupBtn[iGroup][iPos]->setIcon(pImageBtn[iImgIndex-1][0]);
            }
            else
            {
                videoGroupBtn[iGroup][iPos]->setIcon(pImageBtn[iImgIndex-1][1]);
            }

        }
        else
        {
            if(iOnline)
            {
                videoGroupBtn[iGroup][iPos]->setIcon(pImageBtn[iImgIndex-1][2]);
            }
            else
            {
                videoGroupBtn[iGroup][iPos]->setIcon(pImageBtn[iImgIndex-1][3]);
            }

        }

        if(iOnline && iShelterWarn)
        {
            videoGroupBtn[iGroup][iPos]->setIcon(pImageBtn[iImgIndex-1][4]);

        }

        videoGroupBtn[iGroup][iPos]->setIconSize(QSize(60,50));



    }


}

void cctv::UpdateWarnBtnfunc()
{
    for(int i=0;i<m_iFireCount;i++)
    {
        m_pBoxFire[i]->hide();
        m_pBoxFire[i]->show();
    }
    for(int i=0;i<m_iDoorCount;i++)
    {
        m_pBoxDoor[i]->hide();
        m_pBoxDoor[i]->show();
    }

    for(int i=0;i<m_iDoorClipCount;i++)
    {
        m_pBoxDoorClip[i]->hide();
        m_pBoxDoorClip[i]->show();
    }


    for(int i=0;i<m_iPecuCount;i++)
    {
        m_pBoxPecu[i]->hide();
        m_pBoxPecu[i]->show();

    }

}



void cctv::PlayStyleChangedfunc()
{
    if(g_eNextPlayStyle == E_FOUR_VPLAY)
    {
        //将之前的单画面播放句柄关掉
        if(g_hSinglePlay)
        {
            CMP_CloseMedia(g_hSinglePlay);
            CMP_UnInit(g_hSinglePlay);
            g_hSinglePlay = NULL;
        }
        g_iCurSingleVideoIdx = -1;
        g_iNextSingleVideoIdx = g_iCurSingleVideoIdx;

        if(g_hBackSinglePlay)
        {
            CMP_CloseMedia(g_hBackSinglePlay);
            CMP_UnInit(g_hBackSinglePlay);
            g_hBackSinglePlay = NULL;
        }
        g_iBackSingleVideoIdx = -1;
        m_playSingleWidget->hide();


        for(int i=0;i<4;i++)
        {
            if(g_pHplay[i])
            {
               CMP_CloseMedia(g_pHplay[i]);
               CMP_UnInit(g_pHplay[i]);
               g_pHplay[i] = NULL;
            }
            if(g_pHplay[i+4])
            {
               CMP_CloseMedia(g_pHplay[i+4]);
               CMP_UnInit(g_pHplay[i]);
               g_pHplay[i+4] = NULL;
            }
            g_aiCurFourVideoIdx[i] = -1;
            g_aiBackFourVideoIdx[i] = -1;
        }

        for(int i=0;i<4;i++)
        {
            int  iNextVideoIdx = g_aiNextFourVideoIdx[i];
            char acUrl[256] = {0};

            m_playWidget[i]->show();

            if(iNextVideoIdx != -1)
            {
                if(g_pHplay[i] == NULL)
                {
                    GetVideoRtspUrl(iNextVideoIdx,acUrl,sizeof(acUrl));
                    cmplayMultiInit(m_playWidget[i], i);
                    g_pHplay[i] = CMP_Init(m_RealMonitorVideos[i],CMP_VDEC_NORMAL);
                    CMP_OpenMediaPreview(g_pHplay[i], acUrl, CMP_TCP);
                    CMP_PlayMedia(g_pHplay[i]);
                    CMP_SetDisplayEnable(g_pHplay[i],1);
                }

            }

            g_aiCurFourVideoIdx[i] = g_aiNextFourVideoIdx[i];

        }

    }
    else
    {
        if(g_hBackSinglePlay)
        {
            CMP_CloseMedia(g_hBackSinglePlay);
            CMP_UnInit(g_hBackSinglePlay);
            g_hBackSinglePlay = NULL;
        }
        g_iBackSingleVideoIdx = -1;

        for(int i=0;i<4;i++)
        {
            if(g_pHplay[i])
            {
                CMP_CloseMedia(g_pHplay[i]);
                CMP_UnInit(g_pHplay[i]);
                g_pHplay[i] = NULL;
                g_aiCurFourVideoIdx[i] = -1;

            }
            g_aiNextFourVideoIdx[i] = -1;

            if(g_pHplay[i+4])
            {
               CMP_CloseMedia(g_pHplay[i+4]);
               CMP_UnInit(g_pHplay[i+4]);
               g_pHplay[i+4] = NULL;
               g_aiBackFourVideoIdx[i] = -1;

            }
            m_playWidget[i]->hide();

        }

        if(g_iCurSingleVideoIdx != g_iNextSingleVideoIdx)
        {
            m_playSingleWidget->show();
            char acUrl[256] = {0};

            if(g_hSinglePlay)
            {
                CMP_CloseMedia(g_hSinglePlay);
                CMP_UnInit(g_hSinglePlay);
                g_hSinglePlay = NULL;
            }

            cmplayInit(m_playSingleWidget);
            GetVideoMainRtspUrl(g_iNextSingleVideoIdx,acUrl,sizeof(acUrl));
            g_hSinglePlay = CMP_Init(m_RealMonitorSingleVideo,CMP_VDEC_NORMAL);
            CMP_OpenMediaPreview(g_hSinglePlay, acUrl, CMP_TCP);
            CMP_PlayMedia(g_hSinglePlay);
            CMP_SetDisplayEnable(g_hSinglePlay,1);

        }
        else
        {
            CMP_SetDisplayEnable(g_hSinglePlay,1);
        }
        g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;

    }
    g_eCurPlayStyle = g_eNextPlayStyle;



}

void cctv::SinglePlayStylefunc()
{

    if(g_iNextSingleVideoIdx != g_iCurSingleVideoIdx)
    {
        int iWarnIdx = -1;
        CMPHandle hTmp = NULL;  //保存当前相机的配置
        int iTmpCurIdx = -1;

        //判断当前单报警的相机是哪个
        //一直将其保存，目的是为了点击报警图标时能很快切回去
        if(1 == g_iWarnNum)
        {
           if(1 == m_iDoorCount)
           {
               iWarnIdx = m_aiDoorIdx[0];
           }
           else if(1 == m_iDoorClipCount)
           {
               iWarnIdx = m_aiDoorClipIdx[0];
           }
           else if(1 == m_iPecuCount)
           {
                iWarnIdx = m_aiPecuIdx[0];
           }
           else if(1 == m_iFireCount)
           {
                iWarnIdx = m_aiFireIdx[0];
           }
           g_iNeedUpdateWarnIcon = 1;
        }

        if(g_hSinglePlay)
        {
            //如果当前的不是报警索引
            if(iWarnIdx != g_iCurSingleVideoIdx)
            {
                CMP_CloseMedia(g_hSinglePlay);
                g_iCurSingleVideoIdx = -1;

            }
            else
            {

                iTmpCurIdx = g_iCurSingleVideoIdx;
                hTmp = g_hSinglePlay;
                g_hSinglePlay = NULL;
                g_iCurSingleVideoIdx = -1;
                CMP_SetDisplayEnable(hTmp,0);


            }

        }

        if(-1 != g_iNextSingleVideoIdx)
        {
            if(g_iNextSingleVideoIdx == g_iBackSingleVideoIdx)
            {
                g_hSinglePlay = g_hBackSinglePlay;
                g_hBackSinglePlay = NULL;
                g_iBackSingleVideoIdx = -1;
            }
            else
            {
                char acUrl[256] = {0};
                m_playSingleWidget->show();
                GetVideoMainRtspUrl(g_iNextSingleVideoIdx ,acUrl,sizeof(acUrl));

                if( !g_init_flag)
                {
                g_init_flag = 1;
                cmplayInit(m_playSingleWidget);
                g_hSinglePlay = CMP_Init(m_RealMonitorSingleVideo,CMP_VDEC_NORMAL);

                }

                CMP_OpenMediaPreview(g_hSinglePlay, acUrl, CMP_TCP);
                CMP_PlayMedia(g_hSinglePlay);
                CMP_SetDisplayEnable(g_hSinglePlay,1);
                single_node.empty();

            }

        }

       g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;

    }

}
void cctv::getPLayStrameState()
{
    if(g_eCurPlayStyle == E_FOUR_VPLAY)
    {
        for(int i = 0; i< 4; i++)
        {
            m_nodes[i].iIndex++;
            if(m_nodes[i].iIndex < 4)//2s
            {
                continue;
            }

            if(CMP_STATE_PLAY != CMP_GetPlayStatus(g_pHplay[i]))
            {
                    continue;
            }

            int ret = CMP_GetStreamState(g_pHplay[i]);
            if(m_nodes[i].iStreamState != ret)
            {
                m_nodes[i].iStreamState = ret;
                if(ret == 0)
                {
                    CMP_FillDisplayBk(g_pHplay[i],0);  //0x35BAB400
                }
            }

        }

    }
    else
    {
        single_node.iIndex++;
        if(single_node.iIndex < 4)//2s
        {
            return;
        }

        if(CMP_STATE_PLAY != CMP_GetPlayStatus(g_hSinglePlay))
        {
                return;
        }
        int ret = CMP_GetStreamState(g_hSinglePlay);
        if(single_node.iStreamState != ret)
        {
            single_node.iStreamState = ret;
            if(ret == 0)
            {
                CMP_FillDisplayBk(g_hSinglePlay,0);

            }
        }

    }

}

void cctv::FourPlayStylefunc()
{
    //单画面界面的视频发生了变化
    //从单到四 或从单到单 以及从四到单CMP_Init
    if(g_iNextSingleVideoIdx != g_iCurSingleVideoIdx)
    {
        qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

        if(g_hSinglePlay)   //切换时都需要把前面单播放句柄的先关掉
        {

            CMP_CloseMedia(g_hSinglePlay);
            CMP_UnInit(g_hSinglePlay);
            g_hSinglePlay = NULL;
            qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

        }

        if(g_iNextSingleVideoIdx != -1)   	//接下来要播放的为单画面视频
        {
            for(int i=0;i<4;i++)
            {
                if(g_pHplay[i])
                {
                       //不需要播放的四画面视频关闭显示了
                    CMP_SetDisplayEnable(g_pHplay[i],0);
                    CMP_SetPlayState(g_pHplay[i], CMP_STATE_PAUSE);

                }
                qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

            }

             char acUrl[256] = {0};

             m_playSingleWidget->show();
            if(g_hSinglePlay == NULL)
            {
                 GetVideoMainRtspUrl(g_iNextSingleVideoIdx,acUrl,sizeof(acUrl));
                 cmplayInit(m_playSingleWidget);
                 g_hSinglePlay = CMP_Init(m_RealMonitorSingleVideo,CMP_VDEC_NORMAL);
                 CMP_OpenMediaPreview(g_hSinglePlay, acUrl, CMP_TCP);
                 CMP_PlayMedia(g_hSinglePlay);
                 CMP_SetDisplayEnable(g_hSinglePlay,1);
//                 qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

            }


        }
        else   //接下来播放的是四视频
        {
            qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

            m_playSingleWidget->hide();
            for(int i=0;i<4;i++)
            {

                if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
                {
                    if(g_pHplay[i])
                    {
                        CMP_CloseMedia(g_pHplay[i]);
                        CMP_UnInit(g_pHplay[i]);
                        g_pHplay[i] = NULL;
                        qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

                    }
                    g_aiCurFourVideoIdx[i] = -1;
                }


            }
            for(int i=0;i<4;i++)
            {

                if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
                {
                    qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

                    m_playWidget[i]->show();

                    if(g_aiNextFourVideoIdx[i]!=-1)
                    {
                        int iFindIdx = -1;

                        for(int j=0;j<4;j++)
                        {
                             if(g_aiBackFourVideoIdx[j] == g_aiNextFourVideoIdx[i])
                             {
                                 iFindIdx = j;
                                 break;
                              }
                         }

                        if(g_pHplay[i] == NULL)
                         {
                            qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

                              char acUrl[256] = {0};
                              GetVideoRtspUrl(g_aiNextFourVideoIdx[i] ,acUrl,sizeof(acUrl));
                              cmplayMultiInit(m_playWidget[i], i);
                              g_pHplay[i] = CMP_Init(m_RealMonitorVideos[i],CMP_VDEC_NORMAL);
                              CMP_OpenMediaPreview(g_pHplay[i], acUrl, CMP_TCP);
                              CMP_PlayMedia(g_pHplay[i]);
                              CMP_SetDisplayEnable(g_pHplay[i],1);

                          }
                    }
                    g_aiCurFourVideoIdx[i] = g_aiNextFourVideoIdx[i];
                }
                for(int i=0;i<4;i++)
                {
                    if(g_pHplay[i])
                    {
                        qDebug()<<"**************"<<__FUNCTION__<<__LINE__;
                        CMP_SetPlayState(g_pHplay[i], CMP_STATE_PLAY);
                        CMP_SetDisplayEnable(g_pHplay[i],1);
                    }

                }
            }

        }

    g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;

    //在四画面与单画面切换时，有可能导致报警图标被隐藏
    g_iNeedUpdateWarnIcon =1;
   }    //从四到四
   else if(-1 == g_iCurSingleVideoIdx )
   {
//        qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

        m_playSingleWidget->hide();

       for(int i=0;i<4;i++)
       {
          if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
          {
               if(g_pHplay[i])
               {
                   CMP_CloseMedia(g_pHplay[i]);

               }
               g_aiCurFourVideoIdx[i] = -1;
          }
       }

       for(int i=0;i<4;i++)
       {
          if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
          {
               if(g_aiNextFourVideoIdx[i]!=-1)
               {
                    char acUrl[256] = {0};
                    m_playWidget[i]->show();
                    GetVideoRtspUrl(g_aiNextFourVideoIdx[i] ,acUrl,sizeof(acUrl));

                    if(!g_imult_init_flag[i])
                    {
                      g_imult_init_flag[i] = 1;
                      qDebug()<<"*******CMP_Init*******"<<__FUNCTION__<<__LINE__;
                      cmplayMultiInit(m_playWidget[i], i);
                      g_pHplay[i] = CMP_Init(m_RealMonitorVideos[i],CMP_VDEC_NORMAL);
                    }

                    CMP_OpenMediaPreview(g_pHplay[i], acUrl, CMP_TCP);
                    CMP_PlayMedia(g_pHplay[i]);
                    CMP_SetDisplayEnable(g_pHplay[i],1);
                    m_nodes[i].empty();
                    qDebug()<<"**************"<<__FUNCTION__<<__LINE__;

               }
               g_aiCurFourVideoIdx[i] = g_aiNextFourVideoIdx[i];
               g_iNeedUpdateWarnIcon =1;
           }

       }

    }


}


void cctv::PlayCtrlFunSlot()
{

    playTimer->stop();

    if(g_eCurPlayStyle != g_eNextPlayStyle)
    {
        PlayStyleChangedfunc();
        g_eCurPlayStyle = g_eNextPlayStyle;
        g_iNeedUpdateWarnIcon = 1;
    }

    if(E_FOUR_VPLAY == g_eCurPlayStyle)
    {
        FourPlayStylefunc();

    }
    else
    {
        SinglePlayStylefunc();

    }


    if(g_iNeedUpdateWarnIcon)  //界面切换时,有可能将报警图标给挡住，所以最好是重新显示一次
    {
        g_iNeedUpdateWarnIcon =0;
        UpdateWarnBtnfunc();
    }
    if(g_iWarnFreshed == 1)
    {
        UpdateCamStatefunc();
    }
    g_iWarnFreshed = 0;


    getPLayStrameState();


    playTimer->start();

    return;

}



#define GET_DEVSTATE_MONITOR_TIME 10     //获取设备状态间隔时间，单位秒
#define SET_TIME_MONITOR_TIME 1   //更新设备维护界面时间显示的间隔时间，单位秒
#define GET_DEVSTATE_WARN_TIME 1
#define GET_VIDEO_PLAY_TIME 1


void *monitorTread(void *param)
{
//    int i = 0, iRet = 0;
    int iPollingTime = 0;

//    int iPollingFlag = 0;
    time_t tPollWarnStateCurTime = 0,tPollWarnStateOldTime = 0;
    time_t tPollingCurTime = 0;
    time_t tGetDevStateCurTime = 0, tGetDevStateOldTime = 0;
    time_t tSetTimeCurTime = 0, tSetTimeOldTime = 0;
    time_t tPlayTimeCurTime = 0, tPLayTimeOldTime = 0;

    struct sysinfo s_info;

    cctv *pvmsMonitorPage = (cctv *)param;
    if (NULL == pvmsMonitorPage)
    {
        return NULL;
    }
    tPollingCurTime = s_info.uptime;
    tGetDevStateCurTime = s_info.uptime;
    tSetTimeCurTime = s_info.uptime;
    tPollWarnStateCurTime = s_info.uptime;
    tPlayTimeCurTime = s_info.uptime;

    tGetDevStateOldTime = tGetDevStateCurTime;
    tSetTimeOldTime     = tSetTimeCurTime;
    tPollWarnStateOldTime = tPollWarnStateCurTime;
    tPLayTimeOldTime = tPlayTimeCurTime;

    pvmsMonitorPage->tPollingOparateTime = tPollingCurTime - iPollingTime;//保证线程一进来，就进行一次循环处理

    pvmsMonitorPage->videoPollingfunction();

    pthread_detach(pthread_self());

    while (1 == pvmsMonitorPage->m_iThreadRunFlag)
    {
        iPollingTime = g_iCycTime;
        if (iPollingTime < 0)
        {
            usleep(500*1000);
            continue;
        }

        if((g_iVideoCycleFlag == 1) && ((tPollingCurTime-pvmsMonitorPage->tPollingOparateTime) >= iPollingTime)) //轮询
        {
//            pvmsMonitorPage->videoPollingfunction();
//            pvmsMonitorPage->triggerPollSignal();
            pvmsMonitorPage->tPollingOparateTime = tPollingCurTime;

        }

        if ((tGetDevStateCurTime - tGetDevStateOldTime) >= GET_DEVSTATE_MONITOR_TIME)  //设备状态更新
        {

            pvmsMonitorPage->triggerGetDevStateSignal();
            tGetDevStateOldTime = tGetDevStateCurTime;


        }

        if ((tSetTimeCurTime - tSetTimeOldTime) >= SET_TIME_MONITOR_TIME) //设备时间刷新
        {

            pvmsMonitorPage->triggerSetTimeSignal();
            tSetTimeOldTime = tSetTimeCurTime;

        }

        if ((tPollWarnStateCurTime - tPollWarnStateOldTime) >= GET_DEVSTATE_WARN_TIME) //报警信息更新
        {


//            pvmsMonitorPage->triggerWarnInfoSignal();
            tPollWarnStateOldTime = tPollWarnStateCurTime;

        }

//        if ((tPlayTimeCurTime - tPLayTimeOldTime) >= GET_VIDEO_PLAY_TIME) //
        {


//            pvmsMonitorPage->triggerPlaySignal();
            tPLayTimeOldTime = tPlayTimeCurTime;

        }


        usleep(200*1000);
        if(sysinfo(&s_info))
        {
            printf("\n\ncode error\n");
        }
        tPollingCurTime = s_info.uptime;
        tGetDevStateCurTime = s_info.uptime;
        tSetTimeCurTime = s_info.uptime;
        tPollWarnStateCurTime = s_info.uptime;
        tPlayTimeCurTime =s_info.uptime;
    }


    return NULL;
}
void cctv::videoPollingfunction()
{

    if(g_iVideoCycleFlag == 1)
    {

        if(E_FOUR_VPLAY == g_eCurPlayStyle)
         {
             g_iNextSingleVideoIdx = -1; //此时不能单画面显示了
             GetNextFourVideo(g_aiNextFourVideoIdx);

         }
         else
         {
             GetNextSingleVideo(&g_iNextSingleVideoIdx);

         }
        UpdateCamStatefunc();

    }

}

cctv::cctv(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::cctv)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
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
    monitorthread = 0;

    memset(g_pHplay,0,sizeof(g_pHplay));

    memset(&m_RealMonitorVideos, 0, sizeof(m_RealMonitorVideos));
    memset(&m_RealMonitorSingleVideo, 0, sizeof(m_RealMonitorSingleVideo));

    setUi();

    g_iCycTime = GetCycTime();

    connect(ui->monitorManageButton,SIGNAL(clicked()),this,SLOT(showMonitorManagePage()));
    connect(ui->signalBUtton,SIGNAL(clicked()),this,SLOT(sigalePageSlot()));
    connect(ui->fourpushButton,SIGNAL(clicked()),this,SLOT(fourPageSlot()));
    connect(ui->openPollButton,SIGNAL(clicked()),this,SLOT(cycleSlot()));
    connect(ui->closePollButton,SIGNAL(clicked()),this,SLOT(cycleSlot()));

    connect(this,SIGNAL(sendWindIndexSignal(int)),this,SLOT(PlayWidCicked(int)));
    connect(this, SIGNAL(setTimeSignal()), this, SLOT(timeupdateSlot()));
//    connect(this,SIGNAL(sendWarnSignal()),this,SLOT(updateWarnInfoSLot()));
//    connect(this,SIGNAL(sendPollSignal()),this,SLOT(videoPollingfunction()));
//    connect(this,SIGNAL(sendPlaySignal()),this,SLOT(PlayCtrlFunSlot()));


    m_iThreadRunFlag = 1;
    pthread_create(&monitorthread, NULL, monitorTread, (void *)this);


    playTimer = new QTimer(this);
    playTimer->setInterval(500);
    playTimer->start();
    connect(playTimer,SIGNAL(timeout()),this,SLOT(PlayCtrlFunSlot()));

    updateWarnTimer = new QTimer(this);
    updateWarnTimer->start(1000);
    connect(updateWarnTimer,SIGNAL(timeout()),this,SLOT(updateWarnInfoSLot()));


    pollTimer = new QTimer(this);
    pollTimer->setInterval(1000*g_iCycTime);
    pollTimer->start();
    connect(pollTimer,SIGNAL(timeout()),this,SLOT(videoPollingfunction()));


}

cctv::~cctv()
{

    if (monitorthread != 0)
    {
        pthread_join(monitorthread, NULL);
        monitorthread = 0;
        m_iThreadRunFlag = 0;
    }

    if (playTimer != NULL)
    {
        if (playTimer ->isActive())
        {
            playTimer ->stop();
        }
        delete playTimer;
        playTimer  = NULL;
    }

    if (updateWarnTimer != NULL)
    {
        if (updateWarnTimer ->isActive())
        {
            updateWarnTimer ->stop();
        }
        delete updateWarnTimer;
        updateWarnTimer  = NULL;
    }

    if (pollTimer != NULL)
    {
        if (pollTimer ->isActive())
        {
            pollTimer ->stop();
        }
        delete pollTimer;
        pollTimer  = NULL;
    }

    for(int i = 0; i <4; i++)
    {
        delete m_playWidget[i];
        m_playWidget[i] = NULL;
    }

    delete m_playSingleWidget;
    m_playSingleWidget = NULL;

    delete  g_fileButtonGroup;
    g_fileButtonGroup = NULL;

    delete  g_videoNumbuttonGroup;
    g_videoNumbuttonGroup = NULL;

    delete g_doorButtonGroup;
    g_doorButtonGroup = NULL;

    delete g_doorclipButtonGroup;
    g_doorclipButtonGroup = NULL;

    delete g_PecuButtonGroup;
    g_PecuButtonGroup = NULL;

    delete ui;
}

void cctv::triggerGetDevStateSignal()
{
    emit getDevStateSignal();
}

void cctv::triggerSetTimeSignal()
{
    emit setTimeSignal();
}
void cctv::triggerWarnInfoSignal()
{
    emit sendWarnSignal();
}

void cctv::triggerPlaySignal()
{
    emit sendPlaySignal();

}
void cctv::triggerPollSignal()
{
    emit sendPollSignal();
}



void cctv::cmplayInit(QWidget *g_widget)
{
    m_RealMonitorSingleVideo = g_widget;

}
void cctv::cmplayMultiInit(QWidget *g_widget, int i)
{

    m_RealMonitorVideos[i] = g_widget;
}

int cctv::FindCameBtnInfo(QAbstractButton* pbtn,int &iGroup,int &iNo)
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
int cctv::GetNextFourVideo(int* piVideo)
{
    int iGroup = -1,iPos = -1;
    int iFirstVideo =-1;
    int iVideoNum = GetVideoNum();

    for(int i=0;i<4;i++)
    {
        if(-1 != g_aiCurFourVideoIdx[i])
        {
             iFirstVideo = g_aiCurFourVideoIdx[i];
        }
    }
    GetBtnPoseAccordVideoIdx(iFirstVideo, &iGroup, &iPos);
    iGroup ++;

           //当只有28个相机的时候 要将最后一组的相机数和第一组的相机数合起来
           //所以点击第一组和点击最后一组要显示的四个画面一样
           //这里是将选中第八组也等同于选中第一组
    if((7 == iGroup ) && (iVideoNum <=28))
    {
        iGroup = 0;
    }
    else if( (8 == iGroup) )
    {
       if(iVideoNum <=28)
       {
           iGroup = 1;
       }
       else
       {
           iGroup = 0;
       }
    }

    for(int i=0;i<4;i++)
    {
         piVideo[i] = GetVideoIdxAccordBtnPose(iGroup,i);
         if(-1 == piVideo[i] && (0 == iGroup && GetVideoNum() <=28))
         {
              piVideo[i] = GetVideoIdxAccordBtnPose(7,i);
         }
    }
    return 0;


}

int cctv::GetNextSingleVideo(int *piVideo)
{

    int iGroup = -1,iPos =-1;
    int iVideoIndex = -1;
    int iRet = -1;

    GetBtnPoseAccordVideoIdx(g_iCurSingleVideoIdx, &iGroup, &iPos);
    iPos ++;
    if(iPos >=4)
    {
       iPos = 0;
       iGroup ++;
       if(8 == iGroup)
       {
           iGroup = 0;
       }
    }
    //按界面的顺序来轮巡，因为下一个按钮有可能是第8组最后一个，
    //而第八组最后一个可能不存在，所以找两次
    for(int iCount =0;iCount<2;iCount++)
    {
       for(int i=iGroup;i<8;i++)
       {
           for(int j = iPos;j<4;j++)
           {
               iVideoIndex = GetVideoIdxAccordBtnPose(i, j);
               if(iVideoIndex >=0)
               {
                   iRet =0;
                   iGroup = i;
                   iPos = j;
                   break;
               }
           }
           if(0 == iRet)
           {
               break;
           }
           iPos =0;
       }
       if(iRet <0)
       {
           iGroup =0;
           iPos =0;
       }
       else
       {
           break;
       }
    }
    *piVideo = GetVideoIdxAccordBtnPose(iGroup, iPos);
    return 0;


}


bool cctv::eventFilter(QObject *target, QEvent *event) //事件过滤器
{
    if (event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseMove) //判断界面操作
    {
        if (event->type()==QEvent::MouseMove)
        {
            QMouseEvent *mEvent = (QMouseEvent *)event;

            if ((m_iMousePosX != mEvent->globalPos().x()) || (m_iMousePosY != mEvent->globalPos().y()))    //防止实际没动鼠标而系统生成了mouseMove事件
            {
                m_iMousePosX = mEvent->globalPos().x();
                m_iMousePosY = mEvent->globalPos().y();
            }
            else
            {
                return true;
            }
        }


        if(event->type()==QEvent::MouseButtonPress)
        {
            if(target == m_playWidget[0])
            {
                emit sendWindIndexSignal(0);
            }
            else if(target == m_playWidget[1])
            {
                emit sendWindIndexSignal(1);
            }
            else if(target == m_playWidget[2])
            {
                emit sendWindIndexSignal(2);
            }
            else if(target == m_playWidget[3])
            {
                emit sendWindIndexSignal(3);
            }
            else if(target == m_playSingleWidget)
            {
                emit sendWindIndexSignal(4);
            }

        }

    }

    return QWidget::eventFilter(target, event);


}

void cctv::PlayWidCicked(int index)
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

        int iPlayWidIndex = index;
        if(-1 != g_iCurSingleVideoIdx)  //当前如果为四画面播放
        {
            g_iNextSingleVideoIdx = -1;
            pollTimer->start();
        }
        else if(iPlayWidIndex >=0 && iPlayWidIndex <4)
        {
            if(g_pHplay[iPlayWidIndex] /*&& CMP_GetStreamState(g_pHplay[iPlayWidIndex])*/)
            {
                g_iNextSingleVideoIdx = g_aiCurFourVideoIdx[iPlayWidIndex];
                pollTimer->stop();

            }
        }

    }
    m_tPrevClickTime = tNow;

}

void cctv::GroupButtonVideoClickSlot(QAbstractButton* pbtn)
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
        qDebug()<<"*****iGroup="<<iGroup<<"***********iNo="<<iNo<<"iVideoIndex="<<iVideoIndex<<__LINE__;
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
    qDebug()<<"*****************g_iVideoCycleFlag="<<g_iVideoCycleFlag<<__FUNCTION__<<__LINE__;

    if(g_iVideoCycleFlag)
    {
        for(int i=4;i<8;i++)
        {
            if(g_pHplay[i])
            {
                CMP_CloseMedia(g_pHplay[i]);
                CMP_UnInit(g_pHplay[i]);

                g_pHplay[i] = NULL;
            }
            g_aiBackFourVideoIdx[i-4] = -1;
        }
        if(g_hBackSinglePlay)
        {
            CMP_CloseMedia(g_hBackSinglePlay);
            CMP_UnInit(g_hBackSinglePlay);

            g_hBackSinglePlay = NULL;
        }
        g_iBackSingleVideoIdx = -1;

    }

    UpdateCamStatefunc();


}

void cctv::GroupButtonFireSlot(int index)
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


void cctv::GroupButtonDoorSlot(int index)
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

void cctv::GroupButtonDoorclipSlot(int index)
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

void cctv::GroupButtonPecuSlot(int index)
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



void cctv::setUi()
{
    char acImageData[5][256];
    char acImgFullName[56] = {0};
    char acDoorClipImage[56] = {0};
    char acPecuImage[56] = {0};
    QIcon iconImage;
    memset(acImageData,0,sizeof(acImageData));

    for(int i=0;i<32;i++)
    {
        snprintf(acImageData[0],sizeof(acImageData[0])-1,":/res/%d_noact.png",i+1);  //在线播放中
        snprintf(acImageData[1],sizeof(acImageData[0])-1,":/res/%d_act_nocon.png",i+1); //不在线播放中
        snprintf(acImageData[2],sizeof(acImageData[0])-1,":/res/%d_act.png",i+1); //在线未播放
        snprintf(acImageData[3],sizeof(acImageData[0])-1,":/res/%d_no.png",i+1); //不在线未播放
        snprintf(acImageData[4],sizeof(acImageData[0])-1,":/res/%d_dis.png",i+1); //不在线未播放



        pImageBtn[i][0].addFile(QString::fromUtf8(acImageData[0]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][1].addFile(QString::fromUtf8(acImageData[1]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][2].addFile(QString::fromUtf8(acImageData[2]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][3].addFile(QString::fromUtf8(acImageData[3]),QSize(),QIcon::Normal,QIcon::Off);
        pImageBtn[i][4].addFile(QString::fromUtf8(acImageData[4]),QSize(),QIcon::Normal,QIcon::Off);

    }

    videoGroupBtn[1][0] = ui->pushButton_1_1;
    videoGroupBtn[1][1] = ui->pushButton_1_2;
    videoGroupBtn[1][2] = ui->pushButton_1_3;
    videoGroupBtn[1][3] = ui->pushButton_1_4;
    videoGroupBtn[0][0] = ui->pushButton_1_5;
    videoGroupBtn[0][1] = ui->pushButton_1_6;
    videoGroupBtn[0][2] = ui->pushButton_1_7;
    videoGroupBtn[0][3] = ui->pushButton_1_8;

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
    videoGroupBtn[7][2] = ui->pushButton_6_7;
    videoGroupBtn[7][3] = ui->pushButton_6_8;

    g_videoNumbuttonGroup = new QButtonGroup(this);


    for (int i = 0; i < 8; i++)
    {
        for(int j= 0;j<4;j++)
        {
            g_videoNumbuttonGroup->addButton(videoGroupBtn[i][j]);

            int iIndex = GetVideoIdxAccordBtnPose(i, j);
            int iImgIndex = GetVideoImgIdx(iIndex);
            if(iIndex >= 0 && iImgIndex <=32 && iImgIndex >=1)
            {
                videoGroupBtn[i][j]->setIcon(pImageBtn[iImgIndex -1][0]);
                videoGroupBtn[i][j]->setIconSize(QSize(60,50));
                videoGroupBtn[i][j]->show();
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



    m_playWidget[0] = new QWidget(this);
    m_playWidget[0]->setGeometry(0,0,416,320);
    m_playWidget[0]->setObjectName("m_playWidget0");


    m_playWidget[2] = new QWidget(this);
    m_playWidget[2]->setGeometry(0,321,416,320);
    m_playWidget[2]->setObjectName("m_playWidget1");


    m_playWidget[1] = new QWidget(this);
    m_playWidget[1]->setGeometry(417,0,416,320);
    m_playWidget[1]->setObjectName("m_playWidget2");


    m_playWidget[3] = new QWidget(this);
    m_playWidget[3]->setGeometry(417,321,416,320);
    m_playWidget[3]->setObjectName("m_playWidget3");


    m_playSingleWidget =new QWidget(this);
    m_playSingleWidget->setGeometry(0,0,832,640);
    m_playSingleWidget->setObjectName("m_playWidget");


    for(int i = 0; i < 4; i++)
    {
        m_playWidget[i]->setStyleSheet("QWidget{background-color: rgb(0, 0, 0);}");     //设置播放窗口背景色为黑色
        m_playWidget[i]->installEventFilter(this);     //播放窗体注册进事件过滤器
        m_playWidget[i]->setMouseTracking(true);
        m_playWidget[i]->show();
        cmplayMultiInit(m_playWidget[i], i);

    }

    m_playSingleWidget->setStyleSheet("QWidget{background-color: rgb(0, 0, 0);}");
    m_playSingleWidget->installEventFilter(this);     //播放窗体注册进事件过滤器
    m_playSingleWidget->setMouseTracking(true);
    m_playSingleWidget->hide();
    cmplayInit(m_playSingleWidget);



    memset(acImgFullName,0,sizeof(acImgFullName));
    snprintf(acImgFullName,sizeof(acImgFullName)-1,":/res/fire.bmp");

    g_fileButtonGroup = new QButtonGroup(this);

    for (int i = 0; i < 6; i++)
    {
        m_pBoxFire[i]= new QPushButton(this);
        m_pBoxFire[i]->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint| Qt::Dialog);
        m_pBoxFire[i]->setFont(QFont("宋体",16));
        m_pBoxFire[i]->setFlat(true);
        m_pBoxFire[i]->setStyleSheet("QPushButton{border:none;background:transparent;color:rgb(255,255,255)}");
        m_pBoxFire[i]->setGeometry(20,10+50*i,100,45);
        iconImage.addFile(QString::fromUtf8(acImgFullName),QSize(),QIcon::Normal,QIcon::Off);
        m_pBoxFire[i]->setIcon(iconImage);
        m_pBoxFire[i]->setIconSize(QSize(60,45));
        m_pBoxFire[i]->hide();
        g_fileButtonGroup->addButton(m_pBoxFire[i]);
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
            m_pBoxDoor[i*12+j] = new QPushButton(this);
            m_pBoxDoor[i*12+j]->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint| Qt::Dialog);
            m_pBoxDoor[i*12+j]->setFont(QFont("宋体",16));
            m_pBoxDoor[i*12+j]->setFlat(true);
            m_pBoxDoor[i*12+j]->setStyleSheet("QPushButton{border:none;background:transparent;color:rgb(255,255,255)}");
            m_pBoxDoor[i*12+j]->setGeometry(130+i*115,10+50*j,100,45);
            iconImage.addFile(QString::fromUtf8(acImgFullName),QSize(),QIcon::Normal,QIcon::Off);
            m_pBoxDoor[i*12+j]->setIcon(iconImage);
            m_pBoxDoor[i*12+j]->setIconSize(QSize(60,45));
            m_pBoxDoor[i*12+j]->hide();

            m_pBoxDoorClip[i*12+j] = new QPushButton(this);
            m_pBoxDoorClip[i*12+j]->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint| Qt::Dialog);
            m_pBoxDoorClip[i*12+j]->setFont(QFont("宋体",16));
            m_pBoxDoorClip[i*12+j]->setFlat(true);
            m_pBoxDoorClip[i*12+j]->setStyleSheet("QPushButton{border:none;background:transparent;color:rgb(255,255,255)}");
            m_pBoxDoorClip[i*12+j]->setGeometry(360+i*115,10+50*j,100,45);
            iconImage.addFile(QString::fromUtf8(acDoorClipImage),QSize(),QIcon::Normal,QIcon::Off);
            m_pBoxDoorClip[i*12+j]->setIcon(iconImage);
            m_pBoxDoorClip[i*12+j]->setIconSize(QSize(60,45));
            m_pBoxDoorClip[i*12+j]->hide();

            m_pBoxPecu[i*12+j] = new QPushButton(this);
            m_pBoxPecu[i*12+j]->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint| Qt::Dialog);
            m_pBoxPecu[i*12+j]->setFont(QFont("宋体",16));
            m_pBoxPecu[i*12+j]->setFlat(true);
            m_pBoxPecu[i*12+j]->setStyleSheet("QPushButton{border:none;background:transparent;color:rgb(255,255,255)}");
            m_pBoxPecu[i*12+j]->setGeometry(590+i*115,10+50*j,100,45);
            iconImage.addFile(QString::fromUtf8(acPecuImage),QSize(),QIcon::Normal,QIcon::Off);
            m_pBoxPecu[i*12+j]->setIconSize(QSize(60,45));
            m_pBoxPecu[i*12+j]->setIcon(iconImage);
            m_pBoxPecu[i*12+j]->hide();


            g_doorButtonGroup->addButton(m_pBoxDoor[i*12+j]);
            g_doorclipButtonGroup->addButton(m_pBoxDoorClip[i*12+j]);
            g_PecuButtonGroup->addButton(m_pBoxPecu[i*12+j]);
        }

    }

    connect(g_videoNumbuttonGroup, SIGNAL(buttonClicked(QAbstractButton *)), this, SLOT(GroupButtonVideoClickSlot(QAbstractButton *)));
    connect(g_fileButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonFireSlot(int)));
    connect(g_doorButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonDoorSlot(int)));
    connect(g_doorclipButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonDoorclipSlot(int)));
    connect(g_PecuButtonGroup,SIGNAL(buttonClicked(int)),this,SLOT(GroupButtonPecuSlot(int)));



}


void cctv::updateWarnInfoSLot()
{
    char acFireWarnInfo[6] = {0};
    char acDoorWarnInfo[6] = {0};
    char acDoorClipWarnInfo[6] = {0};
    unsigned int	 iPecuWarnInfo = 0;
    int	 iPecuFirstWarnVideoIdx = -1;
    int  iWarnNum = 0;
    int  aiWarnVideoIdx[36] ={0};  //报警相机的序号  因为不同的报警类型 相机序号有重复所以要判断是否已存在啦
    int  iChanged = 0;
    char aiDoorWarnVideoIdx[48];
    char aiDoorClipWarnVideoIdx[48];

    memset(acFireWarnInfo,0,sizeof(acFireWarnInfo));
    memset(acDoorWarnInfo,0,sizeof(acDoorWarnInfo));
    memset(acDoorClipWarnInfo,0,sizeof(acDoorClipWarnInfo));

    GetAllDoorWarnInfo(acDoorWarnInfo, 6);
    GetAllFireWarnInfo(acFireWarnInfo, 6);
    GetAllDoorClipInfo(acDoorClipWarnInfo, 6);
    iPecuWarnInfo = GetPecuWarnInfo();
    iPecuFirstWarnVideoIdx = GetPecuFirstWarnVideoIdx();

    memset(aiWarnVideoIdx,0,sizeof(aiWarnVideoIdx));
    memset(aiDoorWarnVideoIdx,0xff,sizeof(aiDoorWarnVideoIdx));
    memset(aiDoorClipWarnVideoIdx,0xff,sizeof(aiDoorClipWarnVideoIdx));

    if(memcmp(acFireWarnInfo,m_acFireWarnInfo,sizeof(acFireWarnInfo))
        || memcmp(acDoorWarnInfo,m_acDoorWarnInfo,sizeof(acDoorWarnInfo))
        || memcmp(acDoorClipWarnInfo,m_acDoorClipWarnInfo,sizeof(acDoorClipWarnInfo))
        || iPecuWarnInfo != m_iPecuInfo)
    {
        iChanged = 1;
        memcpy(m_acFireWarnInfo,acFireWarnInfo,sizeof(acFireWarnInfo));
        memcpy(m_acDoorWarnInfo,acDoorWarnInfo,sizeof(acDoorWarnInfo));
        memcpy(m_acDoorClipWarnInfo,acDoorClipWarnInfo,sizeof(acDoorClipWarnInfo));
        m_iPecuInfo = iPecuWarnInfo;
    }
    if(iChanged)
    {
        int iCount = 0;
        int iDoorClipCount = 0;
        int iFirst = 1;
        g_iWarnFreshed = 1;

        if(iPecuFirstWarnVideoIdx >=0)
        {
            aiWarnVideoIdx[iWarnNum] = iPecuFirstWarnVideoIdx;	//最先PECU报警的放最前
            iWarnNum ++;
        }

        for(int i=0;i<24;i++)
        {
            m_pBoxPecu[i]->hide();
            m_pBoxDoor[i]->hide();  //门禁的也在这里影藏掉
            m_pBoxDoorClip[i]->hide();
            if(iPecuWarnInfo & (0x1 << i))
            {
                char acBuf[8] = {0};
                memset(acBuf,0,sizeof(acBuf));
                int iVideoIdx = GetPecuVideoIndex(i);
                int iHaveExist = 0;  //避免报警相机重复

                GetVideoName(iVideoIdx, acBuf, sizeof(acBuf));

                for(int j=0;j<iWarnNum;j++)
                {
                    if(aiWarnVideoIdx[j] == iVideoIdx)
                    {
                        iHaveExist =1;
                        break;
                    }
                }
                if(!iHaveExist)  //之前没有重复的
                {
                    m_aiPecuIdx[iCount] = iVideoIdx;
                    aiWarnVideoIdx[iWarnNum] = iVideoIdx;
                    iWarnNum ++;
                    m_pBoxPecu[iCount]->setText(QString(acBuf));
                    m_pBoxPecu[iCount]->show();
                    iCount ++; //pecu报警数加1

                }
                //最前的PECU报警也要加进来
                else if(iVideoIdx == iPecuFirstWarnVideoIdx && iFirst)
                {
                    m_aiPecuIdx[iCount] = iVideoIdx;
                    m_pBoxPecu[iCount]->setText(QString(acBuf));
                    m_pBoxPecu[iCount]->show();
                    iCount ++;
                    iFirst = 0;
                }

            }
        }
        m_iPecuCount = iCount;

        iCount = 0;
        for(int i=0;i<6;i++)
        {
            m_pBoxFire[i]->hide();
            if(acFireWarnInfo[i])
            {
                char acBuf[8] = {0};
                memset(acBuf,0,sizeof(acBuf));
                int iVideoIdx = GetFireWarnVideoIdx(i);
                int iHaveExist = 0;

                GetVideoName(iVideoIdx, acBuf, sizeof(acBuf));
                m_pBoxFire[iCount]->setText(QString(acBuf));
                m_pBoxFire[iCount]->show();
                m_aiFireIdx[iCount] = iVideoIdx;
                iCount ++;

                for(int i=0;i<iWarnNum;i++)
                {
                    if(aiWarnVideoIdx[i] == iVideoIdx)
                    {
                        iHaveExist =1;
                        break;
                    }
                }
                if(!iHaveExist)
                {
                    aiWarnVideoIdx[iWarnNum] = iVideoIdx;
                    iWarnNum ++;
                }
            }
        }

        m_iFireCount = iCount;

        iCount = 0;
        for(int i=0;i<6;i++)
            for(int j=0;j<8;j++)
        {
            if(acDoorWarnInfo[i] & (0x01<<j))
            {
                char acBuf[8] = {0};
                memset(acBuf,0,sizeof(acBuf));
                int iHaveExist = 0;
                int iVideoIdx = GetDoorWarnVideoIdx(i,j);

                for(int i=0;i<iCount;i++)
                {
                    if(aiDoorWarnVideoIdx[i] == iVideoIdx)
                    {
                        iHaveExist =1;
                        break;
                    }
                }
                if(!iHaveExist)
                {
                    GetVideoName(iVideoIdx, acBuf, sizeof(acBuf));
                    m_pBoxDoor[iCount]->setText(QString(acBuf));
                    m_pBoxDoor[iCount]->show();
                    aiDoorWarnVideoIdx[iCount] = iVideoIdx;
                    m_aiDoorIdx[iCount] = iVideoIdx;
                    iCount ++;

                    for(int i=0;i<iWarnNum;i++)
                    {
                        if(aiWarnVideoIdx[i] == iVideoIdx)
                        {
                            iHaveExist =1;
                            break;
                        }
                    }
                    if(!iHaveExist)
                    {
                        aiWarnVideoIdx[iWarnNum] = iVideoIdx;
                        iWarnNum ++;
                    }
                }

            }

            if(acDoorClipWarnInfo[i] & (0x01<<j))
            {
                char acBuf[8] = {0};
                memset(acBuf,0,sizeof(acBuf));
                int iHaveExist = 0;
                int iVideoIdx = GetDoorWarnVideoIdx(i,j);

                for(int i=0;i<iDoorClipCount;i++)
                {
                    if(aiDoorClipWarnVideoIdx[i] == iVideoIdx)
                    {
                        iHaveExist =1;
                        break;
                    }
                }
                if(!iHaveExist)
                {
                    GetVideoName(iVideoIdx, acBuf, sizeof(acBuf));
                    m_pBoxDoorClip[iDoorClipCount]->setText(QString(acBuf));
                    m_pBoxDoorClip[iDoorClipCount]->show();
                    aiDoorClipWarnVideoIdx[iDoorClipCount] = iVideoIdx;
                    m_aiDoorClipIdx[iDoorClipCount] = iVideoIdx;
                    iDoorClipCount ++;

                    for(int i=0;i<iWarnNum;i++)
                    {
                        if(aiWarnVideoIdx[i] == iVideoIdx)
                        {
                            iHaveExist =1;
                            break;
                        }
                    }
                    if(!iHaveExist)
                    {
                        aiWarnVideoIdx[iWarnNum] = iVideoIdx;
                        iWarnNum ++;
                    }

                }
            }
        }
        m_iDoorCount = iCount;
        m_iDoorClipCount = iDoorClipCount;

        if(iWarnNum >1)
        {
            int iFreshed = 0;

            if(g_iVideoCycleFlag)
            {
                closeVideoCyc();
            }
            if(E_SINGLE_VPLAY == g_eCurPlayStyle)
            {
                ui->signalBUtton->setStyleSheet("QPushButton{border-image: url(:/res/btn_02_hig.png)}");
                ui->fourpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_01_nor.png)}");
                g_eNextPlayStyle = E_FOUR_VPLAY;
                g_iNextSingleVideoIdx  = -1;

            }

            //先找出不需要动的
            for(int i=0;i<4;i++)
            {
                int iStillWarn = 0;
                iChanged = 0;

                if(g_aiCurFourVideoIdx[i] != -1)
                {
                    for(int j=0;j<iWarnNum;j++)
                    {
                        if(aiWarnVideoIdx[j] == g_aiCurFourVideoIdx[i])
                        {
                            g_aiNextFourVideoIdx[i] = aiWarnVideoIdx[j];
                            aiWarnVideoIdx[j] = -1;
                            iStillWarn =1;
                            break;
                        }
                    }
                }
                if(0 == iStillWarn)
                {
                    g_aiNextFourVideoIdx[i] = -1;
                }

            }

            //再将剩下的报警相机放到队列中
            for(int i=0;i<4;i++)
            {
                if(-1 == g_aiNextFourVideoIdx[i])
                {
                    for(int j=0;j<iWarnNum;j++)
                    {
                        if(-1 != aiWarnVideoIdx[j])
                        {
                            g_aiNextFourVideoIdx[i] =  aiWarnVideoIdx[j];
                            aiWarnVideoIdx[j] = -1;
                            break;
                        }
                    }
                }

                if(g_aiNextFourVideoIdx[i] != g_aiCurFourVideoIdx[i])
                {
                    iFreshed = 1;
                }
            }

            if(-1 != aiWarnVideoIdx[0])
            {
                g_aiNextFourVideoIdx[0] = aiWarnVideoIdx[0];
                iFreshed = 1;
            }

            //只要有一个发生了变化 都需要重新调出四界面
            if(iFreshed)
            {

                g_iNextSingleVideoIdx = -1;
            }
        }
        else if(1 == iWarnNum)
        {
            int iVideoIndex = aiWarnVideoIdx[0];

            if(g_iVideoCycleFlag)
            {
                closeVideoCyc();
            }
            if(E_FOUR_VPLAY == g_eCurPlayStyle)
            {
                ui->signalBUtton->setStyleSheet("QPushButton{border-image: url(:/res/btn_02_nor.png)}");
                ui->fourpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_01_hig.png)}");

                g_eNextPlayStyle = E_SINGLE_VPLAY;

            }
            g_iNextSingleVideoIdx = iVideoIndex;
        }
        else
        {

            if(E_SINGLE_VPLAY == g_eCurPlayStyle)
            {
                if(-1 == g_iCurSingleVideoIdx)
                {
                    g_iNextSingleVideoIdx = 0;

                }
            }
            else
            {
                for(int i=0;i<4;i++)
                {
                    g_aiNextFourVideoIdx[i] = GetVideoIdxAccordBtnPose(1,i);
                }
                g_iNextSingleVideoIdx = -1;
            }
        }

        g_iWarnNum = iWarnNum;
        g_iWarn = iWarnNum >0?1:0;

    }
    return;
}

void cctv::timeupdateSlot()
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

void cctv::showcctvPage()
{
    this->show();
    if(E_FOUR_VPLAY == g_eCurPlayStyle)
    {
        char acUrl[256] = {0};
        for(int i =0;i<4;i++)
        {
            m_playWidget[i]->show();
            if(g_pHplay[i] == NULL)
            {

                GetVideoRtspUrl(g_aiNextFourVideoIdx[i] ,acUrl,sizeof(acUrl));

                cmplayMultiInit(m_playWidget[i], i);
                g_pHplay[i] = CMP_Init(m_RealMonitorVideos[i],CMP_VDEC_NORMAL);

                CMP_OpenMediaPreview(g_pHplay[i], acUrl, CMP_TCP);
                CMP_PlayMedia(g_pHplay[i]);
                CMP_SetDisplayEnable(g_pHplay[i],1);
                m_nodes[i].empty();
            }

        }

          //此时为4画面显示
        g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;
    }
    else
    {
        char acUrl[256] = {0};

        m_playSingleWidget->show();
        if(g_hSinglePlay == NULL)
        {
            GetVideoMainRtspUrl(g_iNextSingleVideoIdx ,acUrl,sizeof(acUrl));

            cmplayInit(m_playSingleWidget);
            g_hSinglePlay = CMP_Init(m_RealMonitorSingleVideo,CMP_VDEC_NORMAL);

            CMP_OpenMediaPreview(g_hSinglePlay, acUrl, CMP_TCP);
            CMP_PlayMedia(g_hSinglePlay);
            CMP_SetDisplayEnable(g_hSinglePlay,1);
             single_node.empty();
        }

    }


    g_iNeedUpdateWarnIcon = 1;

    if(g_iVideoCycleFlag)
    {
        videoPollingfunction();
    }

    usleep(1*1000);
    pollTimer->start();


}


void cctv::showMonitorManagePage()
{

    pollTimer->stop();

    if(E_FOUR_VPLAY == g_eCurPlayStyle)
    {
       for(int i =0;i<4;i++)
       {
           if(g_pHplay[i])
           {
               CMP_CloseMedia(g_pHplay[i]);
               CMP_UnInit(g_pHplay[i]);
               g_pHplay[i] = NULL;
           }
      }

    }
    else
    {

       if(g_hSinglePlay)
       {
            CMP_CloseMedia(g_hSinglePlay);
            CMP_UnInit(g_hSinglePlay);
            g_hSinglePlay = NULL;

       }

   }


    ui->monitorManageButton->setEnabled(false);
    if(mpageChangeTimer == NULL)
    {
        mpageChangeTimer = new QTimer(this);
        mpageChangeTimer->start(2*1000);
        connect(mpageChangeTimer,SIGNAL(timeout()), this,SLOT(mPageChangeSLot()));
    }


    this->hide();
    emit showMonitorSignal();


}


void cctv::mPageChangeSLot()
{

    ui->monitorManageButton->setEnabled(true);

    if (mpageChangeTimer != NULL)
    {
        if (mpageChangeTimer->isActive())
        {
            mpageChangeTimer->stop();
        }
        delete mpageChangeTimer;
        mpageChangeTimer = NULL;
    }

}

void cctv::sigalePageSlot()
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

        if(g_iCurSingleVideoIdx != -1)
        {
            g_iNextSingleVideoIdx = g_iCurSingleVideoIdx;
        }
        else
        {
            for(int i=0;i<4;i++)
            {
                if(g_aiCurFourVideoIdx[i] != -1)
                {
                    g_iNextSingleVideoIdx = g_aiCurFourVideoIdx[i];
                    break;
                }
                g_aiNextFourVideoIdx[i] = -1;
            }
        }
        if(-1 == g_iNextSingleVideoIdx)
        {
            g_iNextSingleVideoIdx = 0;
        }
        if(g_eCurPlayStyle != g_eNextPlayStyle)
        {
            printf("g_eCurPlayStyle != g_eNextPlayStyle---%d \n", __LINE__);
            PlayStyleChangedfunc();
            g_eCurPlayStyle = g_eNextPlayStyle;
            g_iNeedUpdateWarnIcon = 1;
        }

        UpdateCamStatefunc();
        return ;

    }


}

void cctv::fourPageSlot()
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
        int iGroup =-1,iPos =0;

        ui->fourpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_01_nor.png)}");
        ui->signalBUtton->setStyleSheet("QPushButton{border-image: url(:/res/btn_02_hig.png)}");
        g_eNextPlayStyle = E_FOUR_VPLAY;

        if(-1 != g_iCurSingleVideoIdx)
        {
            GetBtnPoseAccordVideoIdx(g_iCurSingleVideoIdx, &iGroup, &iPos);
        }

        if(iGroup <0 || (7== iGroup && GetVideoNum()<=28))
        {
            iGroup=0;
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

        if(g_eCurPlayStyle != g_eNextPlayStyle)
        {
            printf("g_eCurPlayStyle != g_eNextPlayStyle---%d \n", __LINE__);
            PlayStyleChangedfunc();
            g_eCurPlayStyle = g_eNextPlayStyle;
            g_iNeedUpdateWarnIcon = 1;
        }

        UpdateCamStatefunc();
        return ;

    }


}

void cctv::closeVideoCyc()
{
    if(g_iVideoCycleFlag == 1)
    {
        g_iVideoCycleFlag =0;

    }

    ui->openPollButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_03_hig.png)}");
    ui->closePollButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_04_nor.png)}");
}


void cctv::cycleSlot()
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
    if(Sender->objectName() == "openPollButton")
    {
        if(!g_iVideoCycleFlag)
        {
            g_iVideoCycleFlag = 1;
            if(E_FOUR_VPLAY == g_eCurPlayStyle)
            {
                g_iNextSingleVideoIdx = -1; //此时不能单画面显示了
            }

        }

        ui->openPollButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_03_nor.png)}");
        ui->closePollButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_04_hig.png)}");

    }
    else if(g_iVideoCycleFlag == 1)
    {
        closeVideoCyc();

    }

}

