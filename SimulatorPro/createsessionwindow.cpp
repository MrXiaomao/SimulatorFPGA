#include "createsessionwindow.h"
#include "ui_createsessionwindow.h"

CreateSessionWindow::CreateSessionWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateSessionWindow)
{
    ui->setupUi(this);
}

CreateSessionWindow::~CreateSessionWindow()
{
    delete ui;
}

void CreateSessionWindow::on_pushButton_OK_clicked()
{
    if (ui->lineEdit_sessionName->text().isEmpty())
        this->reject();
    else{
        emit reply(ui->lineEdit_sessionName->text(), ui->comboBox_sessionType->currentIndex(), ui->checkBox_saveSession->isChecked(), ui->checkBox_openInNewWindow->isChecked());
        this->accept();
    }
}


void CreateSessionWindow::on_pushButton_cancel_clicked()
{
    this->reject();
}

