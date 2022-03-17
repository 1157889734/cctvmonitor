#include "cctv.h"
#include "./state/state.h"
#include "./debugout/debug.h"
#include "./log/log.h"
#include "./pmsg/pmsgcli.h"
#include "./pmsg/pmsgproc.h"
#include "menuwidget.h"
#include "recordmanage.h"
#include "sysmanage.h"
#include "NVRMsgProc.h"
#include <QDebug>
//#include "msgapp.h"

#include <QApplication>

#define LOG_FILE_DIR  "/home/data"

cctv *g_cctvwidget = NULL;
menuwidget *g_menuwidget = NULL;
#define DEBUG_PORT 9880
static char g_acCCTVVersion[28] ={0};
static int  g_aiNextFourVideoIdx[4] = {-1,-1,-1,-1};



static int GetTLCDSoftVersion(char *pcVersion,int iLen)
{
    FILE *fp = 0;
    int iStrLen = 0;

    memset(pcVersion,0,iLen);
    fp = fopen("/home/data/version.info", "rb");
    if (NULL == fp)
    {
        printf("[%s]can not open file\n", __FUNCTION__);
        return -1;
    }
    fgets(pcVersion, iLen, fp);
    iStrLen = strlen(pcVersion);
    if (iStrLen > 0)
    {
        if ('\n' == pcVersion[iStrLen - 1])
        {
            pcVersion[iStrLen - 1] = 0;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    int iRet = 0;
    T_LOG_INFO tLog;
    char acNvrServerIp[128] = {0};
    LOG_Init(LOG_FILE_DIR);    //本地日志模块初始化

    GetTLCDSoftVersion(g_acCCTVVersion,sizeof(g_acCCTVVersion));
    memset(&tLog,0,sizeof(tLog));
    tLog.iLogType = LOG_TYPE_SYS;
    sprintf(tLog.acLogDesc,"cctv %s start",g_acCCTVVersion);
    LOG_WriteLog(&tLog);

    DebugInit(DEBUG_PORT);    //调试信息模块初始化

    STATE_Init();

    g_cctvwidget = new cctv();
    g_cctvwidget->show();

    g_menuwidget = new menuwidget();
    g_menuwidget->hide();


    for(int i = 0; i < 6; i++)
    {
        char acIp[24] = {0};
        memset(acIp, 0, sizeof(acIp));
        GetNvrIpAddr(i, acIp);
        if(acIp[0] == 0)
        {
            return -1;
        }
        iRet = PMSG_CreateConnect(acIp, 10100);
        if (0 == iRet)
        {
            DebugPrint(DEBUG_ERROR_PRINT, "create connection to server:%s error!\n",acNvrServerIp);
            continue;
        }
        if(iRet != 0)
        {
            if (STATE_SetNvrServerPmsgHandle(i, (PMSG_HANDLE)iRet) < 0)
            {
                DebugPrint(DEBUG_ERROR_PRINT, "save server:%s pmsg handle error!\n",acNvrServerIp);
            }

        }

    }

    InitPmsgproc();

    for(int i=0;i<4;i++)
    {
        g_aiNextFourVideoIdx[i] = GetVideoIdxAccordBtnPose(1,i);
    }

    QObject::connect(g_cctvwidget,SIGNAL(showMonitorSignal()),g_menuwidget,SLOT(showMainfornPage()));
    QObject::connect(g_menuwidget,SIGNAL(sendhidesignal()),g_cctvwidget,SLOT(showcctvPage()));
    QObject::connect(g_cctvwidget,SIGNAL(getDevStateSignal()),g_menuwidget,SLOT(getDevStateSlot()));

    return a.exec();

    delete g_cctvwidget;
    g_cctvwidget = NULL;

    delete  g_menuwidget;
    g_menuwidget = NULL;

    UninitPmsgproc();
    STATE_Uninit();
    DebugUninit();
    LOG_UnInit();

}
