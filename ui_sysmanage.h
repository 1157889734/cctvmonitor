/********************************************************************************
** Form generated from reading UI file 'sysmanage.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SYSMANAGE_H
#define UI_SYSMANAGE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_sysManage
{
public:
    QLabel *label;
    QLabel *label_2;
    QPushButton *restartButton;
    QPushButton *backButton;
    QTableWidget *devStatusTableWidget;
    QLabel *label_3;
    QTableWidget *devLogTableWidget;
    QLabel *yearLabel;
    QPushButton *LastYearButton;
    QPushButton *NextYearButton;
    QLabel *monthLabel;
    QPushButton *LastMonButton;
    QPushButton *NextMonButton;
    QLabel *dayLabel;
    QPushButton *LastDayButton;
    QPushButton *NextDayButton;
    QPushButton *searchSystermLogButton;
    QPushButton *searchWorkLogButton;
    QPushButton *LastPageButton;
    QPushButton *NextPageButton;

    void setupUi(QWidget *sysManage)
    {
        if (sysManage->objectName().isEmpty())
            sysManage->setObjectName(QString::fromUtf8("sysManage"));
        sysManage->resize(1024, 708);
        sysManage->setStyleSheet(QString::fromUtf8("#sysManage { \n"
"border-image: url(:/res/bg_system.png);} \n"
"#sysManage * { \n"
"border-image:url(); \n"
"}\n"
""));
        label = new QLabel(sysManage);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(200, 330, 91, 31));
        label->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        label_2 = new QLabel(sysManage);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(750, 330, 91, 31));
        label_2->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        restartButton = new QPushButton(sysManage);
        restartButton->setObjectName(QString::fromUtf8("restartButton"));
        restartButton->setGeometry(QRect(30, 590, 91, 41));
        QFont font;
        font.setPointSize(12);
        restartButton->setFont(font);
        restartButton->setStyleSheet(QString::fromUtf8("color: rgb(239, 41, 41);\n"
"background-color: rgb(252, 233, 79);"));
        backButton = new QPushButton(sysManage);
        backButton->setObjectName(QString::fromUtf8("backButton"));
        backButton->setGeometry(QRect(910, 660, 91, 41));
        backButton->setFont(font);
        backButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        devStatusTableWidget = new QTableWidget(sysManage);
        devStatusTableWidget->setObjectName(QString::fromUtf8("devStatusTableWidget"));
        devStatusTableWidget->setGeometry(QRect(20, 360, 482, 200));
        devStatusTableWidget->horizontalHeader()->setVisible(true);
        label_3 = new QLabel(sysManage);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(480, 20, 67, 17));
        label_3->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        devLogTableWidget = new QTableWidget(sysManage);
        devLogTableWidget->setObjectName(QString::fromUtf8("devLogTableWidget"));
        devLogTableWidget->setGeometry(QRect(10, 42, 713, 261));
        yearLabel = new QLabel(sysManage);
        yearLabel->setObjectName(QString::fromUtf8("yearLabel"));
        yearLabel->setGeometry(QRect(835, 50, 85, 38));
        QFont font1;
        font1.setPointSize(14);
        yearLabel->setFont(font1);
        yearLabel->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        yearLabel->setAlignment(Qt::AlignCenter);
        LastYearButton = new QPushButton(sysManage);
        LastYearButton->setObjectName(QString::fromUtf8("LastYearButton"));
        LastYearButton->setGeometry(QRect(750, 50, 75, 38));
        LastYearButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"border-image: url(:/res/time_dec.png);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"border-image: url(:/res/time_dec_pressed.png);\n"
"}"));
        NextYearButton = new QPushButton(sysManage);
        NextYearButton->setObjectName(QString::fromUtf8("NextYearButton"));
        NextYearButton->setGeometry(QRect(930, 50, 75, 38));
        NextYearButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"border-image: url(:/res/time_add.png);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"border-image: url(:/res/time_add_pressed.png);\n"
"}"));
        monthLabel = new QLabel(sysManage);
        monthLabel->setObjectName(QString::fromUtf8("monthLabel"));
        monthLabel->setGeometry(QRect(835, 100, 85, 38));
        monthLabel->setFont(font1);
        monthLabel->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        monthLabel->setAlignment(Qt::AlignCenter);
        LastMonButton = new QPushButton(sysManage);
        LastMonButton->setObjectName(QString::fromUtf8("LastMonButton"));
        LastMonButton->setGeometry(QRect(750, 100, 75, 38));
        LastMonButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"border-image: url(:/res/time_dec.png);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"border-image: url(:/res/time_dec_pressed.png);\n"
"}"));
        NextMonButton = new QPushButton(sysManage);
        NextMonButton->setObjectName(QString::fromUtf8("NextMonButton"));
        NextMonButton->setGeometry(QRect(930, 100, 75, 38));
        NextMonButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"border-image: url(:/res/time_add.png);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"border-image: url(:/res/time_add_pressed.png);\n"
"}"));
        dayLabel = new QLabel(sysManage);
        dayLabel->setObjectName(QString::fromUtf8("dayLabel"));
        dayLabel->setGeometry(QRect(835, 150, 85, 38));
        dayLabel->setFont(font1);
        dayLabel->setStyleSheet(QString::fromUtf8("color: rgb(255, 255, 255);"));
        dayLabel->setAlignment(Qt::AlignCenter);
        LastDayButton = new QPushButton(sysManage);
        LastDayButton->setObjectName(QString::fromUtf8("LastDayButton"));
        LastDayButton->setGeometry(QRect(750, 150, 75, 38));
        LastDayButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"border-image: url(:/res/time_dec.png);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"border-image: url(:/res/time_dec_pressed.png);\n"
"}"));
        NextDayButton = new QPushButton(sysManage);
        NextDayButton->setObjectName(QString::fromUtf8("NextDayButton"));
        NextDayButton->setGeometry(QRect(930, 150, 75, 38));
        NextDayButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"border-image: url(:/res/time_add.png);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"border-image: url(:/res/time_add_pressed.png);\n"
"}"));
        searchSystermLogButton = new QPushButton(sysManage);
        searchSystermLogButton->setObjectName(QString::fromUtf8("searchSystermLogButton"));
        searchSystermLogButton->setGeometry(QRect(750, 200, 108, 42));
        searchSystermLogButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        searchWorkLogButton = new QPushButton(sysManage);
        searchWorkLogButton->setObjectName(QString::fromUtf8("searchWorkLogButton"));
        searchWorkLogButton->setGeometry(QRect(900, 200, 108, 42));
        searchWorkLogButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        LastPageButton = new QPushButton(sysManage);
        LastPageButton->setObjectName(QString::fromUtf8("LastPageButton"));
        LastPageButton->setGeometry(QRect(750, 260, 108, 42));
        LastPageButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));
        NextPageButton = new QPushButton(sysManage);
        NextPageButton->setObjectName(QString::fromUtf8("NextPageButton"));
        NextPageButton->setGeometry(QRect(900, 260, 108, 42));
        NextPageButton->setStyleSheet(QString::fromUtf8("color: rgb(0, 0, 0);\n"
"background-color: rgb(70, 173, 223);"));

        retranslateUi(sysManage);

        QMetaObject::connectSlotsByName(sysManage);
    } // setupUi

    void retranslateUi(QWidget *sysManage)
    {
        sysManage->setWindowTitle(QCoreApplication::translate("sysManage", "Form", nullptr));
        label->setText(QCoreApplication::translate("sysManage", "   \350\256\276\345\244\207\347\212\266\346\200\201", nullptr));
        label_2->setText(QCoreApplication::translate("sysManage", "   \345\257\206\347\240\201\347\256\241\347\220\206", nullptr));
        restartButton->setText(QCoreApplication::translate("sysManage", "\351\207\215\345\220\257", nullptr));
        backButton->setText(QCoreApplication::translate("sysManage", "\350\277\224\345\233\236", nullptr));
        label_3->setText(QCoreApplication::translate("sysManage", "\346\227\245\345\277\227\347\256\241\347\220\206", nullptr));
        yearLabel->setText(QString());
        LastYearButton->setText(QString());
        NextYearButton->setText(QString());
        monthLabel->setText(QString());
        LastMonButton->setText(QString());
        NextMonButton->setText(QString());
        dayLabel->setText(QString());
        LastDayButton->setText(QString());
        NextDayButton->setText(QString());
        searchSystermLogButton->setText(QCoreApplication::translate("sysManage", "\346\237\245\347\234\213\347\263\273\347\273\237\346\227\245\345\277\227", nullptr));
        searchWorkLogButton->setText(QCoreApplication::translate("sysManage", "\346\237\245\347\234\213\346\223\215\344\275\234\346\227\245\345\277\227", nullptr));
        LastPageButton->setText(QCoreApplication::translate("sysManage", "\344\270\212\344\270\200\351\241\265", nullptr));
        NextPageButton->setText(QCoreApplication::translate("sysManage", "\344\270\213\344\270\200\351\241\265", nullptr));
    } // retranslateUi

};

namespace Ui {
    class sysManage: public Ui_sysManage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYSMANAGE_H
