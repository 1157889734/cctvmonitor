#include "sysmanage.h"
#include "ui_sysmanage.h"

sysManage::sysManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sysManage)
{
    ui->setupUi(this);
    connect(ui->backButton,SIGNAL(clicked()),this,SLOT(hideSysPageSlots()));
}

sysManage::~sysManage()
{
    delete ui;
}


void sysManage::hideSysPageSlots()
{
    this->hide();
    emit hideSysPage();
}
