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
#include <QDebug>
#include <QMouseEvent>
#include <QEvent>
#include "vdec/cmplayer.h"
//#include "vdec/cmplayer.h"


typedef  void*  CMPHandle;
static   CMPHandle	g_pHplay[8] = {0};	  //最多只能同时存在4个播放句柄，多了会出问题
static int	g_iWarnNum = 0;

static E_PLAY_STYLE g_eCurPlayStyle = E_FOUR_VPLAY;   //当前播放风格
static E_PLAY_STYLE g_eNextPlayStyle = g_eCurPlayStyle;
static int	g_iWarn = 0;			//是否有报警
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
static int	g_iNeedUpdateWarnIcon = 0; //是否需要更新报警图标



void UpdateCamState(void *param)
{
    cctvTest *pCCTVWid = (cctvTest *)param;

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

        QIcon Nowicon = pCCTVWid->videoGroupBtn[iGroup][iPos]->icon();
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
            icon = iOnline ? pCCTVWid->pImageBtn[iImgIndex-1][0]:pCCTVWid->pImageBtn[iImgIndex-1][1];
        }
        else
        {
            icon = iOnline ? pCCTVWid->pImageBtn[iImgIndex-1][2]:pCCTVWid->pImageBtn[iImgIndex-1][3];
        }
        if(iOnline && iShelterWarn)
        {
            icon = pCCTVWid->pImageBtn[iImgIndex-1][4];
        }
        if(icon.name() != Nowicon.name())
        {
            pCCTVWid->videoGroupBtn[iGroup][iPos]->setIcon(icon);
            pCCTVWid->videoGroupBtn[iGroup][iPos]->hide();
            pCCTVWid->videoGroupBtn[iGroup][iPos]->show();
        }

    }

}

void UpdateWarnBtn(void *arg)
{
    cctvTest *pCCTVWid = (cctvTest *)arg;

    for(int i=0;i<pCCTVWid->m_iFireCount;i++)
    {
        pCCTVWid->m_pBoxFire[i]->hide();
        pCCTVWid->m_pBoxFire[i]->show();
    }
    for(int i=0;i<pCCTVWid->m_iDoorCount;i++)
    {
        pCCTVWid->m_pBoxDoor[i]->hide();
        pCCTVWid->m_pBoxDoor[i]->show();
    }

    for(int i=0;i<pCCTVWid->m_iDoorClipCount;i++)
    {
        pCCTVWid->m_pBoxDoorClip[i]->hide();
        pCCTVWid->m_pBoxDoorClip[i]->show();
    }

    for(int i=0;i<pCCTVWid->m_iPecuCount;i++)
    {
        pCCTVWid->m_pBoxPecu[i]->hide();
        pCCTVWid->m_pBoxPecu[i]->show();
    }


}

void SinglePlayStyle(void *arg)
{
    cctvTest *pCCTVWid = (cctvTest *)arg;
    if(g_iNextSingleVideoIdx != g_iCurSingleVideoIdx)
    {
        int iWarnIdx = -1;
        CMPHandle hTmp = NULL;  //保存当前相机的配置
        int iTmpCurIdx = -1;

        //判断当前单报警的相机是哪个
        //一直将其保存，目的是为了点击报警图标时能很快切回去
        if(1 == g_iWarnNum)
        {
           if(1 == pCCTVWid->m_iDoorCount)
           {
               iWarnIdx = pCCTVWid->m_aiDoorIdx[0];
           }
           else if(1 == pCCTVWid->m_iDoorClipCount)
           {
               iWarnIdx = pCCTVWid->m_aiDoorClipIdx[0];
           }
           else if(1 == pCCTVWid->m_iPecuCount)
           {
                iWarnIdx = pCCTVWid->m_aiPecuIdx[0];
           }
           else if(1 == pCCTVWid->m_iFireCount)
           {
                iWarnIdx = pCCTVWid->m_aiFireIdx[0];
           }
           g_iNeedUpdateWarnIcon = 1;
        }


        if(g_hSinglePlay)
        {
            //如果当前的不是报警索引
            if(iWarnIdx != g_iCurSingleVideoIdx)
            {
                CMP_CloseMedia(g_hSinglePlay);
//                CMP_DestroyMedia(g_hSinglePlay);
                CMP_UnInit(g_hSinglePlay);
                g_hSinglePlay = NULL;
                g_iCurSingleVideoIdx = -1;
            }
            else
            {
                iTmpCurIdx = g_iCurSingleVideoIdx;
                hTmp = g_hSinglePlay;
                g_hSinglePlay = NULL;
                g_iCurSingleVideoIdx = -1;
                CMP_SetPlayEnable(g_hSinglePlay,0);

//                CMP_SetWndDisplayEnable(hTmp,0,0);
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

                GetVideoMainRtspUrl(g_iNextSingleVideoIdx ,acUrl,sizeof(acUrl));
                strcpy(acUrl,"rtsp://admin:admin123@192.168.104.88:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"); //11
//                g_hSinglePlay = CMP_CreateMedia(pCCTVWid->m_pSinglePlayBg);
                g_hSinglePlay = CMP_Init(&pCCTVWid->m_RealMonitorVideos,CMP_VDEC_NORMAL);
                CMP_OpenMediaPreview(g_hSinglePlay, acUrl, CMP_TCP);
            }
            CMP_SetPlayEnable(g_hSinglePlay,1);

//            CMP_SetWndDisplayEnable(g_hSinglePlay,1,1);
        }

        if((hTmp != g_hBackSinglePlay) && hTmp )
       {
           if(g_hBackSinglePlay)
           {
               CMP_CloseMedia(g_hBackSinglePlay);
               CMP_UnInit(g_hBackSinglePlay);

//               CMP_DestroyMedia(g_hBackSinglePlay);
           }
           g_hBackSinglePlay = hTmp;
           g_iBackSingleVideoIdx = iTmpCurIdx;
       }
       g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;


    }

}
void FourPlayStyle(void *arg)
{
    cctvTest *pCCTVWid = (cctvTest *)arg;
    int iFourVideoChanged = 0;

    //单画面界面的视频发生了变化
    //从单到四 或从单到单 以及从四到单
    if(g_iNextSingleVideoIdx != g_iCurSingleVideoIdx)
    {
        if(g_hSinglePlay)   //切换时都需要把前面单播放句柄的先关掉
        {
            CMP_CloseMedia(g_hSinglePlay);
            CMP_UnInit(g_hSinglePlay);
//            CMP_DestroyMedia(g_hSinglePlay);
            g_hSinglePlay = NULL;
        }

        if(g_iNextSingleVideoIdx != -1)   	//接下来要播放的为单画面视频
        {
             char acUrl[256] = {0};
             GetVideoMainRtspUrl(g_iNextSingleVideoIdx,acUrl,sizeof(acUrl));
             strcpy(acUrl,"rtsp://admin:admin123@192.168.104.88:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"); //11
//             g_hSinglePlay = CMP_CreateMedia(pCCTVWid->m_pSinglePlayBg);
             g_hSinglePlay = CMP_Init(&pCCTVWid->m_RealMonitorVideos,CMP_VDEC_NORMAL);
             CMP_OpenMediaPreview(g_hSinglePlay, acUrl, CMP_TCP);
             CMP_SetPlayEnable(g_hSinglePlay,0);
//             CMP_SetWndDisplayEnable(g_hSinglePlay,0,0);
            pCCTVWid->m_playSingleWidget->show();
            for(int i=0;i<4;i++)
            {
                if(g_pHplay[i])
                {
                       //不需要播放的四画面视频关闭显示了
//                    CMP_SetWndDisplayEnable(g_pHplay[i],0,0);
                    CMP_SetPlayEnable(g_pHplay[i],0);

                }

                pCCTVWid->m_playWidget[i]->hide();
            }
            pCCTVWid->m_playSingleWidget->show();
            CMP_SetPlayEnable(g_hSinglePlay,1);

//            CMP_SetWndDisplayEnable(g_hSinglePlay,1,1);
        }
        else   //接下来播放的是四视频
        {
            pCCTVWid->m_playSingleWidget->hide();

            for(int i=0;i<4;i++)
            {
                pCCTVWid->m_playWidget[i]->show();

                if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
                {
                    if(g_pHplay[i])
                    {
                        CMP_CloseMedia(g_pHplay[i]);
                        CMP_UnInit(g_pHplay[i]);
    //                    CMP_DestroyMedia(g_pHplay[i]);
                        g_pHplay[i] = NULL;
                    }
                    g_aiCurFourVideoIdx[i] = -1;
                }
            }
            for(int i=0;i<4;i++)
            {
                pCCTVWid->m_playWidget[i]->show();

                if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
                {
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
                         if((iFindIdx != -1) &&g_pHplay[iFindIdx +4])
                         {
                            g_pHplay[i] = g_pHplay[iFindIdx +4];
                            g_pHplay[iFindIdx +4] = NULL;
                            g_aiBackFourVideoIdx[iFindIdx] = -1;
                         }
                         else
                         {
                              char acUrl[256] = {0};

                              GetVideoRtspUrl(g_aiNextFourVideoIdx[i] ,acUrl,sizeof(acUrl));
                              strcpy(acUrl,"rtsp://admin:admin123@192.168.104.88:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"); //11

                              g_pHplay[i] = CMP_Init(&pCCTVWid->m_RealMonitorVideos,CMP_VDEC_NORMAL);
    //                          g_pHplay[i] = CMP_CreateMedia(pCCTVWid->m_p4BgPlay[i]);
                              CMP_OpenMediaPreview(g_pHplay[i], acUrl, CMP_TCP);
                          }
                    }
                    g_aiCurFourVideoIdx[i] = g_aiNextFourVideoIdx[i];
                }
                for(int i=0;i<4;i++)
                {
                    if(g_pHplay[i])
                    {
                        CMP_SetPlayEnable(g_pHplay[i],1);
    //                    CMP_SetWndDisplayEnable(g_pHplay[i],1,1);
                    }
                    else
                    {
//                        int ix = pCCTVWid->m_playWidget[i]->x();
//                        int iy = pCCTVWid->m_playWidget[i]->y();
//                        int iw = pCCTVWid->m_playWidget[i]->width();
//                        int ih = pCCTVWid->m_playWidget[i]->height();

                        pCCTVWid->m_playWidget[i]->setStyleSheet("background-color: rgb(0,0,0)");

    //                    CMP_SetBlackBackground(ix ,iy ,iw ,ih);
                        //不足四个时候，背景要刷黑，因为报警按钮的背景是透明底色
                    }
                }
            }

        }

    g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;

    //在四画面与单画面切换时，有可能导致报警图标被隐藏
    g_iNeedUpdateWarnIcon =1;
    }
    //从四到四
   else if(-1 == g_iCurSingleVideoIdx )
   {

       for(int i=0;i<4;i++)
       {
          if(g_aiCurFourVideoIdx[i] != g_aiNextFourVideoIdx[i])
          {
               if(g_pHplay[i])
               {
                   CMP_CloseMedia(g_pHplay[i]);
                   CMP_UnInit(g_pHplay[i]);

//                   CMP_DestroyMedia(g_pHplay[i]);
                   g_pHplay[i] = NULL;
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
                   int iFindIdx = -1;

                  for(int j=0;j<4;j++)
                  {
                       if(g_aiBackFourVideoIdx[j] == g_aiNextFourVideoIdx[i])
                       {
                           iFindIdx = j;
                           break;
                       }
                  }
                  if((iFindIdx != -1) &&g_pHplay[iFindIdx +4])
                  {
                       g_pHplay[i] = g_pHplay[iFindIdx +4];
                       g_pHplay[iFindIdx +4] = NULL;
                       g_aiBackFourVideoIdx[iFindIdx] = -1;

                       CMP_SetPlayEnable(g_pHplay[i],1);
//                       CMP_SetWndDisplayEnable(g_pHplay[i],1,1);
                  }
                  else
                  {
                       char acUrl[256] = {0};

                       GetVideoRtspUrl(g_aiNextFourVideoIdx[i] ,acUrl,sizeof(acUrl));
                       strcpy(acUrl,"rtsp://admin:admin123@192.168.104.88:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"); //11

                       g_pHplay[i] = CMP_Init(&pCCTVWid->m_RealMonitorVideos,CMP_VDEC_NORMAL);
//                       g_pHplay[i] = CMP_CreateMedia(pCCTVWid->m_p4BgPlay[i]);
                       CMP_OpenMediaPreview(g_pHplay[i], acUrl, CMP_TCP);
                       CMP_SetPlayEnable(g_pHplay[i],1);
//                       CMP_SetWndDisplayEnable(g_pHplay[i],1,1);
                  }
               }
               g_aiCurFourVideoIdx[i] = g_aiNextFourVideoIdx[i];
               g_iNeedUpdateWarnIcon =1;
           }
       }
       if(g_iNeedUpdateWarnIcon)
       {
          for(int i=0;i<4;i++)
          {
             if(!g_pHplay[i])
             {
//               int ix = pCCTVWid->m_playWidget[i]->x();
//               int iy = pCCTVWid->m_playWidget[i]->y();
//               int iw = pCCTVWid->m_playWidget[i]->width();
//               int ih = pCCTVWid->m_playWidget[i]->height();

               pCCTVWid->m_playWidget[i]->setStyleSheet("background-color: rgb(0,0,0)");

//               CMP_SetBlackBackground(ix ,iy ,iw ,ih);
             }
           }
       }
   }


}
void PlayStyleChanged(void *arg)
{
    cctvTest *pCCTVWid = (cctvTest *)arg;
    if(g_eNextPlayStyle == E_FOUR_VPLAY)
    {
        //将之前的单画面播放句柄关掉
        if(g_hSinglePlay)
        {
            CMP_CloseMedia(g_hSinglePlay);
            CMP_UnInit(g_hSinglePlay);
//            CMP_DestroyMedia(g_hSinglePlay);
            g_hSinglePlay = NULL;
        }
        g_iCurSingleVideoIdx = -1;
        g_iNextSingleVideoIdx = g_iCurSingleVideoIdx;

        if(g_hBackSinglePlay)
        {
            CMP_CloseMedia(g_hBackSinglePlay);
//            CMP_DestroyMedia(g_hBackSinglePlay);
            CMP_UnInit(g_hSinglePlay);
            g_hBackSinglePlay = NULL;
        }
        g_iBackSingleVideoIdx = -1;
        pCCTVWid->m_playSingleWidget->hide();

        for(int i=0;i<4;i++)
        {
            if(g_pHplay[i])
            {
               CMP_CloseMedia(g_pHplay[i]);
               CMP_UnInit(g_pHplay[i]);

//               CMP_DestroyMedia(g_pHplay[i]);
               g_pHplay[i] = NULL;
            }
            if(g_pHplay[i+4])
            {
               CMP_CloseMedia(g_pHplay[i+4]);
//               CMP_DestroyMedia(g_pHplay[i+4]);
               CMP_UnInit(g_pHplay[i]);
               g_pHplay[i+4] = NULL;
            }
            g_aiCurFourVideoIdx[i] = -1;
            g_aiBackFourVideoIdx[i] = -1;
        }
        printf("*****************%s*****%d\n",__FUNCTION__,__LINE__);
        for(int i=0;i<4;i++)
        {
            int  iNextVideoIdx = g_aiNextFourVideoIdx[i];
            char acUrl[256] = {0};

            pCCTVWid->m_playWidget[i]->show();
            printf("*****************%s*****%d\n",__FUNCTION__,__LINE__);

            if(iNextVideoIdx != -1)
            {
                pCCTVWid->cmplayInit(pCCTVWid->m_playWidget[i]);
                GetVideoRtspUrl(iNextVideoIdx,acUrl,sizeof(acUrl));
                strcpy(acUrl,"rtsp://admin:admin123@192.168.104.88:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"); //11
                printf("*****************%s*****%d\n",__FUNCTION__,__LINE__);

//                g_pHplay[i] = CMP_CreateMedia(pCCTVWid->m_playWidget[i]);
                g_pHplay[i] = CMP_Init(&pCCTVWid->m_RealMonitorVideos,CMP_VDEC_NORMAL);
                CMP_OpenMediaFile(g_pHplay[i], acUrl, CMP_TCP);
                CMP_SetPlayEnable(g_pHplay[i],1);
            }
            else
            {
//                int ix = pCCTVWid->m_playWidget[i]->x();
//                int iy = pCCTVWid->m_playWidget[i]->y();
//                int iw = pCCTVWid->m_playWidget[i]->width();
//                int ih = pCCTVWid->m_playWidget[i]->height();

                //如果此时这个界面不需要显示视频，需要把此处背景刷黑
//                CMP_SetBlackBackground(ix ,iy ,iw ,ih);
                pCCTVWid->m_playWidget[i]->setStyleSheet("background-color: rgb(0,0,0)");
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
//            CMP_DestroyMedia(g_hBackSinglePlay);
            g_hBackSinglePlay = NULL;
        }
        g_iBackSingleVideoIdx = -1;

        for(int i=0;i<4;i++)
        {
            pCCTVWid->m_playWidget[i]->hide();
            if(g_pHplay[i])
            {
                CMP_CloseMedia(g_pHplay[i]);
                CMP_UnInit(g_pHplay[i]);
//                CMP_DestroyMedia(g_pHplay[i]);
                g_pHplay[i] = NULL;
                g_aiCurFourVideoIdx[i] = -1;
            }
            g_aiNextFourVideoIdx[i] = -1;

            if(g_pHplay[i+4])
            {
               CMP_CloseMedia(g_pHplay[i+4]);
               CMP_UnInit(g_pHplay[i+4]);
//               CMP_DestroyMedia(g_pHplay[i+4]);
               g_pHplay[i+4] = NULL;
               g_aiBackFourVideoIdx[i] = -1;
            }
        }
        pCCTVWid->m_playSingleWidget->show();

//        pCCTVWid->m_pSinglePlayBg->show();

        if(g_iCurSingleVideoIdx != g_iNextSingleVideoIdx)
        {
            char acUrl[256] = {0};

            if(g_hSinglePlay)
            {
                CMP_CloseMedia(g_hSinglePlay);
                CMP_UnInit(g_hSinglePlay);
//                CMP_DestroyMedia(g_hSinglePlay);
                g_hSinglePlay = NULL;
            }

            GetVideoMainRtspUrl(g_iNextSingleVideoIdx,acUrl,sizeof(acUrl));
            strcpy(acUrl,"rtsp://admin:admin123@192.168.104.88:554/cam/realmonitor?channel=1&subtype=0&unicast=true&proto=Onvif"); //11

            g_hSinglePlay = CMP_Init(&pCCTVWid->m_RealMonitorVideos,CMP_VDEC_NORMAL);

//            g_hSinglePlay = CMP_CreateMedia(pCCTVWid->m_pSinglePlayBg);
            CMP_OpenMediaFile(g_hSinglePlay, acUrl, CMP_TCP);
            CMP_SetPlayEnable(g_hSinglePlay,1);
        }
        else
        {
            CMP_SetPlayEnable(g_hSinglePlay,1);
//            CMP_SetWndDisplayEnable(g_hSinglePlay,1,1);
        }
        g_iCurSingleVideoIdx = g_iNextSingleVideoIdx;
    }
    g_eCurPlayStyle = g_eNextPlayStyle;


}


void PlayCtrlFun(void *arg)
{
    if(g_eCurPlayStyle != g_eNextPlayStyle)
    {
        PlayStyleChanged(arg);
        g_eCurPlayStyle = g_eNextPlayStyle;
        g_iNeedUpdateWarnIcon = 1;
    }

    if(E_FOUR_VPLAY == g_eCurPlayStyle)
    {
        FourPlayStyle(arg);
    }
    else
    {
        SinglePlayStyle(arg);
    }
    if(g_iNeedUpdateWarnIcon)  //界面切换时,有可能将报警图标给挡住，所以最好是重新显示一次
    {
        g_iNeedUpdateWarnIcon =0;
        UpdateWarnBtn(arg);
    }
    g_iWarnFreshed = 0;

}

void* updatePlaySlot(void *param)
{

    UpdateCamState(param);

    PlayCtrlFun(param);

    usleep(15*1000);

    return NULL;

}


cctvTest::cctvTest(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::cctvTest)
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
    m_playthreadId = 0;
    memset(&m_RealMonitorVideos, 0, sizeof(m_RealMonitorVideos));
    m_RealMonitorVideos.pRenderHandle = NULL;

    setUi();

    g_iCycTime = GetCycTime();

    connect(ui->monitorpushButton,SIGNAL(clicked()),this,SLOT(showMonitorPage()));
    connect(ui->signalBUtton,SIGNAL(clicked()),this,SLOT(sigalePageSlot()));
    connect(ui->fourpushButton,SIGNAL(clicked()),this,SLOT(fourPageSlot()));
    connect(ui->startpushButton,SIGNAL(clicked()),this,SLOT(cycleSlot()));
    connect(ui->stoppushButton,SIGNAL(clicked()),this,SLOT(cycleSlot()));

   connect(this,SIGNAL(sendWindIndexSignal(int)),this,SLOT(PlayWidCicked(int)));

    UpdateTimer = new QTimer(this);
    UpdateTimer->start(1000);
    connect(UpdateTimer,SIGNAL(timeout()),this,SLOT(timeupdateSlot()));


    pthread_create(&m_playthreadId, NULL, updatePlaySlot, (void *)this);    //创建监控线程

//    updatePlayTimer = new QTimer(this);
//    updatePlayTimer->start(150);
//    connect(updatePlayTimer,SIGNAL(timeout()),this,SLOT(updatePlaySlot()));

    updateWarnTimer = new QTimer(this);
    updateWarnTimer->start(400);
    connect(updateWarnTimer,SIGNAL(timeout()),this,SLOT(updateWarnInfoSLot()));


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

    for(int i = 0; i <4; i++)
    {
        delete m_playWidget[i];
        m_playWidget[i] = NULL;
    }

    delete m_playSingleWidget;
    m_playSingleWidget = NULL;

    delete ui;
}

void cctvTest::cmplayInit(QWidget *g_widget)
{

    if(m_RealMonitorVideos.pRenderHandle)
        return;

    QRect rt;
    QPoint pt;
    QWidget *pWnd = g_widget; //
    rt = pWnd->geometry();
    pt = pWnd->mapToGlobal(QPoint(0, 0));
    m_RealMonitorVideos.nVideoWidth = 0;
    m_RealMonitorVideos.nVideoHeight = 0;
    m_RealMonitorVideos.nX = rt.x();
    m_RealMonitorVideos.nY = rt.y();
    m_RealMonitorVideos.nWidth = rt.width();
    m_RealMonitorVideos.nHeight = rt.height();
    m_RealMonitorVideos.hWnd = (HWND)pWnd;
    m_RealMonitorVideos.pRenderHandle = NULL;

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

bool cctvTest::eventFilter(QObject *target, QEvent *event) //事件过滤器
{
    if (event->type()==QEvent::MouseButtonPress || event->type()==QEvent::MouseMove) //判断界面操作
    {
//            DebugPrint(DEBUG_UI_NOMAL_PRINT, "[%s] a mousemove or movebuttonpress or a keypress is checked!\n", __FUNCTION__);
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

//            qDebug()<<"******m_iMousePosX="<<m_iMousePosX<<"*************m_iMousePosY="<<m_iMousePosY<<__LINE__;

        if(event->type()==QEvent::MouseButtonPress)
        {
//                qDebug()<<"*****************"<<__LINE__;
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

void cctvTest::PlayWidCicked(int index)
{

//    if(send == "m_playWidget[0]")
    qDebug()<<"*************"<<index<<__LINE__;

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
        }
        else if(iPlayWidIndex >=0 && iPlayWidIndex <4)
        {
            if(g_pHplay[iPlayWidIndex] /*&& CMP_GetStreamState(g_pHplay[iPlayWidIndex])*/)
            {
                g_iNextSingleVideoIdx = g_aiCurFourVideoIdx[iPlayWidIndex];
            }
        }

    }
    m_tPrevClickTime = tNow;

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



    for(int i=0;i<32;i++)
    {
        memset(szData,0,sizeof(szData));
        snprintf(szData[0],sizeof(szData[0])-1,":/res/%d_act.png",i+1);
        snprintf(szData[1],sizeof(szData[0])-1,":/res/%d_act_nocon.png",i+1);
        snprintf(szData[2],sizeof(szData[0])-1,":/res/%d_noact.png",i+1);
        snprintf(szData[3],sizeof(szData[0])-1,":/res/%d_no.png",i+1);
        snprintf(szData[4],sizeof(szData[0])-1,":/res/%d_dis.png",i+1);

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
    videoGroupBtn[7][2] = ui->pushButton_6_7;
    videoGroupBtn[7][3] = ui->pushButton_6_8;

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

//            int iIndex = GetVideoIdxAccordBtnPose(i, j);
//            int iImgIndex = GetVideoImgIdx(iIndex);
//            if(iIndex >= 0 && iImgIndex <=32 && iImgIndex >=1)
            {

//                icon.addFile(QString::fromUtf8(pImageBtn[iImgIndex-1][3]),QSize(),QIcon::Normal,QIcon::Off);
                videoGroupBtn[i][j]->setIcon(pImageBtn[i][3]);
//                videoGroupBtn[i][j]->setIcon(pImageBtn[iImgIndex-1][3]);
                videoGroupBtn[i][j]->show();

            }
//            else
//            {
//                videoGroupBtn[i][j]->hide();

//            }

        }

        for(int j = 0; j < 4; j++)
        {
            videoGroupBtn[i][j]->installEventFilter(this);
            videoGroupBtn[i][j]->setMouseTracking(true);
        }

    }



    m_playWidget[0] = new playwidget();
    m_playWidget[0]->setParent(this);
    m_playWidget[0]->setGeometry(0,0,416,312);
    m_playWidget[0]->setObjectName("m_playWidget0");


    m_playWidget[1] = new playwidget();
    m_playWidget[1]->setParent(this);
    m_playWidget[1]->setGeometry(0,313,416,312);
    m_playWidget[1]->setObjectName("m_playWidget1");


    m_playWidget[2] = new playwidget();
    m_playWidget[2]->setParent(this);
    m_playWidget[2]->setGeometry(417,0,416,312);
    m_playWidget[2]->setObjectName("m_playWidget2");


    m_playWidget[3] = new playwidget();
    m_playWidget[3]->setParent(this);
    m_playWidget[3]->setGeometry(417,313,416,312);
    m_playWidget[3]->setObjectName("m_playWidget3");


    m_playSingleWidget =new playwidget();
    m_playSingleWidget->setParent(this);
    m_playSingleWidget->setGeometry(0,0,832,624);
    m_playSingleWidget->setObjectName("m_playWidget");



//    for(int i=0;i<4;i++)
//    {
//       connect(m_playWidget[i],SIGNAL(sendClickSignal(QObject*)),this,SLOT(PlayWidCicked(QObject*)));

//    }
//    connect(m_playSingleWidget,SIGNAL(sendClickSignal(QPoint)),this,SLOT(PlayWidCicked(QPoint)));



    for(int i = 0; i < 4; i++)
    {
        m_playWidget[i]->setStyleSheet("QWidget{background-color: rgb(0, 0, 0);}");     //设置播放窗口背景色为黑色
        m_playWidget[i]->installEventFilter(this);     //播放窗体注册进事件过滤器
        m_playWidget[i]->setMouseTracking(true);
        m_playWidget[i]->show();
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
        m_pBoxFire[i]= new QPushButton(this);
        m_pBoxFire[i]->setGeometry(20,10+50+i,100,45);
        icon.addFile(QString::fromUtf8(acImgFullName),QSize(),QIcon::Normal,QIcon::Off);
        m_pBoxFire[i]->setIcon(icon);
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
            m_pBoxDoor[i*12+j]->setGeometry(130+i*115,10+50*j,100,45);
            icon.addFile(QString::fromUtf8(acImgFullName),QSize(),QIcon::Normal,QIcon::Off);
            m_pBoxDoor[i*12+j]->setIcon(icon);
            m_pBoxDoor[i*12+j]->hide();

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


            g_doorButtonGroup->addButton(m_pBoxDoor[i*12+j]);
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


void cctvTest::updateWarnInfoSLot()
{
    char acFireWarnInfo[6] = {0};
    char acDoorWarnInfo[6] = {0};
    char acDoorClipWarnInfo[6] = {0};
    u32int	 iPecuWarnInfo = 0;
    int	 iPecuFirstWarnVideoIdx = -1;
    int  iWarnNum = 0;
    int  aiWarnVideoIdx[36] ={0};  //报警相机的序号  因为不同的报警类型 相机序号有重复所以要判断是否已存在啦
    int  iChanged = 0;
    char aiDoorWarnVideoIdx[48];
    char aiDoorClipWarnVideoIdx[48];

    GetAllDoorWarnInfo(acDoorWarnInfo, 6);
    GetAllFireWarnInfo(acFireWarnInfo, 6);
    GetAllDoorClipInfo(acDoorClipWarnInfo, 6);
    iPecuWarnInfo = GetPecuWarnInfo();
    iPecuFirstWarnVideoIdx = GetPecuFirstWarnVideoIdx();

    memset(aiDoorWarnVideoIdx,0xff,sizeof(aiDoorWarnVideoIdx));
    memset(aiDoorClipWarnVideoIdx,0xff,sizeof(aiDoorClipWarnVideoIdx));

    if(memcmp(acFireWarnInfo,m_acFireWarnInfo,sizeof(acFireWarnInfo))
        || memcmp(acDoorWarnInfo,m_acDoorWarnInfo,sizeof(acDoorWarnInfo))
        || memcmp(acDoorClipWarnInfo,m_acDoorClipWarnInfo,6)
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
//                        m_pBoxPecu[iCount]->copy_label(acBuf);
                        m_pBoxPecu[iCount]->setText(acBuf);
                        m_pBoxPecu[iCount]->show();
                        iCount ++; //pecu报警数加1

                    }
                    //最前的PECU报警也要加进来
                    else if(iVideoIdx == iPecuFirstWarnVideoIdx && iFirst)
                    {
                        m_aiPecuIdx[iCount] = iVideoIdx;
//                        m_pBoxPecu[iCount]->copy_label(acBuf);
                        m_pBoxPecu[iCount]->setText(acBuf);
                        m_pBoxPecu[iCount]->show();
                        iCount ++;
                        iFirst = 0;
                    }
                }
            }
            m_iPecuCount = iCount;

            iCount = 0;
            for(int i=0;i<6;i++)
                for(int j=0;j<8;j++)
            {
                if(acDoorWarnInfo[i] & (0x01<<j))
                {
                    char acBuf[8] = {0};
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
//                        m_pBoxDoor[iCount]->copy_label(acBuf);
                         m_pBoxDoor[iCount]->setText(acBuf);
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
//                        m_pBoxDoorClip[iDoorClipCount]->copy_label(acBuf);
                        m_pBoxDoor[iCount]->setText(acBuf);
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
//                    CloseVideoCyc(arg);
                }
                if(E_SINGLE_VPLAY == g_eCurPlayStyle)
                {
                    ui->signalBUtton->setStyleSheet("QPushButton{border-image: url(:/res/btn_02_hig.png)}");
                    ui->fourpushButton->setStyleSheet("QPushButton{border-image: url(:/res/btn_01_nor.png)}");

//                    pCCTVWid->m_pBtn2[0]->image(m_pImgBtn1[0]);
//                    pCCTVWid->m_pBtn2[0]->redraw();
//                    pCCTVWid->m_pBtn2[1]->image(m_pImgBtn1[3]);
//                    pCCTVWid->m_pBtn2[1]->redraw();
                    g_eNextPlayStyle = E_FOUR_VPLAY;
                    g_iNextSingleVideoIdx  = -1;
                }

                //先找出不需要动的
                for(int i=0;i<4;i++)
                {
                    int iStillWarn = 0;
                    int iChanged = 0;

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

    }

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

void cctvTest::showcctvPage()
{
    this->show();
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
        return ;

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
        return ;

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
    else if(g_iVideoCycleFlag == 1)
    {
        closeVideoCyc();

    }

}

