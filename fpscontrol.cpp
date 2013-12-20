#include "fpscontrol.h"
#include "ui_fpscontrol.h"


Demo::FPSControl::FPSControl(QWidget* p):
    QWidget(p),
    mUI(new Ui::FPSControl)
{
    mUI->setupUi(this);
}

Demo::FPSControl::~FPSControl() {
    delete mUI;
}

void Demo::FPSControl::on_fps_1_pressed() {
    emit valueChanged(1);
}

void Demo::FPSControl::on_fps_5_pressed() {
    emit valueChanged(5);
}

void Demo::FPSControl::on_fps_10_pressed() {
    emit valueChanged(10);
}

void Demo::FPSControl::on_fps_25_pressed() {
    emit valueChanged(25);
}

void Demo::FPSControl::on_fps_50_pressed() {
    emit valueChanged(50);
}

void Demo::FPSControl::on_fps_100_pressed() {
    emit valueChanged(100);
}
