#include "clientwindow.h"
#include "ui_clientwindow.h"
#include <QHostInfo>
#include <QNetworkSession>
#include <QNetworkInterface>
#include <QNetworkConfigurationManager>
#include <QFileDialog>
#include <QAction>
#include <QMenu>
#include <QSplitter>
#include <QToolTip>
#include "pluginmanager.h"

ClientWindow::ClientWindow(ClientData* data, QWidget *parent)
    : QWidget{parent}
    , ui(new Ui::ClientWindow)
    , mClientData(data)
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

    ui->pushButton_connect->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);
<<<<<<< HEAD
    ui->pushButton_update->setEnabled(false);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010

    connect(this, &ClientWindow::reportRecvLog, this, &ClientWindow::replyRecvLog);
    connect(this, &ClientWindow::reportSendLog, this, &ClientWindow::replySendLog);

    mTcpClient = new QTcpSocket(this);
    //数据到达
    connect(mTcpClient, SIGNAL(readyRead()), this, SLOT(readyRead()));
    //网络故障
    connect(mTcpClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
    //客户端连接
    connect(mTcpClient, SIGNAL(connected()), this, SLOT(connected()));

    //更改系统默认超时时长，让网络连接返回能够快点
    /*QNetworkConfigurationManager manager;
    QNetworkConfiguration config = manager.defaultConfiguration();
    QList<QNetworkConfiguration> cfg_list = manager.allConfigurations();
    if (cfg_list.size() > 0)
    {
        cfg_list.first().setConnectTimeout(1000);
        config = cfg_list.first();
    }
    QSharedPointer<QNetworkSession> spNetworkSession(new QNetworkSession(config));
    mTcpClient->setProperty("_q_networksession", QVariant::fromValue(spNetworkSession));
    */
// 根据 Qt 版本选择不同的实现
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    // 旧版本 Qt 使用 QNetworkConfigurationManager
    QNetworkConfigurationManager manager;
    QNetworkConfiguration config = manager.defaultConfiguration();
    QList<QNetworkConfiguration> cfg_list = manager.allConfigurations();
    if (cfg_list.size() > 0)
    {
        cfg_list.first().setConnectTimeout(1000);
        config = cfg_list.first();
    }
    QSharedPointer<QNetworkSession> spNetworkSession(new QNetworkSession(config));
    mTcpClient->setProperty("_q_networksession", QVariant::fromValue(spNetworkSession));
#else
    // 新版本 Qt 使用其他方式设置超时
    mTcpClient->setSocketOption(QAbstractSocket::LowDelayOption, 1);
#endif

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

    //ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableWidget->setMouseTracking(true);
    // connect(ui->tableWidget, &QTableWidget::entered, this, [=](const QModelIndex &index){
    //     QToolTip::showText(QCursor::pos(), index.data().toString());
    // });
    connect(ui->tableWidget, &QTableWidget::cellEntered, this, [=](int row, int col){
        if (col == 0)
            return;

        QTableWidgetItem* item = ui->tableWidget->item(row, col);
        if(item == nullptr) {
            return;
        }

        QToolTip::showText(QCursor::pos(), item->text());
    });

    ui->tableWidget_parameters->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableWidget_parameters->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    ui->tableWidget_parameters->setMouseTracking(true);
    // connect(ui->tableWidget_parameters, &QTableWidget::entered, this, [=](const QModelIndex &index){
    //     QToolTip::showText(QCursor::pos(), index.data().toString());
    // });
    connect(ui->tableWidget_parameters, &QTableWidget::cellEntered, this, [=](int row, int col){
        if (col == 0)
            return;

        QTableWidgetItem* item = ui->tableWidget->item(row, col);
        if(item == nullptr) {
            return;
        }

        QToolTip::showText(QCursor::pos(), item->text());
    });

    this->load();

    /*指令发生变化，实时保存*/
    connect(ui->tableWidget->model(), &QAbstractItemModel::dataChanged, this, [=](const QModelIndex &/*topLeft*/, const QModelIndex &/*bottomRight*/, const QVector<int> &/*roles = QVector<int>()*/){
        ClientData *clientData = mClientData;//reinterpret_cast<ClientData*>(storedPtr);
        clientData->commandsName.clear();
        clientData->askCommands.clear();
        clientData->ackCommands.clear();
        for (int i=0; i<ui->tableWidget->rowCount(); ++i){
            QString text0 = ui->tableWidget->item(i, 0)->text();
            QString text1 = ui->tableWidget->item(i, 1)->text();
            QString text2 = ui->tableWidget->item(i, 2)->text();
            clientData->commandsName << text0.trimmed();
            clientData->askCommands << text1.trimmed();
            clientData->ackCommands << text2.trimmed();
        }
        clientData->save();
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
            if (ui->tableWidget_parameters->cellWidget(i, 1)->inherits("QLineEdit"))
                mTempParams[ui->tableWidget_parameters->item(i, 0)->text()] = qobject_cast<QLineEdit*>(ui->tableWidget_parameters->cellWidget(i, 1))->text();
            //mTempParams[ui->tableWidget_parameters->item(i, 0)->text()] = ui->tableWidget_parameters->item(i, 1)->text();
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
                    //QTableWidgetItem* item1 = new QTableWidgetItem(iter.value().toString());
                    //ui->tableWidget_parameters->setItem(row, 1, item1);
                    //这里替换成QLineEdit，方便选择文件名
                    QLineEdit *lineEdit = new QLineEdit(iter.value().toString());
                    if (iter.key().contains(tr("路径"))){
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

                    // 后面可扩展升级增加参数数据类型，如QFile、QString、double、int，QComboBox
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

    connect(this, &ClientWindow::reportTransferData, this, &ClientWindow::replyTransferData);
    connect(this, &ClientWindow::reportNumberOfPackets, this, &ClientWindow::replyNumberOfPackets);
<<<<<<< HEAD
    connect(this, &ClientWindow::reportFileInfo, this, &ClientWindow::replyFileInfo);
=======
    connect(this, &ClientWindow::reportfileInfo, this, &ClientWindow::replyfileInfo);
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
}

ClientWindow::~ClientWindow()
{
    stopTransfer();
<<<<<<< HEAD

    for (auto iter = mPlugins.begin(); iter != mPlugins.end(); ++iter){
        IPlugin* plugin = qobject_cast<IPlugin*>(iter.value());
        plugin->deleteLater();
    }

=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    delete ui;
}

void ClientWindow::load()
{
    ClientData *clientData = mClientData;
    ui->comboBox_ip->setCurrentText(clientData->remoteIp);
    ui->spinBox_remotePort->setValue(clientData->remotePort);
    ui->spinBox_localPort->setValue(clientData->loacalPort);
    ui->comboBox_pluginName->setCurrentText(clientData->pluginName);
    ui->lineEdit_startCommand->setText(clientData->startCommand);
    ui->lineEdit_stopCommand->setText(clientData->stopCommand);
    ui->lineEdit_externTriggerCommand->setText(clientData->externTriggerCommand);
    ui->checkBox_bindSocket->setChecked(mClientData->bindSocket);

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(clientData->commandsName.size());
<<<<<<< HEAD

    QAbstractItemModel* model = ui->tableWidget->model();
    model->blockSignals(true);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    for (int i=0; i<clientData->commandsName.size(); ++i){
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(clientData->commandsName.at(i)));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(clientData->askCommands.at(i)));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(clientData->ackCommands.at(i)));
    }
<<<<<<< HEAD
    model->blockSignals(false);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010

    ui->tableWidget_parameters->clearContents();
    ui->tableWidget_parameters->setRowCount(0);
    if (clientData->params.size() > 0){
        for (auto iter = clientData->params.begin(); iter != clientData->params.end(); ++iter){
            int row = ui->tableWidget_parameters->rowCount();
            ui->tableWidget_parameters->insertRow(row);
            QTableWidgetItem* item0 = new QTableWidgetItem(iter.key());
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);//禁止编辑            
            ui->tableWidget_parameters->setItem(row, 0, item0);
            //QTableWidgetItem* item1 = new QTableWidgetItem(iter.value().toString());
            //ui->tableWidget_parameters->setItem(row, 1, item1);
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

    mTempParams = clientData->params;
}

<<<<<<< HEAD
void ClientWindow::on_pushButton_connect_clicked()
{
    //qintptr storedPtr = this->property("UserRoleData").value<qintptr>();
    this->updateData();
=======
void ClientWindow::on_refreshPara_clicked()
{
    //先读取界面当前参数
    ClientData *clientData = mClientData;
    clientData->params.clear();
    for (int i=0; i<ui->tableWidget_parameters->rowCount(); ++i){
        QString key = ui->tableWidget_parameters->item(i, 0)->text();
        if (ui->tableWidget_parameters->cellWidget(i, 1)->inherits("QLineEdit")){
            QString value = qobject_cast<QLineEdit*>(ui->tableWidget_parameters->cellWidget(i, 1))->text();
            clientData->params[key] = value;
        }
    }

    //参数写入到插件中
    PluginManager *pluginManager = PluginManager::instance();
    IPlugin* plugin = pluginManager->getPlugin(mClientData->pluginName);
    if (plugin){
        plugin->invoke("writeParameters", mClientData->params);
    }
}

void ClientWindow::on_pushButton_connect_clicked()
{
    //qintptr storedPtr = this->property("UserRoleData").value<qintptr>();
    ClientData *clientData = mClientData;//reinterpret_cast<ClientData*>(storedPtr);
    clientData->remoteIp = ui->comboBox_ip->currentText();
    clientData->remotePort = ui->spinBox_remotePort->value();
    clientData->loacalPort = ui->spinBox_localPort->value();
    clientData->pluginName = ui->comboBox_pluginName->currentText();
    clientData->startCommand = ui->lineEdit_startCommand->text();
    clientData->stopCommand = ui->lineEdit_stopCommand->text();
    clientData->externTriggerCommand = ui->lineEdit_externTriggerCommand->text();
    clientData->bindSocket = ui->checkBox_bindSocket->isChecked();

    clientData->commandsName.clear();
    clientData->askCommands.clear();
    clientData->ackCommands.clear();
    for (int i=0; i<ui->tableWidget->rowCount(); ++i){
        QString text0 = ui->tableWidget->item(i, 0)->text();
        QString text1 = ui->tableWidget->item(i, 1)->text();
        QString text2 = ui->tableWidget->item(i, 2)->text();
        clientData->commandsName << text0.trimmed();
        clientData->askCommands << text1.trimmed();
        clientData->ackCommands << text2.trimmed();
    }

    clientData->enableLoopback = ui->checkBox_loopback->isChecked();
    clientData->params.clear();
    for (int i=0; i<ui->tableWidget_parameters->rowCount(); ++i){
        QString key = ui->tableWidget_parameters->item(i, 0)->text();
        if (ui->tableWidget_parameters->cellWidget(i, 1)->inherits("QLineEdit")){
            QString value = qobject_cast<QLineEdit*>(ui->tableWidget_parameters->cellWidget(i, 1))->text();
            clientData->params[key] = value;
        }
    }

    clientData->save();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010

    if (mTcpClient->state() == QAbstractSocket::ConnectedState){
        mTcpClient->disconnectFromHost();
        mTcpClient->waitForDisconnected();
    }
    if (ui->checkBox_bindSocket->isChecked())
        mTcpClient->bind(ui->spinBox_localPort->value());

    ui->lcdNumber_numberOfPackets->display("0");
    QString log = QString("Connecting to server...");
    writeLog(log);

    mTcpClient->connectToHost(ui->comboBox_ip->currentText(), ui->spinBox_remotePort->value());
    mTcpClient->waitForConnected();
    if (mTcpClient->state() == QAbstractSocket::ConnectedState){
        ui->pushButton_connect->setEnabled(false);
        ui->pushButton_disconnect->setEnabled(true);
<<<<<<< HEAD
        ui->pushButton_update->setEnabled(true);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    }
    else {
        QString log = QString("请检查服务器端口%1是否开启监听，或者路由项是否被防火墙拦截！").arg(ui->spinBox_remotePort->value());
        writeLog(log);
    }

}


void ClientWindow::on_pushButton_disconnect_clicked()
{
    stopTransfer();

    mTcpClient->disconnectFromHost();
    if (mTcpClient->state() == QAbstractSocket::ConnectedState)
        mTcpClient->waitForDisconnected();

    QString log = QString("Server disconnected");
    writeLog(log);

    ui->pushButton_connect->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);
<<<<<<< HEAD
    ui->pushButton_update->setEnabled(false);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    emit reportSocketClosed();
}

void ClientWindow::error(QAbstractSocket::SocketError error)
{
    if (error != QAbstractSocket::SocketTimeoutError){
        QString log = QString("Server disconnected");
        writeLog(log);

        ui->pushButton_connect->setEnabled(true);
        ui->pushButton_disconnect->setEnabled(false);
<<<<<<< HEAD
        ui->pushButton_update->setEnabled(false);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
        mTcpClient->close();
        this->stopTransfer();
        emit reportSocketError(error);
    }
}

void ClientWindow::connected()
{
    QString log = QString("Server connected from local %1 :%2").arg(mTcpClient->peerAddress().toString()).arg(mTcpClient->localPort());
    writeLog(log);

    ui->pushButton_connect->setEnabled(false);
    ui->pushButton_disconnect->setEnabled(true);
<<<<<<< HEAD
    ui->pushButton_update->setEnabled(true);
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    emit reportSocketConnected();
}


void ClientWindow::readyRead()
{
    //读取新的数据
    QByteArray rawData = mTcpClient->readAll();
    QString command = rawData.toHex(' ');

    emit reportRecvLog(rawData);
    for (int i=0; i<mClientData->askCommands.size(); ++i){
        if (mClientData->askCommands[i].trimmed().toUpper() == command.toUpper()){
            QByteArray sendCommand = QByteArray::fromHex(mClientData->ackCommands[i].toLocal8Bit());
            if (!sendCommand.isEmpty()){
                mTcpClient->write(sendCommand);

                emit reportSendLog(sendCommand);
            }
            return;
        }
    }

    if (command.toUpper() == mClientData->startCommand.trimmed().toUpper()){
        QByteArray sendCommand = QByteArray::fromHex(mClientData->startCommand.toLocal8Bit());
        if (mClientData->enableLoopback){
            mTcpClient->write(sendCommand);
            emit reportSendLog(sendCommand);
        }

        if (!ui->externTriggerCommand->isChecked())
            this->startTransfer();
        return;
    }

    if (command.toUpper() == mClientData->stopCommand.trimmed().toUpper()){
        this->stopTransfer();

        if (mClientData->enableLoopback){
            QByteArray sendCommand = QByteArray::fromHex(mClientData->stopCommand.toLocal8Bit());
            mTcpClient->write(sendCommand);
            emit reportSendLog(sendCommand);
        }

        return;
    }

    //解析失败的指令，按照原路返回去吧
    // if (ui->checkBox_loopback->isChecked())
    // {
    //     mTcpClient->write(rawData);
    //     emit reportSendLog(rawData, true);
    // }
}

void ClientWindow::on_toolButton_send_clicked()
{
    if (mTcpClient->isOpen()){
        mTcpClient->write(QByteArray::fromHex(mClientData->externTriggerCommand.toLocal8Bit()));

        this->startTransfer();
    }
}

void ClientWindow::replyRecvLog(QByteArray& command)
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

void ClientWindow::replySendLog(QByteArray& command, bool isException/* = false*/)
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

void ClientWindow::startTransfer()
{
<<<<<<< HEAD
    if (mCurrentPlugin){
        mElapsedTimer.restart();
        mTimer->start(50);

        mCurrentPlugin->invoke("writeParameters", mClientData->params);
        connect(mCurrentPlugin, &IPlugin::notifyEvent, this, [=](const QString& event, const QVariantMap& data){
=======
    PluginManager *pluginManager = PluginManager::instance();
    IPlugin* plugin = pluginManager->getPlugin(mClientData->pluginName);
    if (plugin){
        mElapsedTimer.restart();
        mTimer->start(500);

        plugin->invoke("writeParameters", mClientData->params);
        connect(plugin, &IPlugin::notifyEvent, this, [=](const QString& event, const QVariantMap& data){
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
            if (event == "waveform" || event == "spectrum"){
                QByteArray waveformBytes = data["data"].toByteArray();
                QMetaObject::invokeMethod(this, "reportTransferData", Qt::DirectConnection, Q_ARG(QByteArray&, waveformBytes));
            }
            else if (event == "numberOfPackets"){
                quint64 numberOfPackets = data["data"].toULongLong();
                QMetaObject::invokeMethod(this, "reportNumberOfPackets", Qt::DirectConnection, Q_ARG(quint64&, numberOfPackets));
            }
            else if (event == "fileInfo"){
                quint32 fileSize = data["fileSize"].toUInt();
                quint64 totalpacketsNum = data["totalPackets"].toULongLong();
<<<<<<< HEAD
                QMetaObject::invokeMethod(this, "reportFileInfo", Qt::DirectConnection, Q_ARG(quint32&, fileSize), Q_ARG(quint64&, totalpacketsNum));
            }
        });
        mCurrentPlugin->initialize();
=======
                replyfileInfo(fileSize,totalpacketsNum);
            }
        });
        plugin->initialize();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    }
}

void ClientWindow::stopTransfer()
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
    IPlugin* plugin = pluginManager->getPlugin(mClientData->pluginName);
    if (plugin){
        disconnect(plugin, &IPlugin::notifyEvent, this, nullptr);
        plugin->shutdown();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    }
}

void ClientWindow::replyTransferData(QByteArray& data)
{
    if (mTcpClient->state() == QAbstractSocket::ConnectedState){
        mTcpClient->write(data);
        mTcpClient->waitForBytesWritten(100);
    }
    else if (mTcpClient->state() == QAbstractSocket::UnconnectedState){
        //网络已经断开了

        //停止发送
        stopTransfer();
    }
}

void ClientWindow::replyNumberOfPackets(quint64& numberOfPackets)
{
    ui->lcdNumber_numberOfPackets->display(QString::number(numberOfPackets));
}

<<<<<<< HEAD
void ClientWindow::replyFileInfo(quint32& fileSize, quint64& totalPackets)
=======
void ClientWindow::replyfileInfo(quint32& fileSize, quint64& totalPackets)
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
{
    updateTableCell(3, QString::number(totalPackets));
    updateTableCell(4, QString::number(fileSize));
}

// 更新指定行、第2列（索引1）的内容
bool ClientWindow::updateTableCell(int row, const QString& text)
{
    const int column = 1; // 第2列索引为1

    // 检查行有效性
    if (row < 0 || row >= ui->tableWidget_parameters->rowCount()) {
        qWarning() << "行索引无效:" << row << "，表格总行数:" << ui->tableWidget_parameters->rowCount();
        return false;
    }

    // 检查表格是否有足够的列
    if (ui->tableWidget_parameters->columnCount() <= column) {
        qWarning() << "表格列数不足，当前列数:" << ui->tableWidget_parameters->columnCount();
        return false;
    }

    qDebug() << "正在更新第" << row << "行第2列，内容:" << text;

    // 获取或创建第2列的 QLineEdit
    QWidget* widget = ui->tableWidget_parameters->cellWidget(row, column);
    if (widget && widget->inherits("QLineEdit")) {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);
        lineEdit->setText(text);
        qDebug() << "成功更新现有QLineEdit";
        return true;
    }
    else {
        // 创建新的 QLineEdit
        QLineEdit* lineEdit = new QLineEdit(text);

        // 如果是文件路径列，添加文件选择按钮
        QTableWidgetItem* keyItem = ui->tableWidget_parameters->item(row, 0);
        if (keyItem && keyItem->text().contains(tr("路径"))) {
            QAction *action = lineEdit->addAction(QIcon(":/open.png"), QLineEdit::TrailingPosition);
            QToolButton* button = qobject_cast<QToolButton*>(action->associatedWidgets().last());
            button->setCursor(QCursor(Qt::PointingHandCursor));
            connect(button, &QToolButton::pressed, this, [=](){
                QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), lineEdit->text(), tr("所有文件(*.*)"));
                if (!fileName.isEmpty() && QFileInfo::exists(fileName)) {
                    lineEdit->setText(fileName);
                }
            });
        }

        ui->tableWidget_parameters->setCellWidget(row, column, lineEdit);
        qDebug() << "成功创建新的QLineEdit";
        return true;
    }
}

void ClientWindow::writeLog(QString& log)
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

void ClientWindow::on_pushButton_update_clicked()
{
    this->updateData();

    if (mCurrentPlugin){
        mCurrentPlugin->invoke("writeParameters", mClientData->params);
    }
}

void ClientWindow::updateData()
{
    ClientData *clientData = mClientData;
    clientData->remoteIp = ui->comboBox_ip->currentText();
    clientData->remotePort = ui->spinBox_remotePort->value();
    clientData->loacalPort = ui->spinBox_localPort->value();
    clientData->pluginName = ui->comboBox_pluginName->currentText();
    clientData->startCommand = ui->lineEdit_startCommand->text();
    clientData->stopCommand = ui->lineEdit_stopCommand->text();
    clientData->externTriggerCommand = ui->lineEdit_externTriggerCommand->text();
    clientData->bindSocket = ui->checkBox_bindSocket->isChecked();

    clientData->commandsName.clear();
    clientData->askCommands.clear();
    clientData->ackCommands.clear();
    for (int i=0; i<ui->tableWidget->rowCount(); ++i){
        QString text0 = ui->tableWidget->item(i, 0)->text();
        QString text1 = ui->tableWidget->item(i, 1)->text();
        QString text2 = ui->tableWidget->item(i, 2)->text();
        clientData->commandsName << text0.trimmed();
        clientData->askCommands << text1.trimmed();
        clientData->ackCommands << text2.trimmed();
    }

    clientData->enableLoopback = ui->checkBox_loopback->isChecked();
    clientData->params.clear();
    for (int i=0; i<ui->tableWidget_parameters->rowCount(); ++i){
        QString key = ui->tableWidget_parameters->item(i, 0)->text();
        if (ui->tableWidget_parameters->cellWidget(i, 1)->inherits("QLineEdit")){
            QString value = qobject_cast<QLineEdit*>(ui->tableWidget_parameters->cellWidget(i, 1))->text();
            clientData->params[key] = value;
        }
    }

    clientData->save();
}
=======
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
