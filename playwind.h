#ifndef PLAYWIND_H
#define PLAYWIND_H

#include <QWidget>

namespace Ui {
class playWind;
}

class playWind : public QWidget
{
    Q_OBJECT

public:
    explicit playWind(QWidget *parent = nullptr);
    ~playWind();

signals:
    void sendClickSignal(QPoint mousePos);

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    QPoint mousePos;

private:
    Ui::playWind *ui;
};

#endif // PLAYWIND_H
