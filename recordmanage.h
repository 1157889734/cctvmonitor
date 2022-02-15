#ifndef RECORDMANAGE_H
#define RECORDMANAGE_H

#include <QWidget>

namespace Ui {
class recordManage;
}

class recordManage : public QWidget
{
    Q_OBJECT

public:
    explicit recordManage(QWidget *parent = nullptr);
    ~recordManage();


public slots:
    void hideRecPageSlots();



signals:
    void hideRecSysPage();

private:
    Ui::recordManage *ui;
};

#endif // RECORDMANAGE_H
