#ifndef CCTVTEST_H
#define CCTVTEST_H

#include <QWidget>
#include <QPushButton>
#include "QTimer"
#include <QButtonGroup>
#include <QWidget>
#include "playwind.h"
#include "playwidget.h"
#include "types.h"

QT_BEGIN_NAMESPACE
namespace Ui { class cctvTest; }
QT_END_NAMESPACE

class cctvTest : public QWidget
{
    Q_OBJECT

public:
    cctvTest(QWidget *parent = nullptr);
    ~cctvTest();
    void closeVideoCyc();
    void setUi();
    void cmplayInit(QWidget *g_widget);

    int GetNextFourVideo(int* piVideo);
    int GetNextSingleVideo(int *piVideo);
//    void UpdateCamState();
//    void PlayCtrlFun();
    int FindCameBtnInfo(QAbstractButton* pbtn,int &iGroup,int &iNo);
//    void PlayStyleChanged();
//    void FourPlayStyle();
//    void SinglePlayStyle();
//    void UpdateWarnBtn();
//    bool eventFilter(QObject *target, QEvent *event);  //事件过滤器
    pthread_t m_backVideothreadId;      //
    pthread_t m_requesIpcthreadId;      //



//    playwidget *m_playWidget[4];
//    playwidget *m_playSingleWidget;

    QWidget *m_playWidget[4];
    QWidget *m_playSingleWidget;

    QPushButton *videoGroupBtn[8][4];
    QButtonGroup *g_fileButtonGroup;
    QButtonGroup *g_buttonGroup;
    QButtonGroup *g_doorButtonGroup;
    QButtonGroup *g_doorclipButtonGroup;
    QButtonGroup *g_PecuButtonGroup;

    QIcon  pImageBtn[32][5];

    QPushButton *m_pBoxFire[6];//烟火报警的图标
    QPushButton *m_pBoxDoor[24];//门禁报警的图标
    QPushButton *m_pBoxDoorClip[24];//门夹报警的图标
    QPushButton *m_pBoxPecu[24];//PECU报警的图标
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
    unsigned int    m_iPecuInfo;		//Pecu报警的值
    int m_iMousePosX;
    int m_iMousePosY;
    T_WND_INFO                     m_RealMonitorVideos;

    void UpdateCamState();
    void UpdateWarnBtn();
    void PlayStyleChangedfunc();
    void SinglePlayStylefunc();
    void FourPlayStylefunc();
public slots:
    void showcctvPage();
    void showMonitorPage();
    void sigalePageSlot();
    void fourPageSlot();
    void cycleSlot();
    void timeupdateSlot();
    void updateWarnInfoSLot();
    void PlayWidCicked(int index);
    void GroupButtonClickSlot(QAbstractButton* btn);
    void GroupButtonFireSlot(int index);
    void GroupButtonDoorSlot(int index);
    void GroupButtonDoorclipSlot(int index);
    void GroupButtonPecuSlot(int index);
//    void GetBackVideoTImerSlot();
    void playSlot();

signals:
    void showMonitorSignal();
    void sendWindIndexSignal(int index);
private:
    Ui::cctvTest *ui;
    QTimer *UpdateTimer;
    QTimer *updatePlayTimer;
    QTimer *updateWarnTimer;
    QTimer *playTimer;


};
#endif // CCTVTEST_H
