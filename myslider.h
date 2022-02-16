#ifndef MYSLIDER_H
#define MYSLIDER_H


#include <QSlider>

#include <QMouseEvent>

class mySlider : public QSlider {
    Q_OBJECT
public:
    explicit mySlider(QWidget *parent = 0);
    ~mySlider();

    void mousePressEvent(QMouseEvent *mouseEvent);
    void mouseReleaseEvent(QMouseEvent *mouseEvent);

signals:
    void sliderPressSianal(int iPosTime);
    void sliderMoveSianal(int iPosTime);
};
#endif // MYSLIDER_H
