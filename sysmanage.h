#ifndef SYSMANAGE_H
#define SYSMANAGE_H

#include <QWidget>

namespace Ui {
class sysManage;
}

class sysManage : public QWidget
{
    Q_OBJECT

public:
    explicit sysManage(QWidget *parent = nullptr);
    ~sysManage();

public slots:
    void hideSysPageSlots();



signals:
    void hideSysPage();

private:
    Ui::sysManage *ui;
};

#endif // SYSMANAGE_H
