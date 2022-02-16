#include "playwind.h"
#include "ui_playwind.h"
#include <QMouseEvent>

playWind::playWind(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::playWind)
{
    ui->setupUi(this);
}

playWind::~playWind()
{
    delete ui;
}
void playWind::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        mousePos = QPoint(ev->x(),ev->y());
        emit sendClickSignal(mousePos);
    }

}
