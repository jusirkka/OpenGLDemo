#ifndef NEWDIALOG_H
#define NEWDIALOG_H

#include <QDialog>
#include <QModelIndex>

namespace Ui {

class NewDialog;
}

namespace Demo {

class Project;

class NewDialog : public QDialog
{
    Q_OBJECT

public:

    NewDialog(Project* p);

    const QString& name() const {return mName;}
    QModelIndex listParent();


    ~NewDialog() override;

private slots:

    void on_scriptButton_pressed();
    void on_modelButton_pressed();
    void on_imageButton_pressed();
    void on_textureButton_pressed();
    void on_shaderButton_pressed();
    void on_lineEdit_textEdited(const QString& text);

private:

    Ui::NewDialog* mUI;
    Project* mProject;
    int mParentRow;
    QString mName;
};

}
#endif // NEWDIALOG_H
