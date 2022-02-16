#include "sysmanage.h"
#include "ui_sysmanage.h"
#include <QAbstractItemView>
#include <QTableWidget>

sysManage::sysManage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sysManage)
{
    ui->setupUi(this);
    this->setAutoFillBackground(true);

//    QPalette palette;
//    palette.setBrush(QPalette::Background,QBrush(QPixmap(":/res/bg_system.png")));
//    this->setPalette(palette);
    this->setStyleSheet("QWidget{background-image: url(:/res/bg_system.png)}");

    connect(ui->backButton,SIGNAL(clicked()),this,SLOT(hideSysPageSlots()));

    ui->devStatusTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->devStatusTableWidget->setColumnCount(6);
    ui->devStatusTableWidget->setRowCount(6);
    ui->devStatusTableWidget->setShowGrid(true);
    ui->devStatusTableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(252, 233, 79) }");

//    ui->devStorageTableWidget->setStyleSheet("QTableWidget{ gridline-color : rgb(255, 255, 255)}");
    QStringList header;
    header<<tr("设备位置")<<tr("硬盘个数")<<tr("硬盘容量")<<tr("使用量")<<tr("硬盘状态")<<tr("在线状态");
    ui->devStatusTableWidget->horizontalHeader()->setStyleSheet("background-color:white");
    ui->devStatusTableWidget->setHorizontalHeaderLabels(header);
    ui->devStatusTableWidget->horizontalHeader()->setVisible(true);//temp
    ui->devStatusTableWidget->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击
    ui->devStatusTableWidget->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    ui->devStatusTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->devStatusTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    ui->devStatusTableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中

    ui->devStatusTableWidget->horizontalHeader()->resizeSection(1,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(2,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(3,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(4,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(5,70);
    ui->devStatusTableWidget->horizontalHeader()->resizeSection(6,70);


    ui->devLogTableWidget->setFocusPolicy(Qt::NoFocus);
    ui->devLogTableWidget->setColumnCount(4);
//    ui->devLogTableWidget->setRowCount(6);
    ui->devLogTableWidget->setShowGrid(true);
    ui->devLogTableWidget->setStyleSheet("QTableWidget::item:selected { background-color: rgb(252, 233, 79) }");

    QStringList logheader;
    logheader<<tr("序号")<<tr("类型")<<tr("时间")<<tr("日志信息");
    ui->devLogTableWidget->horizontalHeader()->setStyleSheet("background-color:white");
    ui->devLogTableWidget->setHorizontalHeaderLabels(logheader);
    ui->devLogTableWidget->horizontalHeader()->setVisible(true);//temp
    ui->devLogTableWidget->horizontalHeader()->setSectionsClickable(false); //设置表头不可点击
    ui->devLogTableWidget->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
    ui->devLogTableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers); //设置不可编辑
    ui->devLogTableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);  //整行选中的方式
    ui->devLogTableWidget->setSelectionMode(QAbstractItemView::SingleSelection); //设置只能选择一行，不能多行选中

    ui->devLogTableWidget->horizontalHeader()->resizeSection(1,10);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(2,70);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(3,170);
    ui->devLogTableWidget->horizontalHeader()->resizeSection(4,430);



    getTrainConfig();

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
void sysManage::getTrainConfig()     //获取车型配置信息
{
    int i = 0, j = 0, row = 0;
    QString item = "";
    QString devStatus = tr("离线");     //设备状态初始默认值为离线
//    T_TRAIN_CONFIG tTrainConfigInfo;
    char tranNum[32] = {0};


    /*设备状态和设备存储列表清空*/
    row = ui->devStatusTableWidget->rowCount();
    for (i = 0; i < row; i++)
    {
        ui->devStatusTableWidget->removeRow(i);
    }
    ui->devStatusTableWidget->setRowCount(0);


    /*获取各服务器即摄像机信息，填充相应的列表控件*/
    for (i = 0; i < 6; i++)
    {
//        m_NvrServerPhandle[i] = STATE_GetNvrServerPmsgHandle(i);
        row = ui->devStatusTableWidget->rowCount();//获取表格中当前总行数
        ui->devStatusTableWidget->insertRow(row);//添加一行
//        m_aiServerIdex[i] = row+1;
        item = "";
        item = QString::number(row+1);
        item += "车NVR";
        ui->devStatusTableWidget->setItem(row, 0, new QTableWidgetItem(item));  //新建一个文本列并插入到列表中
        ui->devStatusTableWidget->item(row, 0)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);    //设置列文件对齐方式为居中对齐

        item = "0";
        item += "个";
        ui->devStatusTableWidget->setItem(row, 1, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 1)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);

        item = "";
        item = "0";
        item += tr("G");
        ui->devStatusTableWidget->setItem(row, 2, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 2)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);


        item = "";
        item = "0";
        item += tr("G");
        ui->devStatusTableWidget->setItem(row, 3, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 3)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);


        item = "";
        item = "离线";
        ui->devStatusTableWidget->setItem(row, 5, new QTableWidgetItem(item));
        ui->devStatusTableWidget->item(row, 5)->setTextAlignment(Qt::AlignLeft|Qt::AlignVCenter);


    }


}

