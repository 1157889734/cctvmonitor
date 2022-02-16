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


public slots:
    void showMainfornPage();
    void menuButtonClick();
    void hidePageSlots();

signals:
    void sendhidesignal();


private:
    Ui::mainforn *ui;
    recordManage *g_recordManage;
    sysManage *g_sysManage;
};

#endif // MAINFORN_H
