#ifndef CCTVTEST_H
#define CCTVTEST_H

#include <QWidget>
#include <QPushButton>
#include "QTimer"
#include <QButtonGroup>
#include <QWidget>
#include "types.h"
#include "./include/CMPlayerInterface.h"
#define MAX_SERVER_NUM 6    //最大服务器个数
typedef  unsigned long PMSG_HANDLE;

QT_BEGIN_NAMESPACE
namespace Ui { class cctv; }
QT_END_NAMESPACE

class cctv : public QWidget
{
    Q_OBJECT

public:
    cctv(QWidget *parent = nullptr);
    ~cctv();
    void closeVideoCyc();
    void setUi();
    void cmplayInit(QWidget *g_widget);
    void cmplayMultiInit(QWidget *g_widget, int i);

    int GetNextFourVideo(int* piVideo);
    int GetNextSingleVideo(int *piVideo);


    void triggerGetDevStateSignal();
    void triggerSetTimeSignal();
//    void triggerWarnInfoSignal();
//    void triggerPlaySignal();
//    void triggerPollSignal();

    int FindCameBtnInfo(QAbstractButton* pbtn,int &iGroup,int &iNo);

    int FindDoorBtnInfo(QAbstractButton* pbtn);

    int FindDoorClipBtnInfo(QAbstractButton* pbtn);

    int FindBoxPecuBtnInfo(QAbstractButton* pbtn);

    int FindBoxFireBtnInfo(QAbstractButton* pbtn);

    bool eventFilter(QObject *target, QEvent *event);  //事件过滤器
    pthread_t monitorthread;
    int     m_iThreadRunFlag;

    QWidget *m_playWidget[4];
    QWidget *m_playSingleWidget;


    QPushButton     *videoGroupBtn[8][4];
    QButtonGroup    *g_fileButtonGroup;
    QButtonGroup    *g_videoNumbuttonGroup;
    QButtonGroup    *g_doorButtonGroup;
    QButtonGroup    *g_doorclipButtonGroup;
    QButtonGroup    *g_PecuButtonGroup;

    QIcon           pImageBtn[32][5];

    QPushButton     *m_pBoxFire[6];//烟火报警的图标
    QPushButton     *m_pBoxDoor[24];//门禁报警的图标
    QPushButton     *m_pBoxDoorClip[24];//门夹报警的图标
    QPushButton     *m_pBoxPecu[24];//PECU报警的图标
    char			m_acFireWarnInfo[6];	//烟火报警的值
    char			m_acDoorWarnInfo[6];	//门禁报警的值
    char            m_acDoorClipWarnInfo[6];//门夹报警的值
    int				m_iFireCount;		//门禁报警的数量
    int				m_iDoorCount;		//烟火报警的数量
    int				m_iDoorClipCount;	//门卡报警的数量
    int				m_iPecuCount;		//Pecu报警的数量
    int				m_aiFireIdx[6];		//门禁报警的相机序号
    int				m_aiDoorIdx[24];		//烟火报警的相机序号
    int				m_aiPecuIdx[24];		//Pecu报警的相机序号
    int             m_aiDoorClipIdx[24]; //门卡报警的相机序号
    unsigned short 	m_u16Year;
    unsigned char  	m_u8Mon;
    unsigned char	m_u8Day;
    int			  	m_iWeek;
    time_t 			m_tLastTime;		//播放按钮点击的最后时间  避免视频切换太快
    struct timeval  m_tPrevClickTime;   //全屏切换上次点击的时间 避免全屏非全屏切换太快
    time_t          tPollingOparateTime;  //轮询操作时间

    unsigned int    m_iPecuInfo;		//Pecu报警的值
    int m_iMousePosX;
    int m_iMousePosY;
    HWND                     m_RealMonitorVideos[8];
    HWND                     m_RealMonitorSingleVideo;

    PMSG_HANDLE     m_NvrServerPhandle[MAX_SERVER_NUM];    //nvr服务器PMSG通信句柄


    struct T_Node
    {
        int iStreamState;
        int iIndex;
        T_Node()
        {
            iStreamState = 0xffff;
            iIndex = 0;
        }
        void empty()
        {
            iStreamState = 0xffff;
            iIndex = 0;
        }

    };
    T_Node m_nodes[8];
    T_Node single_node;
    void UpdateCamStatefunc();
    void UpdateWarnBtnfunc();
    void PlayStyleChangedfunc();
    void SinglePlayStylefunc();
    void FourPlayStylefunc();
    void getPLayStrameState();
public slots:
    void showcctvPage();
    void showMonitorManagePage();
    void sigalePageSlot();
    void fourPageSlot();
    void cycleSlot();
    void timeupdateSlot();
    void PlayWidCicked(int index);
    void GroupButtonVideoClickSlot(QAbstractButton* btn);
    void GroupButtonFireSlot(QAbstractButton* btn);
    void GroupButtonDoorSlot(QAbstractButton *btn);
    void GroupButtonDoorclipSlot(QAbstractButton *btn);
    void GroupButtonPecuSlot(QAbstractButton *btn);
    void PlayCtrlFunSlot();

    void updateWarnInfoSLot();
    void videoPollingfunction();

    void mPageChangeSLot();


signals:
    void showMonitorSignal();
    void sendWindIndexSignal(int index);

    void getDevStateSignal();
    void setTimeSignal();


    void sendPLaySignal();
private:
    Ui::cctv *ui;
    QTimer *playTimer;
    QTimer *updateWarnTimer;
    QTimer *pollTimer;
    QTimer *mpageChangeTimer;

};
#endif // CCTVTEST_H
