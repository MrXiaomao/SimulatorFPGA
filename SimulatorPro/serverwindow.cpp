#include "serverwindow.h"
#include "ui_serverwindow.h"
#include <QHostInfo>
#include <QNetworkInterface>
#include <QFileDialog>
#include <QAction>
#include <QSplitter>
#include <QMenu>
#include "pluginmanager.h"

ServerWindow::ServerWindow(ServerData* data, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerWindow)
    , mServerData(data)
{
    ui->setupUi(this);
    PluginManager *pluginManager = PluginManager::instance();
    ui->comboBox_pluginName->addItems(pluginManager->getPluginNames());

    //创建分隔栏
    QSplitter *splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setObjectName("splitter");
    splitter->setHandleWidth(5);
    this->layout()->addWidget(splitter);
    splitter->addWidget(ui->leftWidget);
    splitter->addWidget(ui->rightWidget);
    splitter->setSizes(QList<int>() << 100000 << 400000);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

    //获取主机名称
    QString HostName = QHostInfo::localHostName();
    //根据主机名获取主机IP地址
    QHostInfo hostinfo = QHostInfo::fromName(HostName);
    QList<QHostAddress> AddressList = hostinfo.addresses();
    if(!AddressList.isEmpty())
    {
        for(int i = 0;i<AddressList.size();i++)
        {
            QHostAddress hostaddress = AddressList.at(i);
            if(hostaddress.protocol() == QAbstractSocket::IPv4Protocol)
            {
                ui->comboBox_ip->addItem(hostaddress.toString());
            }
        }
    }

    ui->comboBox_ip->addItem("127.0.0.1");
    ui->comboBox_ip->addItem("0.0.0.0");

    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
    mTcpServer = new QTcpServer(this);
    //网络故障
    connect(mTcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(acceptError(QAbstractSocket::SocketError)));
    //客户端连接
    connect(mTcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));

    connect(ui->toolButton_add, &QToolButton::clicked, this, [=](){
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(row + 1);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(""));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(""));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(""));
        ui->tableWidget->setCurrentItem(ui->tableWidget->item(row, 0));
    });
    connect(ui->toolButton_remove, &QToolButton::clicked, this, [=](){
        QList<QTableWidgetItem*> items = ui->tableWidget->selectedItems();
        for (auto item : items){
            int row = item->row();
            ui->tableWidget->removeRow(row);
            if (row >= ui->tableWidget->rowCount())
                row--;
            if (row >=0)
                ui->tableWidget->setCurrentItem(ui->tableWidget->item(row, 0));
            break;
        }
    });
    connect(ui->toolButton_empty, &QToolButton::clicked, this, [=](){
        ui->tableWidget->clearContents();
        ui->tableWidget->setRowCount(0);
    });

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    ui->tableWidget_parameters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_parameters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    this->load();

    /*指令发生变化，实时保存*/
    connect(ui->tableWidget->model(), &QAbstractItemModel::dataChanged, this, [=](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles = QVector<int>()*/){
        ServerData *serverData = mServerData;//reinterpret_cast<ClientData*>(storedPtr);
        serverData->commandsName.clear();
        serverData->askCommands.clear();
        serverData->ackCommands.clear();
        for (int i=0; i<ui->tableWidget->rowCount(); ++i){
            QString text0 = ui->tableWidget->item(i, 0)->text();
            QString text1 = ui->tableWidget->item(i, 1)->text();
            QString text2 = ui->tableWidget->item(i, 2)->text();
            serverData->commandsName << text0.trimmed();
            serverData->askCommands << text1.trimmed();
            serverData->ackCommands << text2.trimmed();
        }
        serverData->save();
    });

    ui->textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textEdit, &QTextEdit::customContextMenuRequested, this, [=](const QPoint &pos){
        QMenu *menu = ui->textEdit->createStandardContextMenu();

        // 添加分隔线
        menu->addSeparator();

        // 添加"清空"动作
        QAction *clearAction = menu->addAction(tr("清空内容"));
        connect(clearAction, &QAction::triggered, this, [this]() {
            ui->textEdit->clear();
        });

        // 显示菜单
        menu->exec(ui->textEdit->viewport()->mapToGlobal(pos));

        // 删除菜单对象
        delete menu;
    });

    connect(ui->comboBox_pluginName, &QComboBox::currentTextChanged, this, [=](const QString &){
        //保存现在的参数列表值
        for (int i=0; i<ui->tableWidget_parameters->rowCount(); ++i){
            mTempParams[ui->tableWidget_parameters->item(i, 0)->text()] = ui->tableWidget_parameters->item(i, 1)->text();
        }

        QString pluginName = ui->comboBox_pluginName->currentText();
<<<<<<< HEAD
        auto iter = mPlugins.find(pluginName);
        if (iter == mPlugins.end()){
            PluginManager *pluginManager = PluginManager::instance();
            mCurrentPlugin = pluginManager->getPlugin(pluginName);
        }
        else{
            mCurrentPlugin = *iter;
        }

        if (mCurrentPlugin){
            QVariantMap parameter = mCurrentPlugin->invoke("readParameters").toMap();
=======
        PluginManager *pluginManager = PluginManager::instance();
        IPlugin* plugin = pluginManager->getPlugin(pluginName);
        if (plugin){
            QVariantMap parameter = plugin->invoke("readParameters").toMap();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
            ui->tableWidget_parameters->clearContents();
            ui->tableWidget_parameters->setRowCount(0);
            if (parameter.size() > 0){
                for (auto iter = parameter.begin(); iter != parameter.end(); ++iter){
                    int row = ui->tableWidget_parameters->rowCount();
                    ui->tableWidget_parameters->insertRow(row);
                    QTableWidgetItem* item0 = new QTableWidgetItem(iter.key());
                    item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);//禁止编辑
                    ui->tableWidget_parameters->setItem(row, 0, item0);
                    // QTableWidgetItem* item1 = new QTableWidgetItem(iter.value().toString());
                    // ui->tableWidget_parameters->setItem(row, 1, item1);
                    QLineEdit *lineEdit = new QLineEdit(iter.value().toString());
                    if (iter.key().contains(tr("文件"))){
                        QAction *action = lineEdit->addAction(QIcon(":/open.png"), QLineEdit::TrailingPosition);
                        QToolButton* button = qobject_cast<QToolButton*>(action->associatedWidgets().last());
                        button->setCursor(QCursor(Qt::PointingHandCursor));
                        connect(button, &QToolButton::pressed, this, [=](){
                            QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), lineEdit->text(), tr("所有文件(*.*)"));
                            if (fileName.isEmpty() || !QFileInfo::exists(fileName))
                                return;

                            lineEdit->setText(fileName);
                        });
                    }
                    ui->tableWidget_parameters->setCellWidget(row, 1, lineEdit);

                    auto constIter = mTempParams.constFind(iter.key());
                    if (constIter != mTempParams.end()){
                        //item1->setText(constIter.value().toString());
                        if (ui->tableWidget_parameters->cellWidget(row, 1)->inherits("QLineEdit"))
                            qobject_cast<QLineEdit*>(ui->tableWidget_parameters->cellWidget(row, 1))->setText(constIter.value().toString());
                    }
                }
            }
        }
    });
    emit ui->comboBox_pluginName->currentTextChanged(ui->comboBox_pluginName->currentText());

    mTimer = new QTimer(this);
    ui->lcdNumber_timer->display("00:00:00.000");
    connect(mTimer, &QTimer::timeout, this, [=](){
        ui->lcdNumber_timer->display(QDateTime::fromMSecsSinceEpoch(mElapsedTimer.elapsed(), Qt::UTC).toString("hh:mm:ss.zzz"));
    });

    connect(this, &ServerWindow::reportTransferData, this, &ServerWindow::replyTransferData);
    connect(this, &ServerWindow::reportNumberOfPackets, this, &ServerWindow::replyNumberOfPackets);
}

ServerWindow::~ServerWindow()
{
    stopTransfer(mTcpClient);
<<<<<<< HEAD

    for (auto iter = mPlugins.begin(); iter != mPlugins.end(); ++iter){
        IPlugin* plugin = qobject_cast<IPlugin*>(iter.value());
        plugin->deleteLater();
    }

=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    delete ui;
}

void ServerWindow::load()
{
    ServerData *serverData = mServerData;
    ui->comboBox_ip->setCurrentText(serverData->localIp);
    ui->spinBox_localPort->setValue(serverData->loacalPort);
    ui->comboBox_pluginName->setCurrentText(serverData->pluginName);
    ui->lineEdit_startCommand->setText(serverData->startCommand);
    ui->lineEdit_stopCommand->setText(serverData->stopCommand);
    ui->lineEdit_externTriggerCommand->setText(serverData->externTriggerCommand);

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(serverData->commandsName.size());
<<<<<<< HEAD

    QAbstractItemModel* model = ui->tableWidget->model();
    model->blockSignals(true);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    for (int i=0; i<serverData->commandsName.size(); ++i){
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(serverData->commandsName.at(i)));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(serverData->askCommands.at(i)));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(serverData->ackCommands.at(i)));
    }
<<<<<<< HEAD
    model->blockSignals(false);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010

    ui->tableWidget_parameters->clearContents();
    ui->tableWidget_parameters->setRowCount(0);
    if (serverData->params.size() > 0){
        for (auto iter = serverData->params.begin(); iter != serverData->params.end(); ++iter){
            int row = ui->tableWidget_parameters->rowCount();
            ui->tableWidget_parameters->insertRow(row);
            QTableWidgetItem* item0 = new QTableWidgetItem(iter.key());
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);//禁止编辑
            ui->tableWidget_parameters->setItem(row, 0, item0);
            // QTableWidgetItem* item1 = new QTableWidgetItem(iter.value().toString());
            // ui->tableWidget_parameters->setItem(row, 1, item1);
            QLineEdit *lineEdit = new QLineEdit(iter.value().toString());
            if (iter.key().contains(tr("文件"))){
                QAction *action = lineEdit->addAction(QIcon(":/open.png"), QLineEdit::TrailingPosition);
                QToolButton* button = qobject_cast<QToolButton*>(action->associatedWidgets().last());
                button->setCursor(QCursor(Qt::PointingHandCursor));
                connect(button, &QToolButton::pressed, this, [=](){
                    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), lineEdit->text(), tr("所有文件(*.*)"));
                    if (fileName.isEmpty() || !QFileInfo::exists(fileName))
                        return;

                    lineEdit->setText(fileName);
                });
            }
            ui->tableWidget_parameters->setCellWidget(row, 1, lineEdit);
        }
    }

    mTempParams = serverData->params;
}

void ServerWindow::on_pushButton_start_clicked()
{
<<<<<<< HEAD
    this->updateData();
=======
    ServerData *serverData = mServerData;//reinterpret_cast<ClientData*>(storedPtr);
    serverData->localIp = ui->comboBox_ip->currentText();
    serverData->loacalPort = ui->spinBox_localPort->value();
    serverData->pluginName = ui->comboBox_pluginName->currentText();
    serverData->startCommand = ui->lineEdit_startCommand->text();
    serverData->stopCommand = ui->lineEdit_stopCommand->text();
    serverData->externTriggerCommand = ui->lineEdit_externTriggerCommand->text();

    serverData->commandsName.clear();
    serverData->askCommands.clear();
    serverData->ackCommands.clear();
    for (int i=0; i<ui->tableWidget->rowCount(); ++i){
        QString text0 = ui->tableWidget->item(i, 0)->text();
        QString text1 = ui->tableWidget->item(i, 1)->text();
        QString text2 = ui->tableWidget->item(i, 2)->text();
        serverData->commandsName << text0;
        serverData->askCommands << text1;
        serverData->ackCommands << text2;
    }

    serverData->enableLoopback = ui->checkBox_loopback->isChecked();
    serverData->params.clear();
    for (int i=0; i<ui->tableWidget_parameters->rowCount(); ++i){
        serverData->params[ui->tableWidget_parameters->item(i, 0)->text()] = ui->tableWidget_parameters->item(i, 1)->text();
    }

    serverData->save();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010

    if (mTcpServer->listen(QHostAddress(ui->comboBox_ip->currentText()), ui->spinBox_localPort->value())){
        ui->pushButton_start->setEnabled(false);
        ui->pushButton_stop->setEnabled(true);
        emit reportSocketStarted();
    }
}


void ServerWindow::on_pushButton_stop_clicked()
{
    this->stopTransfer(mTcpClient);
    mTcpClient = nullptr;
    mTcpServer->close();
    ui->pushButton_start->setEnabled(true);
    ui->pushButton_stop->setEnabled(false);
    emit reportSocketClosed();
}

void ServerWindow::acceptError(QAbstractSocket::SocketError error)
{
    emit reportSocketAcceptError(error);
}


void ServerWindow::newConnection()
{
    if (mTcpServer->hasPendingConnections()){
        QTcpSocket *tcpClient = mTcpServer->nextPendingConnection();
        mTcpClient = tcpClient;
        connect(tcpClient, SIGNAL(readyRead()), this, SLOT(readyRead()));
        connect(tcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

        QString log = QString("Client %1 :%2 online.").arg(mTcpClient->peerAddress().toString()).arg(mTcpClient->peerPort());
        writeLog(log);

        emit reportSocketNewConnection(tcpClient);
    }
}

void ServerWindow::error(QAbstractSocket::SocketError)
{
    QTcpSocket *tcpClient = qobject_cast<QTcpSocket*>(sender());

    QString log = QString("Client %1 :%2 offline.").arg(mTcpClient->peerAddress().toString()).arg(mTcpClient->peerPort());
    writeLog(log);

    this->stopTransfer(tcpClient);
    mTcpClient = nullptr;
}


void ServerWindow::readyRead()
{
    QTcpSocket *tcpClient = qobject_cast<QTcpSocket*>(sender());
    //读取新的数据
    QByteArray rawData = tcpClient->readAll();
    QString command = rawData.toHex(' ');

    writeRecvLog(rawData);
    for (int i=0; i<mServerData->askCommands.size(); ++i){
        if (mServerData->askCommands[i].trimmed().toUpper() == command.toUpper()){
            QByteArray sendCommand = QByteArray::fromHex(mServerData->ackCommands[i].toLocal8Bit());
            if (!sendCommand.isEmpty()){
                tcpClient->write(sendCommand);

                writeSendLog(sendCommand);
            }
            return;
        }
    }

    if (command.toUpper() == mServerData->startCommand.trimmed().toUpper()){
        QByteArray sendCommand = QByteArray::fromHex(mServerData->startCommand.toLocal8Bit());
        if (mServerData->enableLoopback){
            tcpClient->write(sendCommand);
            writeSendLog(sendCommand);
        }

        if (!ui->externTriggerCommand->isChecked())
            this->startTransfer(tcpClient);
        return;
    }

    if (command.toUpper() == mServerData->stopCommand.trimmed().toUpper()){
        this->stopTransfer(tcpClient);

        if (mServerData->enableLoopback){
            QByteArray sendCommand = QByteArray::fromHex(mServerData->stopCommand.toLocal8Bit());
            tcpClient->write(sendCommand);
            writeSendLog(sendCommand);
        }
        return;
    }
}

void ServerWindow::writeRecvLog(QByteArray& command)
{
    QString strDateTime = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]");
    strDateTime += QString("# RECV HEX/%1").arg(command.size());

#if 1
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(QString("<span>%1</span><br>").arg(strDateTime));
    cursor.insertHtml(QString("<span style='color:#00FFFF;'>%1</span><br><br>").arg(QString(command.toHex(' ').toUpper())));
    //cursor.insertHtml("<br>");
    ui->textEdit->setTextCursor(cursor);
#else
    ui->textEdit->append(QString("%1 %2").arg(QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]"), QString(command.toHex(' ').toUpper())));
#endif
}

void ServerWindow::writeSendLog(QByteArray& command, bool isException/* = false*/)
{
    QString strDateTime = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]");
    strDateTime += QString("# SEND HEX/%1").arg(command.size());

#if 1
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(QString("<span>%1</span><br>").arg(strDateTime));
    if (isException)
        cursor.insertHtml(QString("<span style='color:#FF00FF;'>%1</span><br><br>").arg(QString(command.toHex(' ').toUpper())));
    else
        cursor.insertHtml(QString("<span style='color:#0000FF;'>%1</span><br><br>").arg(QString(command.toHex(' ').toUpper())));
    //cursor.insertHtml("<br>");
    ui->textEdit->setTextCursor(cursor);
#else
    ui->textEdit->append(QString("%1 %2").arg(QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]"), QString(command.toHex(' ').toUpper())));
#endif
}

void ServerWindow::startTransfer(QTcpSocket* /*tcpSocket*/)
{
<<<<<<< HEAD
    if (mCurrentPlugin){
        mElapsedTimer.restart();
        mTimer->start(50);

        mCurrentPlugin->invoke("writeParameters", mServerData->params);
        connect(mCurrentPlugin, &IPlugin::notifyEvent, this, [=](const QString& event, const QVariantMap& data){
=======
    PluginManager *pluginManager = PluginManager::instance();
    IPlugin* plugin = pluginManager->getPlugin(mServerData->pluginName);
    if (plugin){
        mElapsedTimer.restart();
        mTimer->start(50);

        plugin->invoke("writeParameters", mServerData->params);
        connect(plugin, &IPlugin::notifyEvent, this, [=](const QString& event, const QVariantMap& data){
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
            if (event == "waveform"){
                QByteArray waveformBytes = data["data"].toByteArray();
                QMetaObject::invokeMethod(this, "reportTransferData", Qt::DirectConnection, Q_ARG(QByteArray&, waveformBytes));
            }
            else if (event == "numberOfPackets"){
                quint64 numberOfPackets = data["data"].toULongLong();
                QMetaObject::invokeMethod(this, "reportNumberOfPackets", Qt::DirectConnection, Q_ARG(quint64&, numberOfPackets));
            }
        });
<<<<<<< HEAD
        mCurrentPlugin->initialize();
=======
        plugin->initialize();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    }
}

void ServerWindow::stopTransfer(QTcpSocket* /*tcpSocket*/)
{
    if (this->property("free").toBool())
        return;

    mTimer->stop();
<<<<<<< HEAD
    if (mCurrentPlugin){
        disconnect(mCurrentPlugin, &IPlugin::notifyEvent, this, nullptr);
        mCurrentPlugin->shutdown();
=======
    PluginManager *pluginManager = PluginManager::instance();
    IPlugin* plugin = pluginManager->getPlugin(mServerData->pluginName);
    if (plugin){
        disconnect(plugin, &IPlugin::notifyEvent, this, nullptr);
        plugin->shutdown();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    }
}

void ServerWindow::replyTransferData(QTcpSocket* tcpSocket, QByteArray& data)
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState){
        tcpSocket->write(data);
        tcpSocket->waitForBytesWritten(100);
    }
    else if (tcpSocket->state() == QAbstractSocket::UnconnectedState){
        //网络已经断开了

        //停止发送
        stopTransfer(tcpSocket);
    }
}

void ServerWindow::replyNumberOfPackets(quint64& numberOfPackets)
{
    ui->lcdNumber_numberOfPackets->display(QString::number(numberOfPackets));
}

void ServerWindow::on_toolButton_send_clicked()
{
    if (mTcpClient){
        mTcpClient->write(QByteArray::fromHex(mServerData->externTriggerCommand.toLocal8Bit()));

        this->startTransfer(mTcpClient);
    }
}

void ServerWindow::writeLog(QString& log)
{
    QString strDateTime = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]");
    strDateTime += QString("# %1").arg(log);

#if 1
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(QString("<span>%1</span><br><br>").arg(strDateTime));
    //cursor.insertHtml(QString("<span style='color:#00ff00;'>%1</span><br><br>").arg(QString(command.toHex(' ').toUpper())));
    ui->textEdit->setTextCursor(cursor);
#else
    ui->textEdit->append(QString("%1 %2").arg(QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]"), QString(command.toHex(' ').toUpper())));
#endif
}
<<<<<<< HEAD

void ServerWindow::on_pushButton_update_clicked()
{
    this->updateData();

    if (mCurrentPlugin){
        mCurrentPlugin->invoke("writeParameters", mServerData->params);
    }
}

void ServerWindow::updateData()
{
    ServerData *serverData = mServerData;//reinterpret_cast<ClientData*>(storedPtr);
    serverData->localIp = ui->comboBox_ip->currentText();
    serverData->loacalPort = ui->spinBox_localPort->value();
    serverData->pluginName = ui->comboBox_pluginName->currentText();
    serverData->startCommand = ui->lineEdit_startCommand->text();
    serverData->stopCommand = ui->lineEdit_stopCommand->text();
    serverData->externTriggerCommand = ui->lineEdit_externTriggerCommand->text();

    serverData->commandsName.clear();
    serverData->askCommands.clear();
    serverData->ackCommands.clear();
    for (int i=0; i<ui->tableWidget->rowCount(); ++i){
        QString text0 = ui->tableWidget->item(i, 0)->text();
        QString text1 = ui->tableWidget->item(i, 1)->text();
        QString text2 = ui->tableWidget->item(i, 2)->text();
        serverData->commandsName << text0;
        serverData->askCommands << text1;
        serverData->ackCommands << text2;
    }

    serverData->enableLoopback = ui->checkBox_loopback->isChecked();
    serverData->params.clear();
    for (int i=0; i<ui->tableWidget_parameters->rowCount(); ++i){
        serverData->params[ui->tableWidget_parameters->item(i, 0)->text()] = ui->tableWidget_parameters->item(i, 1)->text();
    }

    serverData->save();
}

=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
