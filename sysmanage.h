#ifndef SYSMANAGE_H
#define SYSMANAGE_H

#include <QWidget>
#include <QButtonGroup>
#include "state/state.h"

#define MAX_SERVER_NUM 6    //最大服务器个数
#define MAX_CAMERA_OFSERVER 4    //每个服务器连接的最大摄像机数


namespace Ui {
class sysManage;
}

class sysManage : public QWidget
{
    Q_OBJECT

public:
    explicit sysManage(QWidget *parent = 0);
    ~sysManage();
    void getTrainConfig();     //获取车型配置信息
    void setYear(int iYear);
    void setMonth(int iMonth);
    void setDay(int iDay);
    int pmsgCtrl(PMSG_HANDLE pHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen);     //与服务器通信消息处理
    void getNvrStatusCtrl(PMSG_HANDLE pHandle, char *pcMsgData);
    int getYear();
    int getMonth();
    int getDay();


    QButtonGroup *g_buttonGroup;
    pthread_t m_GetDevStatethreadId;      //

    T_NVR_STATE m_atNVRState[6];
    int  rows_per_page = 10;
    int cur_pages_index = 1 ; //当前所在页
    int total_pages = 1; //表格总页数
    int main_pages = 1; //主页页数


public slots:
    void hideSysPageSlots();
    void GroupButtonClickSlot(int index);
    void searchSystermLog();
    void lastpageSlot();
    void nextPageSlot();
    void getDevStateSignalCtrl();

signals:
    void hideSysPage();

private:
    Ui::sysManage *ui;
    int		    m_iYear;
    int		    m_iMonth;
    int		    m_iDay;
    int         m_aiServerIdex[MAX_SERVER_NUM];   //服务器在表中的行号索引，方便获取到服务器状态信息后刷新表对应的行，下标与m_Phandle是一一对应的
    PMSG_HANDLE m_NvrServerPhandle[MAX_SERVER_NUM];    //nvr服务器PMSG通信句柄
    int         m_aiNvrOnlineFlag[MAX_SERVER_NUM];   //服务器在线状态
    int         m_iCheckDiskErrFlag[MAX_SERVER_NUM];  //是否检查服务器硬盘错误标志
    int         m_iNoCheckDiskErrNum[MAX_SERVER_NUM];   //不检测服务器硬盘错误计数，每10秒加1

};

#endif // SYSMANAGE_H
