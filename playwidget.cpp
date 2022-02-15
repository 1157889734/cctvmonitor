#include "playwidget.h"
#include <QMouseEvent>

playwidget::playwidget(QWidget *parent) :
    QWidget(parent)
{
//    connect(this, SIGNAL(clicked()), this, SLOT(mouseClicked()));

}

playwidget::~playwidget()
{


}


void playwidget::mousePressEvent(QMouseEvent *ev)
{
    if(ev->button() == Qt::LeftButton)
    {
        mousePos = QPoint(ev->x(),ev->y());
        emit sendClickSignal(mousePos);
    }

}
