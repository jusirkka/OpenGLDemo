#include "newdialog.h"
#include "ui_newdialog.h"
#include "project.h"


Demo::NewDialog::NewDialog(Project* p):
    QDialog(),
    mUI(new Ui::NewDialog),
    mProject(p),
    mName("unnamed")
{

    mUI->setupUi(this);
    mUI->lineEdit->setText(mName);
    mUI->lineEdit->selectAll();
    setResult(QDialog::Rejected);
}

Demo::NewDialog::~NewDialog() {
    delete mUI;
}

void Demo::NewDialog::on_groupButton_pressed() {
    mParentRow = mProject->groupParent().row();
    accept();
}

void Demo::NewDialog::on_modelButton_pressed() {
    mParentRow = mProject->modelParent().row();
    accept();
}

void Demo::NewDialog::on_textureButton_pressed() {
    mParentRow = mProject->textureParent().row();
    accept();
}

void Demo::NewDialog::on_lineEdit_textEdited(const QString &text) {
    mName = text;
}

QModelIndex Demo::NewDialog::listParent() {
    return mProject->index(mParentRow, QModelIndex());
}
