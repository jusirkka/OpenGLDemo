#include <QDebug>

#include "scriptselector.h"
#include "ui_scriptselector.h"
#include "project.h"


using namespace Demo;

ScriptSelector::ScriptSelector(QWidget *parent):
    QWidget(parent),
    mUI(new Ui::ScriptSelector)
{
    mUI->setupUi(this);
}

ScriptSelector::~ScriptSelector() {delete mUI;}

void ScriptSelector::setup(const Project* target) {

    // we do not want to handle signals while setting up
    mUI->initCombo->disconnect();
    mUI->drawCombo->disconnect();

    mUI->initCombo->clear();
    mUI->drawCombo->clear();
    int initIndex = -1;
    int drawIndex = -1;
    for (int i = 0; i < target->rowCount(target->scriptParent()); ++i) {
        QModelIndex script = target->index(i, target->scriptParent());
        QString name = target->data(script).toString();
        if (target->initScript() == name) initIndex = i;
        if (target->drawScript() == name) drawIndex = i;
        mUI->initCombo->addItem(name);
        mUI->drawCombo->addItem(name);
    }

    mUI->initCombo->setCurrentIndex(initIndex);
    mUI->drawCombo->setCurrentIndex(drawIndex);


    QMetaObject::connectSlotsByName(this);
}


void ScriptSelector::on_initCombo_currentIndexChanged(const QString& text) {
    emit initScriptChanged(text);
}

void ScriptSelector::on_drawCombo_currentIndexChanged(const QString& text) {
    emit drawScriptChanged(text);
}
