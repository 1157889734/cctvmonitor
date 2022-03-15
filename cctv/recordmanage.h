#ifndef RECORDMANAGE_H
#define RECORDMANAGE_H
#include <QLabel>
#include <QWidget>
#include "myslider.h"
#include <QStyle>
#include <QStyleFactory>
#include "timeset.h"
#include "./pmsg/pmsgcli.h"
#include <QTimer>
#include "./log/log.h"

#include "debugout/debug.h"
#include "includeVdecc/CMPlayerInterface.h"
#include <QTableWidgetItem>

#define MAX_RECORD_SEACH_NUM 10000
#define MAX_RECFILE_PATH_LEN 256
#define MAX_SERVER_NUM 6    //最大服务器个数
typedef unsigned long PFTP_HANDLE;


namespace Ui {
class recordManage;
}

class recordManage : public QWidget
{
    Q_OBJECT

public:
    explicit recordManage(QWidget *parent = 0);
    ~recordManage();
    int pmsgCtrl(PMSG_HANDLE pHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen);     //与服务器通信消息处理
    void recordQueryCtrl(char *pcMsgData, int iMsgDataLen);
    void cmplaybackInit();
    void closePlayWin();

    void getTrainConfig();
    void recordPlayCtrl(int iRow, int iDex);
    void triggerSetRangeLabelSignal();
    void triggerSetSliderValueSignal(int iValue);   //触发设置播放进度条值的信号
    void triggerCloseRecordPlaySignal();
    void triggerDownloadProcessBarDisplaySignal(int iEnableFlag);   //触发是否显示文件下载进度条的信号，iEnableFlag为1，显示，为0不显示
    void triggerSetDownloadProcessBarValueSignal(int iValue);   //触发设置文件下载进度条的值的信号

    mySlider    *m_playSlider;    //播放进度条
    QStyle      *m_tableWidgetStyle;
    timeset     *timeSetWidget;    //时间设置控制窗体
    int         m_iPlayRange;    //录像文件总播放时长
    int         m_iPlayFlag;   //播放标志，0-暂停状态，未播放，1-在播放

    T_TIME_INFO m_tStartTime;
    T_TIME_INFO m_tEndTime;
    int		    m_iNvrNo;
    int		    m_iWaitFileCnt;
    double		m_dbSpeed;
    int			m_iTotolTime;
    int			m_iTimeCount;
    QWidget     *playWidget;
    T_WND_INFO                     m_RealMonitorVideos;
    PFTP_HANDLE m_tFtpHandle[MAX_SERVER_NUM];  //FTP句柄
    CMPHandle   m_cmpHandle;   //客户端媒体播放句柄
    int         m_iSliderValue;     //进度条当前值
    pthread_t   m_threadId;      //刷新进度条线程ID
    int         m_iThreadRunFlag;    //刷新进度条线程运行标识
    int         m_iFtpServerIdex;    //当前ftp服务器索引编号
    int         recordPlayFlag;
    double      m_dPlaySpeed;   //播放速度
    QLabel      *messageLable;

public slots:
    void hideRecPageSlots();
    void openStartTimeSetWidgetSlot();
    void openStopTimeSetWidgetSlot();
    void timeSetRecvMsg(QString year, QString month, QString day, QString hour, QString min, QString sec);
    void carNoChangeSlot();
    void SearchBtnClicked();
    void recordQueryEndSlot();
    void DownBtnClicked();
    void PrevBtnClicked();
    void SlowBtnClicked();
    void PlayBtnClicked();
    void PauesBtnClicked();
    void StopBtnClicked();
    void QuickBtnClicked();
    void NextBtnClicked();
    void recordTableWidgetFillSlot();
    void recordTableWidgetFillFunc();
    void recordSelectionSlot(QTableWidgetItem *item);
    void recordPlaySlot(QTableWidgetItem *item);

    void setPlaySliderValueSlot(int iValue);       //刷新进度条
    void closeRecordPlaySlot();
    void setRangeLabelSlot();

    void playSliderPressSlot(int iPosTime);
    void playSliderMoveSlot(int iPosTime);

    void downloadProcessBarDisplaySlot(int iEnableFlag);	//是否显示文件下载进度条，iEnableFlag为1，显示，为0不显示
    void setDownloadProcessBarValueSlot(int iValue);   //设置文件下载进度条的值

    void manualSwitchVideoEndSlot();
    void manualtableSwitchVideoEndSlot();
signals:
    void hideRecSysPage();
    void recordTableWidgetFillSignal();
    void setSliderValueSignal(int iValue);
    void setRangeLabelSignal();
    void closeRecordPlaySignal();
    void recordSeletPlay(QTableWidgetItem *item);
    void downloadProcessBarDisplaySignal(int iEnableFlag);
    void setDownloadProcessBarValueSignal(int iValue);

private:
    Ui::recordManage *ui;
    char *m_pcRecordFileBuf;
    char m_acFilePath[MAX_RECORD_SEACH_NUM][MAX_RECFILE_PATH_LEN];   //记录查询到的录像文件路径全名
    int m_iRecordIdex;
    PMSG_HANDLE m_Phandle[MAX_SERVER_NUM];    //服务器PMSG通信句柄
    int m_iTotalLen;
    QTimer *m_recordTabelWidgetFillTimer;
    QTimer *FileSearchTimer;
    QTimer *m_VideoSwitchTimer;
    QTimer *m_tableVideoSwitchTimer;


};

#endif // RECORDMANAGE_H
