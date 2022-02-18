#ifndef SYSMANAGE_H
#define SYSMANAGE_H

#include <QWidget>
#include <QButtonGroup>


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

    int getYear();
    int getMonth();
    int getDay();

    QButtonGroup *g_buttonGroup;

public slots:
    void hideSysPageSlots();
    void GroupButtonClickSlot(int index);
    void searchSystermLog();
    void searchWorkLog();
    void lastpageSlot();
    void nextPageSlot();

signals:
    void hideSysPage();

private:
    Ui::sysManage *ui;
    int		   m_iYear;
    int		   m_iMonth;
    int		   m_iDay;
};

#endif // SYSMANAGE_H
