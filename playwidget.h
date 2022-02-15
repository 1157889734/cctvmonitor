#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>

class playwidget:public QWidget
{
    Q_OBJECT


public:
    playwidget(QWidget *parent = 0);
    ~playwidget();


signals:
//    void clicked();
    void sendClickSignal(QPoint mousePos);

public slots:
//    void mouseClicked();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QPoint mousePos;

};

#endif // PLAYWIDGET_H
