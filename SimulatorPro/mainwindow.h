#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QGoodWindow"
#include "QGoodCentralWidget"

QT_BEGIN_NAMESPACE
namespace Ui {
class CentralWindow;
}
QT_END_NAMESPACE

class MainWindow;
class ClientData;
class ServerData;
class CentralWindow : public QMainWindow
{
    Q_OBJECT

public:
    CentralWindow(bool isDarkTheme = true, QWidget *parent = nullptr);
    ~CentralWindow();

    enum TreeNodeType{
        nodeRoot,    //根节点
        nodeCOM,    //串口
        nodeClient,//客户端
        nodeServer,//服务器
        nodeGroup,//分组
    };

    enum TreeNodeRole{
        roleType = Qt::UserRole,
        roleData = Qt::UserRole+1,
        roleGuid = Qt::UserRole+2,
    };

public:
    virtual void closeEvent(QCloseEvent *event) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void on_action_lightTheme_triggered();

    void on_action_darkTheme_triggered();

    void on_action_colorTheme_triggered();

    void on_action_newSession_triggered();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_action_delSession_triggered();

    void on_action_exit_triggered();

    void on_action_sessionManager_triggered();

    void on_action_client_triggered();

    void on_action_server_triggered();

    void on_action_menuBar_triggered(bool checked);

    void on_action_toolBar_triggered(bool checked);

    void on_action_statusBar_triggered(bool checked);

    void on_action_newWindow_triggered();

    void on_action_about_triggered();

    void on_action_aboutQt_triggered();

    void on_action_addGroup_triggered();

    void on_action_delGroup_triggered();

    void on_action_copy_triggered();

    void on_action_paste_triggered();

    void on_action_open_triggered();

private:
    Ui::CentralWindow *ui;
    class MainWindow *mainWindow = nullptr;
    QMap<QWidget*, QTreeWidgetItem*> mMapTabWidget;

    ClientData* mCloneClientData = nullptr;
    ServerData* mCloneServerData = nullptr;

    bool mIsDarkTheme = true;
    bool mThemeColorEnable = true;
    bool mIsOneLayout = false;
    QColor mThemeColor = QColor(255,255,255);

    void initUi();
    void loadTreeChildNode(QTreeWidgetItem*);
    void restoreSettings();
    void openSession(QTreeWidgetItem *item);
    void saveSessionSettings();
    QPixmap maskPixmap(QPixmap, QColor clrMask);
    QPixmap maskPixmap(QString, QColor clrMask);
};

class MainWindow : public QGoodWindow
{
    Q_OBJECT
public:
    explicit MainWindow(bool isDarkTheme = true, QWidget *parent = nullptr);
    ~MainWindow();
    void fixMenuBarWidth(void) {
        if (mMenuBar) {
            /* FIXME: Fix the width of the menu bar
             * please optimize this code */
            int width = 0;
            int itemSpacingPx = mMenuBar->style()->pixelMetric(QStyle::PM_MenuBarItemSpacing);
            for (int i = 0; i < mMenuBar->actions().size(); i++) {
                QString text = mMenuBar->actions().at(i)->text();
                QFontMetrics fm(mMenuBar->font());
                width += fm.size(0, text).width() + itemSpacingPx*1.5;
            }
            mGoodCentraWidget->setLeftTitleBarWidth(width);
        }
    }

    CentralWindow* centralWindow() const
    {
        return this->mCentralWindow;
    }


protected:
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent * event) override;

private:
    QGoodCentralWidget *mGoodCentraWidget;
    QMenuBar *mMenuBar = nullptr;
    CentralWindow *mCentralWindow;
};

#endif // MAINWINDOW_H
