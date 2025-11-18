#ifndef CREATESESSIONWINDOW_H
#define CREATESESSIONWINDOW_H

#include <QDialog>

namespace Ui {
class CreateSessionWindow;
}

class CreateSessionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit CreateSessionWindow(QWidget *parent = nullptr);
    ~CreateSessionWindow();

    Q_SIGNAL void reply(QString, quint32, bool, bool);

private slots:
    void on_pushButton_OK_clicked();

    void on_pushButton_cancel_clicked();

private:
    Ui::CreateSessionWindow *ui;
};

#endif // CREATESESSIONWINDOW_H
