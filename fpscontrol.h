#ifndef FPSCONTROL_H
#define FPSCONTROL_H

#include <QWidget>

namespace Ui {class FPSControl;}


namespace Demo {


class FPSControl : public QWidget
{
    Q_OBJECT

public:

    FPSControl(QWidget* p = 0);

    int getValue() const {return mVal;}
    ~FPSControl();


private slots:

    void on_fps_1_pressed();
    void on_fps_5_pressed();
    void on_fps_10_pressed();
    void on_fps_25_pressed();
    void on_fps_50_pressed();
    void on_fps_100_pressed();

signals:

    void valueChanged(int);

private:

    Ui::FPSControl* mUI;
    int mVal;
};

}

#endif // FPSCONTROL_H
