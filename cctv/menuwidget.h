#ifndef MAINFORN_H
#define MAINFORN_H

#include <QWidget>
#include "sysmanage.h"
#include "recordmanage.h"

namespace Ui {
class menuwidget;
}

class menuwidget : public QWidget
{
    Q_OBJECT

public:
    explicit menuwidget(QWidget *parent = nullptr);
    ~menuwidget();
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
    Ui::menuwidget *ui;
    recordManage *g_recordManage;
    sysManage *g_sysManage;
    QTimer *m_PmsgTimer;

};

#endif // MAINFORN_H
