/********************************************************************************
** Form generated from reading UI file 'recordmanage.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RECORDMANAGE_H
#define UI_RECORDMANAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_recordManage
{
public:
    QPushButton *backButton;
    QLabel *label_3;
    QLabel *label_2;
    QLabel *label_4;
    QLabel *label_6;
    QLabel *label_7;
    QLabel *label_5;
    QComboBox *carSeletionComboBox;
    QComboBox *cameraSelectionComboBox;
    QLabel *label_8;
    QComboBox *videoComboBox;
    QLabel *label_9;
    QTableWidget *recordFileTableWidget;
    QPushButton *downloadButton;
    QPushButton *searchloadButton;
    QWidget *widget;
    QPushButton *playLastOnePushButton;
    QPushButton *slowForwardPushButton;
    QPushButton *playPushButton;
    QPushButton *pushButton;
    QPushButton *stopPushButton;
    QPushButton *fastForwardPushButton;
    QPushButton *playNextOnePushButton;
    QLabel *playSpeedlabel;
    QLabel *playMinLabel;
    QLabel *rangeMinLabel;
    QLabel *rangeSecLabel;
    QLabel *label_18;
    QLabel *playSecLabel;
    QLabel *label_15;
    QLabel *label_12;
    QPushButton *startTimeLabel;
    QPushButton *endTimeLabel;
    QProgressBar *fileDownloadProgressBar;

    void setupUi(QWidget *recordManage)
    {
        if (recordManage->objectName().isEmpty())
            recordManage->setObjectName(QString::fromUtf8("recordManage"));
        recordManage->resize(1024, 708);
        recordManage->setStyleSheet(QString::fromUtf8("#recordManage { \n"
"background-image: url(:/res/bg_system0.png);} \n"
"#recordManage * { \n"
"border-image:url(); \n"
"}\n"
""));
        backButton = new QPushButton(recordManage);
        backButton->setObjectName(QString::fromUtf8("backButton"));
        backButton->setGeometry(QRect(870, 660, 91, 41));
        QFont font;
        font.setPointSize(12);
        backButton->setFont(font);
        backButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        label_3 = new QLabel(recordManage);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(10, 50, 71, 30));
        label_3->setFont(font);
        label_3->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_2 = new QLabel(recordManage);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(10, 10, 101, 30));
        QFont font1;
        font1.setPointSize(18);
        label_2->setFont(font1);
        label_2->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_4 = new QLabel(recordManage);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(10, 90, 71, 30));
        label_4->setFont(font);
        label_4->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_6 = new QLabel(recordManage);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(20, 180, 141, 41));
        label_6->setFont(font);
        label_6->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_7 = new QLabel(recordManage);
        label_7->setObjectName(QString::fromUtf8("label_7"));
        label_7->setGeometry(QRect(170, 180, 151, 41));
        label_7->setFont(font);
        label_7->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_5 = new QLabel(recordManage);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(5, 150, 111, 33));
        label_5->setFont(font1);
        label_5->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        carSeletionComboBox = new QComboBox(recordManage);
        carSeletionComboBox->setObjectName(QString::fromUtf8("carSeletionComboBox"));
        carSeletionComboBox->setGeometry(QRect(80, 190, 81, 27));
        carSeletionComboBox->setFont(font);
        cameraSelectionComboBox = new QComboBox(recordManage);
        cameraSelectionComboBox->setObjectName(QString::fromUtf8("cameraSelectionComboBox"));
        cameraSelectionComboBox->setGeometry(QRect(240, 190, 76, 27));
        cameraSelectionComboBox->setFont(font);
        label_8 = new QLabel(recordManage);
        label_8->setObjectName(QString::fromUtf8("label_8"));
        label_8->setGeometry(QRect(20, 220, 91, 41));
        label_8->setFont(font);
        label_8->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        videoComboBox = new QComboBox(recordManage);
        videoComboBox->setObjectName(QString::fromUtf8("videoComboBox"));
        videoComboBox->setGeometry(QRect(110, 230, 131, 27));
        videoComboBox->setFont(font);
        label_9 = new QLabel(recordManage);
        label_9->setObjectName(QString::fromUtf8("label_9"));
        label_9->setGeometry(QRect(10, 270, 101, 41));
        label_9->setFont(font1);
        label_9->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        recordFileTableWidget = new QTableWidget(recordManage);
        if (recordFileTableWidget->columnCount() < 3)
            recordFileTableWidget->setColumnCount(3);
        QFont font2;
        font2.setPointSize(12);
        font2.setBold(false);
        font2.setWeight(50);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        __qtablewidgetitem->setTextAlignment(Qt::AlignCenter);
        __qtablewidgetitem->setFont(font2);
        __qtablewidgetitem->setBackground(QColor(255, 255, 255));
        recordFileTableWidget->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        __qtablewidgetitem1->setTextAlignment(Qt::AlignCenter);
        __qtablewidgetitem1->setFont(font2);
        __qtablewidgetitem1->setBackground(QColor(255, 255, 255));
        recordFileTableWidget->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        __qtablewidgetitem2->setTextAlignment(Qt::AlignCenter);
        __qtablewidgetitem2->setFont(font2);
        __qtablewidgetitem2->setBackground(QColor(255, 255, 255));
        recordFileTableWidget->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        recordFileTableWidget->setObjectName(QString::fromUtf8("recordFileTableWidget"));
        recordFileTableWidget->setGeometry(QRect(10, 310, 311, 321));
        recordFileTableWidget->setFont(font);
        recordFileTableWidget->setStyleSheet(QString::fromUtf8("border-style: none;\n"
"background-color: rgb(255, 255, 255);\n"
"gridline-color: rgb(130, 135, 144);\n"
"border-color: rgb(130, 135, 144);\n"
"border-width: 1px;border-style: solid;\n"
"\n"
""));
        recordFileTableWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        recordFileTableWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
        recordFileTableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        recordFileTableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        recordFileTableWidget->setShowGrid(false);
        recordFileTableWidget->setColumnCount(3);
        recordFileTableWidget->horizontalHeader()->setCascadingSectionResizes(false);
        recordFileTableWidget->horizontalHeader()->setDefaultSectionSize(100);
        recordFileTableWidget->horizontalHeader()->setProperty("showSortIndicator", QVariant(false));
        recordFileTableWidget->horizontalHeader()->setStretchLastSection(true);
        recordFileTableWidget->verticalHeader()->setVisible(false);
        downloadButton = new QPushButton(recordManage);
        downloadButton->setObjectName(QString::fromUtf8("downloadButton"));
        downloadButton->setGeometry(QRect(10, 640, 91, 41));
        downloadButton->setFont(font);
        downloadButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        searchloadButton = new QPushButton(recordManage);
        searchloadButton->setObjectName(QString::fromUtf8("searchloadButton"));
        searchloadButton->setGeometry(QRect(130, 265, 91, 41));
        searchloadButton->setFont(font);
        searchloadButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        widget = new QWidget(recordManage);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(340, 10, 681, 571));
        widget->setStyleSheet(QString::fromUtf8("background-color: rgb(0, 0, 0);"));
        playLastOnePushButton = new QPushButton(recordManage);
        playLastOnePushButton->setObjectName(QString::fromUtf8("playLastOnePushButton"));
        playLastOnePushButton->setGeometry(QRect(330, 620, 70, 35));
        playLastOnePushButton->setFont(font);
        playLastOnePushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        slowForwardPushButton = new QPushButton(recordManage);
        slowForwardPushButton->setObjectName(QString::fromUtf8("slowForwardPushButton"));
        slowForwardPushButton->setGeometry(QRect(420, 620, 70, 35));
        slowForwardPushButton->setFont(font);
        slowForwardPushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        playPushButton = new QPushButton(recordManage);
        playPushButton->setObjectName(QString::fromUtf8("playPushButton"));
        playPushButton->setGeometry(QRect(510, 620, 70, 35));
        playPushButton->setFont(font);
        playPushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        pushButton = new QPushButton(recordManage);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(600, 620, 70, 35));
        pushButton->setFont(font);
        pushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        stopPushButton = new QPushButton(recordManage);
        stopPushButton->setObjectName(QString::fromUtf8("stopPushButton"));
        stopPushButton->setGeometry(QRect(690, 620, 70, 35));
        stopPushButton->setFont(font);
        stopPushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        fastForwardPushButton = new QPushButton(recordManage);
        fastForwardPushButton->setObjectName(QString::fromUtf8("fastForwardPushButton"));
        fastForwardPushButton->setGeometry(QRect(780, 620, 70, 35));
        fastForwardPushButton->setFont(font);
        fastForwardPushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        playNextOnePushButton = new QPushButton(recordManage);
        playNextOnePushButton->setObjectName(QString::fromUtf8("playNextOnePushButton"));
        playNextOnePushButton->setGeometry(QRect(870, 620, 70, 35));
        playNextOnePushButton->setFont(font);
        playNextOnePushButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        playSpeedlabel = new QLabel(recordManage);
        playSpeedlabel->setObjectName(QString::fromUtf8("playSpeedlabel"));
        playSpeedlabel->setGeometry(QRect(950, 626, 67, 21));
        playSpeedlabel->setFont(font);
        playMinLabel = new QLabel(recordManage);
        playMinLabel->setObjectName(QString::fromUtf8("playMinLabel"));
        playMinLabel->setGeometry(QRect(917, 590, 21, 20));
        playMinLabel->setFont(font);
        rangeMinLabel = new QLabel(recordManage);
        rangeMinLabel->setObjectName(QString::fromUtf8("rangeMinLabel"));
        rangeMinLabel->setGeometry(QRect(970, 590, 21, 20));
        rangeMinLabel->setFont(font);
        rangeSecLabel = new QLabel(recordManage);
        rangeSecLabel->setObjectName(QString::fromUtf8("rangeSecLabel"));
        rangeSecLabel->setGeometry(QRect(995, 590, 21, 20));
        rangeSecLabel->setFont(font);
        label_18 = new QLabel(recordManage);
        label_18->setObjectName(QString::fromUtf8("label_18"));
        label_18->setGeometry(QRect(937, 590, 5, 17));
        playSecLabel = new QLabel(recordManage);
        playSecLabel->setObjectName(QString::fromUtf8("playSecLabel"));
        playSecLabel->setGeometry(QRect(942, 590, 21, 20));
        playSecLabel->setFont(font);
        label_15 = new QLabel(recordManage);
        label_15->setObjectName(QString::fromUtf8("label_15"));
        label_15->setGeometry(QRect(962, 590, 8, 17));
        label_12 = new QLabel(recordManage);
        label_12->setObjectName(QString::fromUtf8("label_12"));
        label_12->setGeometry(QRect(990, 590, 5, 17));
        startTimeLabel = new QPushButton(recordManage);
        startTimeLabel->setObjectName(QString::fromUtf8("startTimeLabel"));
        startTimeLabel->setGeometry(QRect(90, 50, 221, 31));
        startTimeLabel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(255, 255, 255);\n"
"border-color: rgb(0, 0, 0);\n"
"border-width: 1px;\n"
"border-style: solid;"));
        endTimeLabel = new QPushButton(recordManage);
        endTimeLabel->setObjectName(QString::fromUtf8("endTimeLabel"));
        endTimeLabel->setGeometry(QRect(90, 90, 221, 31));
        endTimeLabel->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(255, 255, 255);\n"
"border-color: rgb(0, 0, 0);\n"
"border-width: 1px;\n"
"border-style: solid;"));
        fileDownloadProgressBar = new QProgressBar(recordManage);
        fileDownloadProgressBar->setObjectName(QString::fromUtf8("fileDownloadProgressBar"));
        fileDownloadProgressBar->setGeometry(QRect(105, 645, 221, 31));
        fileDownloadProgressBar->setStyleSheet(QString::fromUtf8("QProgressBar {\n"
"	background-color: rgb(0, 193, 37);\n"
"   border: 1px solid grey;\n"
"   border-radius: 2px;\n"
"   background-color: #FFFFFF;\n"
"}\n"
" \n"
"QProgressBar::chunk {\n"
"   background-color: #4FEE9C;\n"
"   width: 20px;\n"
"}\n"
" \n"
"QProgressBar {\n"
"   border: 1px solid grey;\n"
"   border-radius: 2px;\n"
"   text-align: center;\n"
"}"));
        fileDownloadProgressBar->setValue(0);

        retranslateUi(recordManage);

        QMetaObject::connectSlotsByName(recordManage);
    } // setupUi

    void retranslateUi(QWidget *recordManage)
    {
        recordManage->setWindowTitle(QCoreApplication::translate("recordManage", "Form", nullptr));
        backButton->setText(QCoreApplication::translate("recordManage", "\350\277\224\345\233\236", nullptr));
        label_3->setText(QCoreApplication::translate("recordManage", " \350\265\267\345\247\213\346\227\245\346\234\237", nullptr));
        label_2->setText(QCoreApplication::translate("recordManage", "\345\233\236\346\224\276\346\227\266\351\227\264", nullptr));
        label_4->setText(QCoreApplication::translate("recordManage", " \347\273\223\346\235\237\346\227\245\346\234\237", nullptr));
        label_6->setText(QCoreApplication::translate("recordManage", " \350\275\246\345\216\242\345\217\267", nullptr));
        label_7->setText(QCoreApplication::translate("recordManage", "   \346\221\204\345\203\217\346\234\272", nullptr));
        label_5->setText(QCoreApplication::translate("recordManage", " \344\275\215\347\275\256\351\200\211\346\213\251", nullptr));
        label_8->setText(QCoreApplication::translate("recordManage", " \345\275\225\345\203\217\347\261\273\345\236\213", nullptr));
        label_9->setText(QCoreApplication::translate("recordManage", " \346\226\207\344\273\266\345\210\227\350\241\250", nullptr));
        QTableWidgetItem *___qtablewidgetitem = recordFileTableWidget->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QCoreApplication::translate("recordManage", "\351\200\211\344\270\255", nullptr));
        QTableWidgetItem *___qtablewidgetitem1 = recordFileTableWidget->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QCoreApplication::translate("recordManage", "\345\272\217\345\217\267", nullptr));
        QTableWidgetItem *___qtablewidgetitem2 = recordFileTableWidget->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QCoreApplication::translate("recordManage", "\350\247\206\351\242\221", nullptr));
        downloadButton->setText(QCoreApplication::translate("recordManage", "\344\270\213\350\275\275", nullptr));
        searchloadButton->setText(QCoreApplication::translate("recordManage", "\346\237\245\350\257\242", nullptr));
        playLastOnePushButton->setText(QCoreApplication::translate("recordManage", "\344\270\212\344\270\200\344\270\252", nullptr));
        slowForwardPushButton->setText(QCoreApplication::translate("recordManage", "\346\205\242\350\277\233", nullptr));
        playPushButton->setText(QCoreApplication::translate("recordManage", "\346\222\255\346\224\276", nullptr));
        pushButton->setText(QCoreApplication::translate("recordManage", "\346\232\202\345\201\234", nullptr));
        stopPushButton->setText(QCoreApplication::translate("recordManage", "\345\201\234\346\255\242", nullptr));
        fastForwardPushButton->setText(QCoreApplication::translate("recordManage", "\345\277\253\350\277\233", nullptr));
        playNextOnePushButton->setText(QCoreApplication::translate("recordManage", "\344\270\213\344\270\200\344\270\252", nullptr));
        playSpeedlabel->setText(QString());
        playMinLabel->setText(QCoreApplication::translate("recordManage", "00", nullptr));
        rangeMinLabel->setText(QCoreApplication::translate("recordManage", "00", nullptr));
        rangeSecLabel->setText(QCoreApplication::translate("recordManage", "00", nullptr));
        label_18->setText(QCoreApplication::translate("recordManage", ":", nullptr));
        playSecLabel->setText(QCoreApplication::translate("recordManage", "00", nullptr));
        label_15->setText(QCoreApplication::translate("recordManage", "/", nullptr));
        label_12->setText(QCoreApplication::translate("recordManage", ":", nullptr));
        startTimeLabel->setText(QString());
        endTimeLabel->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class recordManage: public Ui_recordManage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RECORDMANAGE_H
