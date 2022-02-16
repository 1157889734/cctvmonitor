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
        devLogTableWidget->setGeometry(QRect(10, 42, 713, 251));

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
    } // retranslateUi

};

namespace Ui {
    class sysManage: public Ui_sysManage {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SYSMANAGE_H
