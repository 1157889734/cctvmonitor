#ifndef MAINFORN_H
#define MAINFORN_H

#include <QWidget>
#include "sysmanage.h"
#include "recordmanage.h"

namespace Ui {
class mainforn;
}

class mainforn : public QWidget
{
    Q_OBJECT

public:
    explicit mainforn(QWidget *parent = nullptr);
    ~mainforn();
    void recvPmsgCtrl(PMSG_HANDLE pHandle, unsigned char ucMsgCmd, char *pcMsgData, int iMsgDataLen);

public slots:
    void showMainfornPage();
    void menuButtonClick();
    void hidePageSlots();
    void pmsgTimerFunc();
    void getDevStateSlot();

signals:
    void sendhidesignal();
    void sendDeviceSignal();

private:
    Ui::mainforn *ui;
    recordManage *g_recordManage;
    sysManage *g_sysManage;
    QTimer *m_PmsgTimer;

};

#endif // MAINFORN_H
