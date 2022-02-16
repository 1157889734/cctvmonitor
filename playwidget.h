#ifndef PLAYWIDGET_H
#define PLAYWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QObject>

class playwidget:public QLabel
{
    Q_OBJECT


public:
    playwidget(QLabel *parent = 0);
    ~playwidget();


signals:
//    void clicked();
    void sendClickSignal(QObject* sender);

public slots:
//    void mouseClicked();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QPoint mousePos;

};

#endif // PLAYWIDGET_H
