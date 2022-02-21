#include "cctvtest.h"
#include "./state/state.h"
#include "./debugout/debug.h"
#include "./log/log.h"
#include "./pmsg/pmsgcli.h"
#include "./pmsg/pmsgproc.h"
#include "mainforn.h"
#include "recordmanage.h"
#include "sysmanage.h"
#include "NVRMsgProc.h"


#include <QApplication>

#define LOG_FILE_DIR  "/home/data"

cctvTest *g_cctvtest = NULL;
mainforn *g_mainforn = NULL;
#define DEBUG_PORT 9880
static char g_acCCTVVersion[28] ={0};
static PMSG_HANDLE 	 g_hResUpdate;	   //资源更新的信号句柄
static int  g_aiNextFourVideoIdx[4] = {-1,-1,-1,-1};



static int GetTLCDSoftVersion(char *pcVersion,int iLen)
{
    FILE *fp = 0;
    int iStrLen = 0;

    memset(pcVersion,0,iLen);
    fp = fopen("/home/user/version.info", "rb");
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
    LOG_Init(LOG_FILE_DIR);    //本地日志模块初始化

    GetTLCDSoftVersion(g_acCCTVVersion,sizeof(g_acCCTVVersion));
    memset(&tLog,0,sizeof(tLog));
    tLog.iLogType = LOG_TYPE_SYS;
    sprintf(tLog.acLogDesc,"cctv %s start",g_acCCTVVersion);
    LOG_WriteLog(&tLog);

    DebugInit(DEBUG_PORT);    //调试信息模块初始化

    iRet = STATE_Init();
    if(0 != iRet)
    {

    }
    g_cctvtest = new cctvTest();
    g_cctvtest->show();

    g_mainforn = new mainforn();
//    g_mainforn->setStyleSheet("border-image: url();background-image:url(:/res/bg_system0.png)");
    g_mainforn->hide();


    NVR_init();
//    InitPmsgproc();

    g_hResUpdate = PMSG_CreateResConn(12016);

    for(int i=0;i<4;i++)
    {
        g_aiNextFourVideoIdx[i] = GetVideoIdxAccordBtnPose(1,i);
    }

    QObject::connect(g_cctvtest,SIGNAL(showMonitorSignal()),g_mainforn,SLOT(showMainfornPage()));
    QObject::connect(g_mainforn,SIGNAL(sendhidesignal()),g_cctvtest,SLOT(showcctvPage()));

    return a.exec();

    delete g_cctvtest;
    g_cctvtest = NULL;

    delete  g_mainforn;
    g_mainforn = NULL;

    UninitPmsgproc();
    NVR_Uninit();
    STATE_Uninit();
    DebugUninit();
    LOG_UnInit();

}
