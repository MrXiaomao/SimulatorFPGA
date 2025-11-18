#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>

#include "globalsettings.h"

namespace Ui {
class ServerWindow;
}

class IPlugin;
class ServerData : public QObject
{
    Q_OBJECT
public:
    explicit ServerData(QString GUID, QString name)
        : guid(GUID)
        , sessionName(name){
        this->load();
    }
    QString guid; //唯一标识
    QString sessionName; //名称
    QString localIp;//远程地址
    quint32 loacalPort = 8000;//本地端口

    QString startCommand;//开始测量
    QString stopCommand;//停止测量
    QString externTriggerCommand;//外部触发指令
    QStringList commandsName;//请求包名称
    QStringList askCommands;//请求包
    QStringList ackCommands;//返回包
    bool enableLoopback;//是否启用回路（如果发现指令没配置，是否按原指令内容返回）

    QString pluginName;//插件名称
    QVariantMap params; //插件参数列表

    void load(){
        GlobalSettings settings("Simulator.ini");
        if (settings.childGroups().contains(guid)){
            settings.beginGroup(guid);
            sessionName = settings.value("sessionName").toString();
            localIp = settings.value("localIp").toString();
            loacalPort = settings.value("loacalPort").toUInt();
            startCommand = settings.value("startCommand").toString();
            stopCommand = settings.value("stopCommand").toString();
            externTriggerCommand = settings.value("externTriggerCommand").toString();
            commandsName = settings.value("commandsName").toStringList();
            askCommands = settings.value("askCommands").toStringList();
            ackCommands = settings.value("ackCommands").toStringList();
            enableLoopback = settings.value("enableLoopback").toBool();
            pluginName = settings.value("pluginName").toString();
            params = settings.value("params").toMap();
            settings.endGroup();
        }
    }

    void save(){
        if (this->guid.isEmpty())
            return;

        GlobalSettings settings("Simulator.ini");
        settings.beginGroup(guid);
        settings.setValue("sessionName", sessionName);
        settings.setValue("localIp", localIp);
        settings.setValue("loacalPort", loacalPort);
        settings.setValue("startCommand", startCommand);
        settings.setValue("stopCommand", stopCommand);
        settings.setValue("externTriggerCommand", externTriggerCommand);
        settings.setValue("commandsName", commandsName);
        settings.setValue("askCommands", askCommands);
        settings.setValue("ackCommands", ackCommands);
        settings.setValue("enableLoopback", enableLoopback);
        settings.setValue("pluginName", pluginName);
        settings.setValue("params", params);
        settings.endGroup();
    }

    void copy(ServerData* dst){
        dst->localIp = this->localIp;
        dst->loacalPort = this->loacalPort;        
        dst->startCommand = this->startCommand;
        dst->stopCommand = this->stopCommand;
        dst->externTriggerCommand = this->externTriggerCommand;
        dst->commandsName = this->commandsName;
        dst->askCommands = this->askCommands;
        dst->ackCommands = this->ackCommands;
        dst->enableLoopback = this->enableLoopback;
        dst->pluginName = this->pluginName;
        dst->params = this->params;
    }

    void paste(ServerData* dst){
        dst->localIp = this->localIp;
        dst->loacalPort = this->loacalPort;
        dst->startCommand = this->startCommand;
        dst->stopCommand = this->stopCommand;
        dst->externTriggerCommand = this->externTriggerCommand;
        dst->commandsName = this->commandsName;
        dst->askCommands = this->askCommands;
        dst->ackCommands = this->ackCommands;
        dst->enableLoopback = this->enableLoopback;
        dst->pluginName = this->pluginName;
        dst->params = this->params;
    }

    void remove(){
        GlobalSettings settings("Simulator.ini");
        settings.remove(guid);
    }
};

#include <qlitethread.h>
class ServerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit ServerWindow(ServerData* data, QWidget *parent = nullptr);
    ~ServerWindow();

    Q_SIGNAL void reportSocketAcceptError(QAbstractSocket::SocketError);
    Q_SIGNAL void reportSocketNewConnection(QTcpSocket *tcpClient);
    Q_SIGNAL void reportSocketStarted();
    Q_SIGNAL void reportSocketClosed();
    Q_SIGNAL void reportTransferData(QTcpSocket*,QByteArray&);
    Q_SIGNAL void reportNumberOfPackets(quint64&);

    Q_SLOT void acceptError(QAbstractSocket::SocketError);
    Q_SLOT void newConnection();

    Q_SLOT void error(QAbstractSocket::SocketError);
    Q_SLOT void readyRead();

    Q_SLOT void writeRecvLog(QByteArray&);
    Q_SLOT void writeSendLog(QByteArray&, bool isException = false);

    Q_SLOT void startTransfer(QTcpSocket*);
    Q_SLOT void stopTransfer(QTcpSocket*);
    Q_SLOT void replyTransferData(QTcpSocket*,QByteArray&);
    Q_SLOT void replyNumberOfPackets(quint64&);

    Q_SLOT void load();
    Q_SLOT void updateData();
    Q_SLOT void writeLog(QString&);

private slots:
    void on_pushButton_start_clicked();

    void on_pushButton_stop_clicked();

    void on_toolButton_send_clicked();

    void on_pushButton_update_clicked();

private:
    Ui::ServerWindow *ui;
    ServerData* mServerData = nullptr;
    QTcpServer* mTcpServer = nullptr;
    QTcpSocket* mTcpClient = nullptr;
    QVariantMap mTempParams; //临时保存参数列表
    QTimer* mTimer = nullptr;
    QElapsedTimer mElapsedTimer;
    IPlugin* mCurrentPlugin = nullptr;
    QMap<QString, IPlugin*> mPlugins;
};

#endif // SERVERWINDOW_H
