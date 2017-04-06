#include "depthzoom.h"
#include "ui_depthzoom.h"


Demo::DepthZoom::DepthZoom(QWidget* p):
    QWidget(p),
    mUI(new Ui::DepthZoom),
    mN(1),
    mF(500)
{
    mUI->setupUi(this);
    mUI->farValue->setText(QString("%1").arg((double) mF, 0, 'f', 1, QChar('0')));
    mUI->nearValue->setText(QString("%1").arg((double) mN, 0, 'f', 1, QChar('0')));
}


Demo::DepthZoom::~DepthZoom() {
    delete mUI;
}

float Demo::DepthZoom::increment(float d) const {
    if (d < 0.25) return 0.01;
    if (d < 2.5) return 0.1;
    if (d < 25) return 1;
    if (d < 100) return 5;
    if (d < 500) return 20;
    return 50;
}

float Demo::DepthZoom::decrement(float d) const {
    if (d < 0.05) return 0;
    return increment(d);
}

void Demo::DepthZoom::on_decFar_pressed() {
    mF -= decrement(mF - mN);
    mUI->farValue->setText(QString("%1").arg((double) mF, 0, 'f', 2, QChar('0')));
    emit valuesChanged(mN, mF);
}

void Demo::DepthZoom::on_incFar_pressed() {
    mF += increment(mF - mN);
    mUI->farValue->setText(QString("%1").arg((double) mF, 0, 'f', 2, QChar('0')));
    emit valuesChanged(mN, mF);
}

void Demo::DepthZoom::on_decNear_pressed() {
    float z = decrement(mN);
    float f = increment(mF - mN);
    mN -= z < f ? z : f;
    mUI->nearValue->setText(QString("%1").arg((double) mN, 0, 'f', 2, QChar('0')));
    emit valuesChanged(mN, mF);
}

void Demo::DepthZoom::on_incNear_pressed() {
    float z = increment(mN);
    float f = decrement(mF - mN);
    mN += z < f ? z : f;
    mUI->nearValue->setText(QString("%1").arg((double) mN, 0, 'f', 2, QChar('0')));
    emit valuesChanged(mN, mF);
}



