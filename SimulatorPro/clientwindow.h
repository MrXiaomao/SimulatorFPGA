#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QWidget>
#include <QTcpSocket>
#include <QVector>
#include <QTimer>
#include <QElapsedTimer>

#include "globalsettings.h"

namespace Ui {
class ClientWindow;
}

class IPlugin;
class ClientData : public QObject
{
    Q_OBJECT
public:
    explicit ClientData(QString GUID, QString name)
        : guid(GUID)
        , sessionName(name){
        this->load();
    }
    QString guid; //唯一标识
    QString sessionName; //名称
    QString remoteIp;//远程地址
    quint32 remotePort = 8000;//远程端口
    bool bindSocket = false;//绑定本地端口
    quint32 loacalPort = 6000;//本地端口
    QString startCommand;//开始测量
    QString stopCommand;//停止测量
    QString externTriggerCommand;//外部触发指令
    QStringList commandsName;//请求包名称
    QStringList askCommands;//请求包
    QStringList ackCommands;//返回包
    bool enableLoopback = false;//是否启用回路（如果发现指令没配置，是否按原指令内容返回）
    QString pluginName;//插件名称
    QVariantMap params; //插件参数列表

    void load(){
        GlobalSettings settings("Simulator.ini");
        if (settings.childGroups().contains(guid)){
            settings.beginGroup(guid);
            sessionName = settings.value("sessionName").toString();
            remoteIp = settings.value("remoteIp").toString();
            remotePort = settings.value("remotePort").toUInt();
            bindSocket = settings.value("bindSocket").toBool();
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
        settings.setValue("remoteIp", remoteIp);
        settings.setValue("remotePort", remotePort);
        settings.setValue("bindSocket", bindSocket);
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

    void copy(ClientData* dst){
        dst->remoteIp = this->remoteIp;
        dst->remotePort = this->remotePort;
        dst->bindSocket = this->bindSocket;
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

    void paste(ClientData* dst){
        dst->remoteIp = this->remoteIp;
        dst->remotePort = this->remotePort;
        dst->bindSocket = this->bindSocket;
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
class ClientWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ClientWindow(ClientData* data, QWidget *parent = nullptr);
    ~ClientWindow();
    
    // 更新指定行、列的内容
    bool updateTableCell(int row, const QString& text);

    Q_SIGNAL void reportSocketError(QAbstractSocket::SocketError);
    Q_SIGNAL void reportSocketConnected();
    Q_SIGNAL void reportSocketClosed();
    Q_SIGNAL void reportTransferData(QByteArray&);
    Q_SIGNAL void reportNumberOfPackets(quint64&);
    Q_SIGNAL void reportFileInfo(quint32& fileSize, quint64& totalPackets);
    Q_SIGNAL void reportRecvLog(QByteArray&);
    Q_SIGNAL void reportSendLog(QByteArray&, bool isException = false);

    Q_SLOT void error(QAbstractSocket::SocketError);
    Q_SLOT void readyRead();
    Q_SLOT void connected();

    Q_SLOT void replyRecvLog(QByteArray&);
    Q_SLOT void replySendLog(QByteArray&, bool isException = false);

    Q_SLOT void startTransfer();
    Q_SLOT void stopTransfer();
    Q_SLOT void replyTransferData(QByteArray&);
    Q_SLOT void replyNumberOfPackets(quint64&);
    Q_SLOT void replyFileInfo(quint32& fileSize, quint64& totalPackets);

    Q_SLOT void load();
    Q_SLOT void updateData();
    Q_SLOT void writeLog(QString&);

private slots:
    void on_pushButton_connect_clicked();

    void on_pushButton_disconnect_clicked();

    void on_toolButton_send_clicked();

    void on_pushButton_update_clicked();

private:
    Ui::ClientWindow *ui;
    ClientData* mClientData = nullptr;
    QVariantMap mTempParams; //临时保存参数列表
    QTcpSocket* mTcpClient = nullptr;
    QTimer* mTimer = nullptr;
    QElapsedTimer mElapsedTimer;
    IPlugin* mCurrentPlugin = nullptr;
    QMap<QString, IPlugin*> mPlugins;
};

#endif // CLIENTWINDOW_H
