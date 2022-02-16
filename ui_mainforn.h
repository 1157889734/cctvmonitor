/********************************************************************************
** Form generated from reading UI file 'mainforn.ui'
**
** Created by: Qt User Interface Compiler version 5.14.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINFORN_H
#define UI_MAINFORN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_mainforn
{
public:
    QPushButton *sysyPushButton;
    QPushButton *recpushButton;

    void setupUi(QWidget *mainforn)
    {
        if (mainforn->objectName().isEmpty())
            mainforn->setObjectName(QString::fromUtf8("mainforn"));
        mainforn->resize(1024, 768);
        mainforn->setStyleSheet(QString::fromUtf8(""));
        sysyPushButton = new QPushButton(mainforn);
        sysyPushButton->setObjectName(QString::fromUtf8("sysyPushButton"));
        sysyPushButton->setGeometry(QRect(0, 0, 511, 60));
        QFont font;
        font.setPointSize(16);
        sysyPushButton->setFont(font);
        sysyPushButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"background-color: rgb(23, 119, 244);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"	background-color: rgb(252, 233, 79);\n"
"}"));
        recpushButton = new QPushButton(mainforn);
        recpushButton->setObjectName(QString::fromUtf8("recpushButton"));
        recpushButton->setGeometry(QRect(512, 0, 512, 60));
        recpushButton->setFont(font);
        recpushButton->setStyleSheet(QString::fromUtf8("QPushButton{\n"
"background-color: rgb(23, 119, 244);\n"
"\n"
"}\n"
"\n"
"QPushButton:pressed{\n"
"	background-color: rgb(252, 233, 79);\n"
"}"));

        retranslateUi(mainforn);

        QMetaObject::connectSlotsByName(mainforn);
    } // setupUi

    void retranslateUi(QWidget *mainforn)
    {
        mainforn->setWindowTitle(QCoreApplication::translate("mainforn", "Form", nullptr));
        sysyPushButton->setText(QCoreApplication::translate("mainforn", "\347\263\273\347\273\237\347\256\241\347\220\206", nullptr));
        recpushButton->setText(QCoreApplication::translate("mainforn", "\345\275\225\345\203\217\347\256\241\347\220\206", nullptr));
    } // retranslateUi

};

namespace Ui {
    class mainforn: public Ui_mainforn {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINFORN_H
