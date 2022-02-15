#include "recordmanage.h"
#include "ui_recordmanage.h"

recordManage::recordManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::recordManage)
{
    ui->setupUi(this);
    connect(ui->backButton,SIGNAL(clicked()),this,SLOT(hideRecPageSlots()));
}

recordManage::~recordManage()
{
    delete ui;
}

void recordManage::hideRecPageSlots()
{
    this->hide();
    emit hideRecSysPage();
}

