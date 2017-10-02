#ifndef SCRIPTSELECTOR_H
#define SCRIPTSELECTOR_H

#include <QWidget>

namespace Ui {class ScriptSelector;}

namespace Demo {

class Project;

class ScriptSelector: public QWidget
{
    Q_OBJECT

public:
    explicit ScriptSelector(QWidget *parent = nullptr);
    void setup(const Project* target);
    ~ScriptSelector() override;

signals:

    void initScriptChanged(const QString&);
    void drawScriptChanged(const QString&);

private slots:

    void on_initCombo_currentIndexChanged(const QString& text);
    void on_drawCombo_currentIndexChanged(const QString& text);


private:

    Ui::ScriptSelector* mUI;

};


}

#endif // SCRIPTSELECTOR_H
