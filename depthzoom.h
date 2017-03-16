#ifndef DEPTHZOOM_H
#define DEPTHZOOM_H


#include <QWidget>

namespace Ui {class DepthZoom;}


namespace Demo {


class DepthZoom : public QWidget
{
    Q_OBJECT

public:

    DepthZoom(QWidget* p = 0);

    float far() const {return mF;}
    float near() const {return mN;}

    ~DepthZoom();


private slots:

    void on_incNear_pressed();
    void on_decNear_pressed();
    void on_incFar_pressed();
    void on_decFar_pressed();

signals:

    void valuesChanged(float near, float far);

private:

    float increment(float base) const;
    float decrement(float ) const;

private:

    Ui::DepthZoom* mUI;
    float mN;
    float mF;
};

}

#endif // DEPTHZOOM_H
