#ifndef RECORDMANAGE_H
#define RECORDMANAGE_H

#include <QWidget>
#include "myslider.h"
#include <QStyle>
#include <QStyleFactory>
#include "timeset.h"
#include "pmsg/pmsgcli.h"
#include <QTimer>

namespace Ui {
class recordManage;
}

class recordManage : public QWidget
{
    Q_OBJECT

public:
    explicit recordManage(QWidget *parent = 0);
    ~recordManage();

    void getTrainConfig();
    mySlider    *m_playSlider;    //播放进度条
    QStyle      *m_tableWidgetStyle;
    timeset     *timeSetWidget;    //时间设置控制窗体

    T_TIME_INFO m_tStartTime;
    T_TIME_INFO m_tEndTime;
    int		    m_iNvrNo;
    int		    m_iWaitFileCnt;
    double		m_dbSpeed;
    int			m_iTotolTime;
    int			m_iTimeCount;
    QTimer      *FileSearchTimer;
    QWidget     *playWidget;


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


signals:
    void hideRecSysPage();

private:
    Ui::recordManage *ui;
};

#endif // RECORDMANAGE_H
