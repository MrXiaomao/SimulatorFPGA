#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "createsessionwindow.h"
#include "clientwindow.h"
#include "serverwindow.h"
#include "globalsettings.h"

/////////////////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(bool isDarkTheme, QWidget *parent)
    : QGoodWindow(parent) {
    mCentralWindow = new CentralWindow(isDarkTheme, this);
    mCentralWindow->setWindowFlags(Qt::Widget);
    mGoodCentraWidget = new QGoodCentralWidget(this);

#ifdef Q_OS_MAC
    //macOS uses global menu bar
    if(QApplication::testAttribute(Qt::AA_DontUseNativeMenuBar)) {
#else
    if(true) {
#endif
        mMenuBar = mCentralWindow->menuBar();
        if (mMenuBar)
        {
            //Set font of menu bar
            QFont font = mMenuBar->font();
#ifdef Q_OS_WIN
            font.setFamily("Segoe UI");
#else
            font.setFamily(qApp->font().family());
#endif
            mMenuBar->setFont(font);

            QTimer::singleShot(0, this, [&]{
                const int title_bar_height = mGoodCentraWidget->titleBarHeight();
                mMenuBar->setStyleSheet(QString("QMenuBar {height: %0px;}").arg(title_bar_height));
            });

            connect(mGoodCentraWidget,&QGoodCentralWidget::windowActiveChanged,this, [&](bool active){
                mMenuBar->setEnabled(active);
            });

            mGoodCentraWidget->setLeftTitleBarWidget(mMenuBar);
        }
    }

    connect(qGoodStateHolder, &QGoodStateHolder::currentThemeChanged, this, [](){
        if (qGoodStateHolder->isCurrentThemeDark())
            QGoodWindow::setAppDarkTheme();
        else
            QGoodWindow::setAppLightTheme();
    });
    connect(this, &QGoodWindow::systemThemeChanged, this, [&]{
        qGoodStateHolder->setCurrentThemeDark(QGoodWindow::isSystemThemeDark());
    });
    qGoodStateHolder->setCurrentThemeDark(isDarkTheme);

    mGoodCentraWidget->setCentralWidget(mCentralWindow);

    setCentralWidget(mGoodCentraWidget);
    setWindowIcon(mCentralWindow->windowIcon());
    setWindowTitle(mCentralWindow->windowTitle());

    mGoodCentraWidget->setTitleAlignment(Qt::AlignCenter);
}

MainWindow::~MainWindow() {
    delete mCentralWindow;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    mCentralWindow->closeEvent(event);
}

bool MainWindow::event(QEvent * event) {
    if(event->type() == QEvent::StatusTip) {
        //mCentralWindow->checkStatusTipEvent(static_cast<QStatusTipEvent *>(event));
        return true;
    }

    return QGoodWindow::event(event);
}

///////////////////////////////////////////////////////////////////////////
/// \brief CentralWindow::CentralWindow
/// \param isDarkTheme
/// \param parent
CentralWindow::CentralWindow(bool isDarkTheme, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::CentralWindow)
    , mainWindow(static_cast<MainWindow *>(parent))
    , mIsDarkTheme(isDarkTheme)
{
    ui->setupUi(this);
    this->initUi();
    this->restoreSettings();

    QTimer::singleShot(0, this, [&](){
        qGoodStateHolder->setCurrentThemeDark(mIsDarkTheme);
        QGoodWindow::setAppCustomTheme(mIsDarkTheme,this->mThemeColor); // Must be >96
    });

    QTimer::singleShot(0, this, [&](){
        if(mainWindow) {
            mainWindow->fixMenuBarWidth();
        }
    });
}

CentralWindow::~CentralWindow()
{
    delete ui;
}

void CentralWindow::initUi()
{
    QActionGroup *themeActionGroup = new QActionGroup(this);
    ui->action_darkTheme->setActionGroup(themeActionGroup);
    ui->action_lightTheme->setActionGroup(themeActionGroup);
    ui->action_lightTheme->setChecked(!mIsDarkTheme);
    ui->action_darkTheme->setChecked(mIsDarkTheme);

    QShortcut *shortcutMenuBarView = new QShortcut(QKeySequence(Qt::ALT|Qt::Key_U),this);
    shortcutMenuBarView->setObjectName("shortcutMenuBarView");
    connect(shortcutMenuBarView,&QShortcut::activated,this,[&](){
        ui->action_menuBar->trigger();
    });
    shortcutMenuBarView->setEnabled(false);

    // QTreeWidgetItem *itemRoot = new QTreeWidgetItem(QStringList() << text);
    // itemRoot->setIcon(0, QIcon(":/resource/image/folder-open.png"));
    // ui->treeWidget->addTopLevelItem(itemRoot);file-folder

    connect(ui->treeWidget, &QTreeWidget::itemExpanded, this, [=](QTreeWidgetItem *item){
        TreeNodeType nodeType = (TreeNodeType)item->data(0, TreeNodeRole::roleType).toUInt();
        if (nodeType == TreeNodeType::nodeRoot || nodeType == TreeNodeType::nodeGroup)
            item->setIcon(0, QIcon(":/resource/image/folder-open.png"));
    });
    connect(ui->treeWidget, &QTreeWidget::itemCollapsed, this, [=](QTreeWidgetItem *item){
        TreeNodeType nodeType = (TreeNodeType)item->data(0, TreeNodeRole::roleType).toUInt();
        if (nodeType == TreeNodeType::nodeRoot || nodeType == TreeNodeType::nodeGroup)
            item->setIcon(0, QIcon(":/resource/image/file-folder.png"));
    });
    // ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    // connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, [=](const QPoint &pos){
    //     QTreeWidget* treeWidget = qobject_cast<QTreeWidget*>(sender());
    //     QTreeWidgetItem* item = treeWidget->itemAt(point);
    // });
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, [=](QTreeWidgetItem *item, int /*column*/){
        TreeNodeType nodeType = (TreeNodeType)item->data(0, TreeNodeRole::roleType).toUInt();
        if (nodeType == TreeNodeType::nodeRoot){
            ui->action_addGroup->setEnabled(true);
            ui->action_delGroup->setEnabled(false);

            ui->action_open->setEnabled(false);
            ui->action_newSession->setEnabled(true);
            ui->action_delSession->setEnabled(false);
        }
        else if (nodeType == TreeNodeType::nodeGroup){
            ui->action_addGroup->setEnabled(false);
            ui->action_delGroup->setEnabled(true);

            ui->action_open->setEnabled(false);
            ui->action_newSession->setEnabled(true);
            ui->action_delSession->setEnabled(false);
        }
        else if (nodeType == TreeNodeType::nodeClient || nodeType == TreeNodeType::nodeServer){
            ui->action_addGroup->setEnabled(false);
            ui->action_delGroup->setEnabled(false);

            ui->action_open->setEnabled(true);
            ui->action_newSession->setEnabled(false);
            ui->action_delSession->setEnabled(true);;

            // ui->action_copy->setEnabled(true);
            // ui->action_paste->setEnabled(true);
        }
    });

    connect(ui->treeWidget, &QTreeWidget::itemPressed, this, [=](QTreeWidgetItem *item, int /*column*/){
        if(qApp->mouseButtons() == Qt::RightButton){
            TreeNodeType nodeType = (TreeNodeType)item->data(0, TreeNodeRole::roleType).toUInt();

            QMenu *menu = new QMenu();
            if (nodeType == TreeNodeType::nodeRoot || nodeType == TreeNodeType::nodeGroup){
                menu->addAction(ui->action_addGroup);
                if (nodeType == TreeNodeType::nodeGroup){
                    menu->addAction(ui->action_delGroup);
                }

                menu->addSeparator();
                menu->addAction(ui->action_newSession);
            }
            else if (nodeType == TreeNodeType::nodeClient){
                menu->addAction(ui->action_open);
                menu->addAction(ui->action_delSession);

                menu->addSeparator();
                menu->addAction(ui->action_copy);
                menu->addAction(ui->action_paste);

                if (mCloneClientData == nullptr)
<<<<<<< HEAD
                    ui->action_paste->setEnabled(false);
                else
                    ui->action_paste->setEnabled(true);
=======
                {
                    ui->action_paste->setEnabled(false);
                }
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
            }
            else if (nodeType == TreeNodeType::nodeServer){
                menu->addAction(ui->action_open);
                menu->addAction(ui->action_delSession);

                menu->addSeparator();
                menu->addAction(ui->action_copy);
                menu->addAction(ui->action_paste);

                if (mCloneServerData == nullptr)
<<<<<<< HEAD
                    ui->action_paste->setEnabled(false);
                else
                    ui->action_paste->setEnabled(true);
=======
                {
                    ui->action_paste->setEnabled(false);
                }
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
            }

            menu->exec(QCursor::pos());
            delete menu;
        }
    });

    // 左侧栏
    QPushButton* showSessoinButton = nullptr;
    {
        showSessoinButton = new QPushButton();
        showSessoinButton->setText(tr("会话管理器"));
        showSessoinButton->setFixedSize(250,29);
        showSessoinButton->setCheckable(true);

        connect(showSessoinButton,&QPushButton::clicked,this,[=](){
            if(ui->leftStackedWidget->isHidden()) {
                ui->leftStackedWidget->show();
                GlobalSettings settings;
                settings.setValue("Global/showleftStackedWidget", "true");
            } else {
                ui->leftStackedWidget->hide();
                GlobalSettings settings;
                settings.setValue("Global/showleftStackedWidget", "false");
            }
        });

        connect(ui->toolButton_closeSessionManagerWidget,&QPushButton::clicked,this,[=](){
            ui->leftStackedWidget->hide();
            GlobalSettings settings;
            settings.setValue("Global/showleftStackedWidget", "false");
        });

        QGraphicsScene *scene = new QGraphicsScene(this);
        QGraphicsProxyWidget *w = scene->addWidget(showSessoinButton);
        w->setPos(0,0);
        w->setRotation(-90);
        ui->graphicsView->setScene(scene);
        ui->graphicsView->setFrameStyle(0);
        ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        ui->graphicsView->setFixedSize(30, 250);
        ui->leftSidewidget->setFixedWidth(30);
    }

    /*设置任务栏信息*/
    QLabel *label_Idle = new QLabel(ui->statusBar);
    label_Idle->setObjectName("label_Idle");
    label_Idle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    label_Idle->setFixedWidth(300);
    label_Idle->setText(tr("准备就绪"));
    connect(ui->statusBar,&QStatusBar::messageChanged,this,[=](const QString &message){
        label_Idle->setText(message);
    });

    /*设置任务栏时钟*/
    QLabel *label_systemtime = new QLabel(ui->statusBar);
    label_systemtime->setObjectName("label_systemtime");
    label_systemtime->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    ui->statusBar->setContentsMargins(5, 0, 5, 0);
    ui->statusBar->addWidget(label_Idle);
    ui->statusBar->addWidget(new QLabel(ui->statusBar), 1);
    // ui->statusBar->addWidget(nullptr, 1);
    ui->statusBar->addPermanentWidget(label_systemtime);

    QTimer* systemClockTimer = new QTimer(this);
    systemClockTimer->setObjectName("systemClockTimer");
    connect(systemClockTimer, &QTimer::timeout, this, [=](){
        // 获取当前时间
        QDateTime currentDateTime = QDateTime::currentDateTime();

        // 获取星期几的数字（1代表星期日，7代表星期日）
        int dayOfWeekNumber = currentDateTime.date().dayOfWeek();

        // 星期几的中文名称列表
        QStringList dayNames = {
            tr("星期日"), QObject::tr("星期一"), QObject::tr("星期二"), QObject::tr("星期三"), QObject::tr("星期四"), QObject::tr("星期五"), QObject::tr("星期六"), QObject::tr("星期日")
        };

        // 根据数字获取中文名称
        QString dayOfWeekString = dayNames.at(dayOfWeekNumber);
        this->findChild<QLabel*>("label_systemtime")->setText(QString(QObject::tr("系统时间：")) + currentDateTime.toString("yyyy/MM/dd hh:mm:ss ") + dayOfWeekString);
    });
    systemClockTimer->start(500);

    //创建分隔栏
    QSplitter *splitter = new QSplitter(Qt::Horizontal,this);
    splitter->setObjectName("splitter");
    splitter->setHandleWidth(5);
    ui->centralwidget->layout()->addWidget(splitter);
    splitter->addWidget(ui->leftStackedWidget);
    splitter->addWidget(ui->rightTabWidget);
    splitter->setSizes(QList<int>() << 100000 << 400000);
    splitter->setCollapsible(0,false);
    splitter->setCollapsible(1,false);

    // ui->treeWidget->setColumnWidth(1, 20);
    // ui->treeWidget->resizeColumnToContents(1);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);

    QTreeWidgetItem *itemRoot = new QTreeWidgetItem(QStringList() << "会话");
    itemRoot->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeRoot);
    itemRoot->setData(0, TreeNodeRole::roleGuid, "0000000000-0000-0000-0000-000000000000");
    //itemRoot->setIcon(0, QIcon(":/folder.png"));
    itemRoot->setIcon(0, QIcon(":/resource/image/file-folder.png"));
    ui->treeWidget->addTopLevelItem(itemRoot);
    loadTreeChildNode(itemRoot);

    connect(ui->rightTabWidget, &QTabWidget::tabCloseRequested, this, [=](int index){
        QWidget* w = ui->rightTabWidget->widget(index);
        ui->rightTabWidget->removeTab(index);
        mMapTabWidget.remove(w);
        w->deleteLater();
    });

    connect(ui->rightTabWidget, &QTabWidget::currentChanged, this, [=](int index){
        QWidget* w = ui->rightTabWidget->widget(index);
        QTreeWidgetItem* item = mMapTabWidget[w];
        ui->treeWidget->setCurrentItem(item);
    });

    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked, this, [=](QTreeWidgetItem *item, int /*column*/){
        for (auto iter=mMapTabWidget.begin(); iter!=mMapTabWidget.end(); ++iter){
            if (iter.value() == item){
                ui->rightTabWidget->setCurrentWidget(iter.key());
                return;
            }
        }

        openSession(item);
    });

    ui->toolButton_newSession->setDefaultAction(ui->action_newSession);
    ui->toolButton_delSession->setDefaultAction(ui->action_delSession);
    ui->toolButton_addGroup->setDefaultAction(ui->action_addGroup);
    ui->toolButton_delGroup->setDefaultAction(ui->action_delGroup);
}

void CentralWindow::loadTreeChildNode(QTreeWidgetItem* itemRoot)
{
    QString groupName = itemRoot->data(0, TreeNodeRole::roleGuid).toString();

    GlobalSettings settings("Simulator.ini");
    settings.beginGroup(groupName);
    QStringList allKeys = settings.allKeys();
    for (auto key : allKeys){
        QString value = settings.value(key).toString();
        QStringList list = value.split('|');
        if (list.size() == 2){
            QString guid = key;
            QString nodeName = list.at(0);
            if (list.at(1) == tr("分组")){
                QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << nodeName);
                itemChild->setText(0, nodeName);
                itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeGroup);
                itemChild->setData(0, TreeNodeRole::roleGuid, guid);
                itemRoot->addChild(itemChild);

                //加载子节点
                {
                    loadTreeChildNode(itemChild);
                }
            }
            else if (list.at(1) == tr("主动")){
                QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << nodeName);
                itemChild->setIcon(0, QIcon(":/resource/image/client.png"));
                itemChild->setText(0, nodeName);
                itemChild->setText(1, "主动");
                itemChild->setToolTip(1, tr("客户端(Client)"));
                itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeClient);
                itemChild->setData(0, TreeNodeRole::roleData, QVariant::fromValue(qintptr(new ClientData(guid, nodeName))));
                itemChild->setData(0, TreeNodeRole::roleGuid, guid);
                itemRoot->addChild(itemChild);
            }
            else if (list.at(1) == tr("被动")){
                QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << nodeName);
                itemChild->setIcon(0, QIcon(":/resource/image/server.png"));
                itemChild->setText(0, nodeName);
                itemChild->setText(1, "被动");
                itemChild->setToolTip(1, tr("服务器(Server)"));
                itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeServer);
                itemChild->setData(0, TreeNodeRole::roleData, QVariant::fromValue(qintptr(new ServerData(guid, nodeName))));
                itemChild->setData(0, TreeNodeRole::roleGuid, guid);
                itemRoot->addChild(itemChild);
            }
            else if (list.at(1) == tr("串口")){

            }
        }
    }
    settings.endGroup();
    ui->treeWidget->expandAll();
}
void CentralWindow::restoreSettings()
{
    GlobalSettings settings;
    if(mainWindow) {
        mainWindow->restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
        mainWindow->restoreState(settings.value("MainWindow/State").toByteArray());
    } else {
        restoreGeometry(settings.value("MainWindow/Geometry").toByteArray());
        restoreState(settings.value("MainWindow/State").toByteArray());
    }
    mThemeColor = settings.value("Global/Startup/themeColor",QColor(30,30,30)).value<QColor>();
    mThemeColorEnable = settings.value("Global/Startup/themeColorEnable",true).toBool();
    if(mThemeColorEnable) {
        QTimer::singleShot(0, this, [&](){
            qGoodStateHolder->setCurrentThemeDark(mIsDarkTheme);
            QGoodWindow::setAppCustomTheme(mIsDarkTheme,this->mThemeColor); // Must be >96
        });
    }
}

void CentralWindow::on_action_lightTheme_triggered()
{
    if(!mIsDarkTheme) return;
    mIsDarkTheme = false;
    qGoodStateHolder->setCurrentThemeDark(mIsDarkTheme);
    if(mThemeColorEnable) QGoodWindow::setAppCustomTheme(mIsDarkTheme,mThemeColor);
    GlobalSettings settings;
    settings.setValue("Global/Startup/darkTheme","false");
}


void CentralWindow::on_action_darkTheme_triggered()
{
    if(mIsDarkTheme) return;
    mIsDarkTheme = true;
    qGoodStateHolder->setCurrentThemeDark(mIsDarkTheme);
    if(mThemeColorEnable) QGoodWindow::setAppCustomTheme(mIsDarkTheme,mThemeColor);
    GlobalSettings settings;
    settings.setValue("Global/Startup/darkTheme","true");
}


void CentralWindow::on_action_colorTheme_triggered()
{
    GlobalSettings settings;
    QColor color = QColorDialog::getColor(mThemeColor, this, tr("选择颜色"));
    if (color.isValid()) {
        mThemeColor = color;
        mThemeColorEnable = true;
        qGoodStateHolder->setCurrentThemeDark(mIsDarkTheme);
        QGoodWindow::setAppCustomTheme(mIsDarkTheme,mThemeColor);
        settings.setValue("Global/Startup/themeColor",mThemeColor);
    } else {
        mThemeColorEnable = false;
        qGoodStateHolder->setCurrentThemeDark(mIsDarkTheme);
    }
    settings.setValue("Global/Startup/themeColorEnable",mThemeColorEnable);
}


void CentralWindow::closeEvent(QCloseEvent *event) {
    int reply = QMessageBox::question(this, tr("提示"), tr("确定要退出软件吗?"),
                                 QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    if (reply == QMessageBox::Yes)
        event->accept();
    else
        event->ignore();
}

bool CentralWindow::eventFilter(QObject *watched, QEvent *event)
{
    return QMainWindow::eventFilter(watched, event);
}

void CentralWindow::on_lineEdit_textChanged(const QString &arg1)
{
    for (int i=0; i<ui->treeWidget->topLevelItemCount(); ++i){
        QTreeWidgetItem *itemRoot = ui->treeWidget->topLevelItem(0);
        for (int j=0; j<itemRoot->childCount(); ++j){
            QTreeWidgetItem *itemChild = itemRoot->child(j);
            if (!itemChild->text(0).contains(arg1)){
                itemChild->setHidden(true);
            }
            else{
                itemChild->setHidden(false);
            }
        }
    }
}


void CentralWindow::openSession(QTreeWidgetItem *item)
{
    QString name = item->text(0);
    TreeNodeType nodeType = (TreeNodeType)item->data(0, TreeNodeRole::roleType).toUInt();
    if (TreeNodeType::nodeClient == nodeType){
        qintptr storedPtr = item->data(0, TreeNodeRole::roleData).value<qintptr>();
        ClientData* clientData = reinterpret_cast<ClientData*>(storedPtr);
        ClientWindow *w = new ClientWindow(clientData, ui->rightTabWidget);
        ui->rightTabWidget->addTab(w, QIcon(":/resource/image/client.png"), QString("%1-%2").arg(name,"主动"));
        ui->rightTabWidget->setCurrentWidget(w);
        ui->rightTabWidget->tabBar()->tabButton(ui->rightTabWidget->indexOf(w), QTabBar::RightSide)->setFixedSize(QSize(12,12));

        connect(w, &ClientWindow::reportSocketConnected, this, [=](){
            QPixmap pixmap(":/resource/image/client.png");
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), Qt::green);
            ui->rightTabWidget->setTabIcon(ui->rightTabWidget->indexOf(w),  QIcon(pixmap));
            item->setIcon(0, QIcon(pixmap));
        });
        connect(w, &ClientWindow::reportSocketError, this, [=](QAbstractSocket::SocketError){
            QPixmap pixmap(":/resource/image/client.png");
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), Qt::red);
            ui->rightTabWidget->setTabIcon(ui->rightTabWidget->indexOf(w),  QIcon(pixmap));
            item->setIcon(0, QIcon(pixmap));
        });
        connect(w, &ClientWindow::reportSocketClosed, this, [=](){
            QPixmap pixmap(":/resource/image/client.png");
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), Qt::red);
            ui->rightTabWidget->setTabIcon(ui->rightTabWidget->indexOf(w),  QIcon(pixmap));
            item->setIcon(0, QIcon(pixmap));
        });

        // qintptr storedPtr = item->data(0, TreeNodeRole::roleData).value<qintptr>();
        // w->setProperty("UserRoleData", QVariant::fromValue<qintptr>(storedPtr));

        mMapTabWidget[w] = item;
    }
    else if (TreeNodeType::nodeServer == nodeType){
        qintptr storedPtr = item->data(0, TreeNodeRole::roleData).value<qintptr>();
        ServerData* serverData = reinterpret_cast<ServerData*>(storedPtr);
        ServerWindow *w = new ServerWindow(serverData, ui->rightTabWidget);
        ui->rightTabWidget->addTab(w, QIcon(":/resource/image/server.png"), QString("%1-%2").arg(name,"被动"));
        ui->rightTabWidget->setCurrentWidget(w);
        ui->rightTabWidget->tabBar()->tabButton(ui->rightTabWidget->indexOf(w), QTabBar::RightSide)->setFixedSize(QSize(12,12));

        connect(w, &ServerWindow::reportSocketStarted, this, [=](){
            QPixmap pixmap(":/resource/image/server.png");
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), Qt::green);
            ui->rightTabWidget->setTabIcon(ui->rightTabWidget->indexOf(w),  QIcon(pixmap));
            item->setIcon(0, QIcon(pixmap));
        });

        connect(w, &ServerWindow::reportSocketClosed, this, [=](){
            QPixmap pixmap(":/resource/image/server.png");
            QPainter painter(&pixmap);
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(pixmap.rect(), Qt::red);
            ui->rightTabWidget->setTabIcon(ui->rightTabWidget->indexOf(w),  QIcon(pixmap));
            item->setIcon(0, QIcon(pixmap));
        });

        mMapTabWidget[w] = item;
    }
}

#include <QUuid>
#include <QMessageBox>
void CentralWindow::on_action_newSession_triggered()
{
    CreateSessionWindow dlg;
    connect(&dlg, &CreateSessionWindow::reply, this, [=](QString name, qint32 index, bool needSave, bool openInNewWindow){
        //判断名称是否重复
        QTreeWidgetItem *itemParent = ui->treeWidget->selectedItems().at(0);
        for (int j=0; j<itemParent->childCount(); ++j){
            QTreeWidgetItem *itemChild = itemParent->child(j);
            if (itemChild->text(0) == name){
                QMessageBox::information(this, tr("提示"), tr("名称重复，请重新输入。"));
                return;
            }
        }

        QUuid uuid = QUuid::createUuid();
        QString guid = uuid.toString();
        guid = guid.remove(0, 1);
        guid.chop(1);
        guid = QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss-") + guid;

        //TreeNodeType nodeType = (TreeNodeType)itemParent->data(0, TreeNodeRole::roleType).toUInt();
        if (0x00 == index){
            QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << name);
            itemChild->setIcon(0, QIcon(":/resource/image/client.png"));
            itemChild->setText(0, name);
            itemChild->setText(1, "主动");
            itemChild->setToolTip(1, "客户端(Client)");
            itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeClient);
            itemChild->setData(0, TreeNodeRole::roleData, QVariant::fromValue(qintptr(new ClientData(needSave ? guid : "", name))));
            itemChild->setData(0, TreeNodeRole::roleGuid, guid);
            itemParent->addChild(itemChild);
            ui->treeWidget->expandItem(itemParent);
            ui->treeWidget->setCurrentItem(itemChild);
            ui->treeWidget->setCurrentItem(itemChild);
            itemChild->setSelected(true);

            if (needSave){
                GlobalSettings settings("Simulator.ini");
                QString parentGuid = itemParent->data(0, TreeNodeRole::roleGuid).toString();
                settings.setValue(QString("%1/%2").arg(parentGuid, guid), QString("%1|%2").arg(name, "主动"));
            }

            if (openInNewWindow)
                openSession(itemChild);
        }
        else if (0x01 == index){
            QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << name);
            itemChild->setIcon(0, QIcon(":/resource/image/server.png"));
            itemChild->setText(0, name);
            itemChild->setText(1, "被动");
            itemChild->setToolTip(1, "服务器(Server)");
            itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeServer);
            itemChild->setData(0, TreeNodeRole::roleData, QVariant::fromValue(qintptr(new ServerData(needSave ? guid : "", name))));
            itemChild->setData(0, TreeNodeRole::roleGuid, guid);
            itemParent->addChild(itemChild);
            ui->treeWidget->expandItem(itemParent);
            ui->treeWidget->setCurrentItem(itemChild);
            itemChild->setSelected(true);

            if (needSave){
                GlobalSettings settings("Simulator.ini");
                QString parentGuid = itemParent->data(0, TreeNodeRole::roleGuid).toString();
                settings.setValue(QString("%1/%2").arg(parentGuid, guid), QString("%1|%2").arg(name, "被动"));
            }

            if (openInNewWindow)
                openSession(itemChild);
        }
    });

    dlg.exec();
}


void CentralWindow::on_action_delSession_triggered()
{
    if ((QMessageBox::question(this, tr("删除会话"), tr("您确定要删除会话吗？"))) == QMessageBox::Yes){
        QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
        if (items.size() <= 0)
            return ;

        TreeNodeType nodeType = (TreeNodeType)items.at(0)->data(0, TreeNodeRole::roleType).toUInt();
        if (TreeNodeType::nodeClient == nodeType || TreeNodeType::nodeServer == nodeType){
            QTreeWidgetItem* itemParent = items.at(0)->parent();
            for (auto iter=mMapTabWidget.begin(); iter!=mMapTabWidget.end(); ++iter){
                if (iter.value() == items.at(0)){
                    if (items.at(0)->data(0, TreeNodeRole::roleType).toUInt() == TreeNodeType::nodeClient){
                        qintptr storedPtr = items.at(0)->data(0, TreeNodeRole::roleData).value<qintptr>();
                        ClientData* clientData = reinterpret_cast<ClientData*>(storedPtr);
                        clientData->remove();
                        clientData->deleteLater();
                    }
                    else if (items.at(0)->data(0, TreeNodeRole::roleType).toUInt() == TreeNodeType::nodeServer){
                        qintptr storedPtr = items.at(0)->data(0, TreeNodeRole::roleData).value<qintptr>();
                        ServerData* serverData = reinterpret_cast<ServerData*>(storedPtr);
                        serverData->remove();
                        serverData->deleteLater();
                    }

                    ui->rightTabWidget->removeTab(ui->rightTabWidget->indexOf(iter.key()));
                    iter.key()->deleteLater();
                    iter.key()->setProperty("free", true);
                    mMapTabWidget.erase(iter);
                    break;
                }
            }

            QString guidChild = items.at(0)->data(0, TreeNodeRole::roleGuid).toString();
            QString guidParent = itemParent->data(0, TreeNodeRole::roleGuid).toString();
            GlobalSettings settings("Simulator.ini");
            settings.remove(QString("%1/%2").arg(guidParent, guidChild));
            settings.remove(guidChild);

            QTreeWidgetItem* itemChild = itemParent->takeChild(itemParent->indexOfChild(items.at(0)));
            delete itemChild;
        }
    }

}


void CentralWindow::on_action_exit_triggered()
{
    this->mainWindow->close();
}


void CentralWindow::on_action_sessionManager_triggered()
{
    if(ui->leftStackedWidget->isHidden()) {
        ui->leftStackedWidget->show();
        GlobalSettings settings;
        settings.setValue("Global/showleftStackedWidget", "true");
    }
}


void CentralWindow::on_action_client_triggered()
{
    QUuid uuid = QUuid::createUuid();
    QString guid = uuid.toString();
    QString name = guid;
    guid = guid.remove(0, 1);
    guid.chop(1);
    guid = QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss-") + guid;

    {
        QTreeWidgetItem *itemRoot = ui->treeWidget->topLevelItem(0);
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << name);
        itemChild->setIcon(0, QIcon(":/resource/image/client.png"));
        itemChild->setText(0, name);
        itemChild->setText(1, "主动");
        itemChild->setToolTip(1, "客户端(Client)");
        itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeClient);
        itemChild->setData(0, TreeNodeRole::roleData, QVariant::fromValue(qintptr(new ClientData(guid, name))));
        itemRoot->addChild(itemChild);

        // {
        //     GlobalSettings settings("Simulator.ini");
        //     settings.setValue(QString("Session/%1").arg(guid), QString("%1|%2").arg(name, "主动"));
        // }

        openSession(itemChild);
    }
}


void CentralWindow::on_action_server_triggered()
{
    QUuid uuid = QUuid::createUuid();
    QString guid = uuid.toString();
    QString name = guid;
    guid = guid.remove(0, 1);
    guid.chop(1);
    guid = QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss-") + guid;

    {
        QTreeWidgetItem *itemRoot = ui->treeWidget->topLevelItem(0);
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << name);
        itemChild->setIcon(0, QIcon(":/resource/image/server.png"));
        itemChild->setText(0, name);
        itemChild->setText(1, "被动");
        itemChild->setToolTip(1, "服务器(Server)");
        itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeServer);
        itemChild->setData(0, TreeNodeRole::roleData, QVariant::fromValue(qintptr(new ServerData(guid, name))));
        itemRoot->addChild(itemChild);

        // {
        //     GlobalSettings settings("Simulator.ini");
        //     settings.setValue(QString("Session/%1").arg(guid), QString("%1|%2").arg(name, "被动"));
        // }

        openSession(itemChild);
    }
}


void CentralWindow::on_action_menuBar_triggered(bool checked)
{
    ui->menuBar->setVisible(checked);
    if(ui->menuBar->isVisible() == false) {
        QShortcut *shortcutMenuBarView = this->findChild<QShortcut*>("shortcutMenuBarView");
        if (shortcutMenuBarView)
            shortcutMenuBarView->setEnabled(true);
    } else {
        QShortcut *shortcutMenuBarView = this->findChild<QShortcut*>("shortcutMenuBarView");
        if (shortcutMenuBarView)
            shortcutMenuBarView->setEnabled(false);
    }

    // if(mainWindow) {
    //     mainWindow->hide();
    //     QTimer::singleShot(100,this,[=](){
    //         mainWindow->show();
    //     });
    // }
}


void CentralWindow::on_action_toolBar_triggered(bool checked)
{
    ui->toolBar->setVisible(checked);
}


void CentralWindow::on_action_statusBar_triggered(bool checked)
{
    ui->statusBar->setVisible(checked);
}


void CentralWindow::on_action_newWindow_triggered()
{
    QProcess::startDetached(QApplication::applicationFilePath(),QApplication::arguments().mid(1));
}


void CentralWindow::on_action_about_triggered()
{
    QString baseName = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
    QMessageBox::about(this, tr("关于"),
                       QString("<p>") +
                           tr("版本") +
                           QString("</p><span style='color:blue;'>%1</span><p>").arg(baseName, APP_VERSION) +
                           tr("提交") +
                           QString("</p><span style='color:blue;'>%1: %2</span><p>").arg(GIT_BRANCH, GIT_HASH) +
                           tr("日期") +
                           QString("</p><span style='color:blue;'>%1</span><p>").arg(GIT_DATE) +
                           tr("开发者") +
                           QString("</p><span style='color:blue;'>SnowWolf</span><p>") +
                           "</p><p>四川大学物理学院 版权所有 (C) 2025</p>"
                       );
}


void CentralWindow::on_action_aboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}


void CentralWindow::on_action_addGroup_triggered()
{
    bool flag = false;
    QString name = QInputDialog::getText(this, tr("新建组"), tr("输入新组的名称："), QLineEdit::Normal, "", &flag);
    if (flag && !name.isEmpty())
    {
        //判断组是否存在
        for (int i=0; i<ui->treeWidget->topLevelItemCount(); ++i){
            QTreeWidgetItem *itemRoot = ui->treeWidget->topLevelItem(0);
            for (int j=0; j<itemRoot->childCount(); ++j){
                QTreeWidgetItem *itemChild = itemRoot->child(j);
                if (itemChild->text(0) == name){
                    QMessageBox::warning(this, tr("新建组"), tr("该组已经存在！"));
                    return;
                }
            }
        }

        QUuid uuid = QUuid::createUuid();
        QString guid = uuid.toString();
        guid = guid.remove(0, 1);
        guid.chop(1);
        guid = QDateTime::currentDateTime().toString("yyyy-MM-dd-HHmmss-") + guid;

        QTreeWidgetItem *itemParent = ui->treeWidget->topLevelItem(0);
        QTreeWidgetItem *itemChild = new QTreeWidgetItem(QStringList() << name);
        itemChild->setIcon(0, QIcon(":/resource/image/file-folder.png"));
        itemChild->setData(0, TreeNodeRole::roleType, TreeNodeType::nodeGroup);
        itemChild->setData(0, TreeNodeRole::roleGuid, guid);
        itemParent->addChild(itemChild);
        ui->treeWidget->setCurrentItem(itemChild);
        itemChild->setSelected(true);

        QString parentGuid = itemParent->data(0, TreeNodeRole::roleGuid).toString();
        GlobalSettings settings("Simulator.ini");
        settings.setValue(QString("%1/%2").arg(parentGuid, guid), QString("%1|%2").arg(name, "分组"));
    }
}


void CentralWindow::on_action_delGroup_triggered()
{
    if ((QMessageBox::question(this, tr("删除组"), tr("您确定要删除该组吗？"))) == QMessageBox::Yes){
        QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
        if (items.size() <= 0)
            return;

        TreeNodeType nodeType = (TreeNodeType)items.at(0)->data(0, TreeNodeRole::roleType).toUInt();
        if (TreeNodeType::nodeGroup == nodeType){
            //先删除子节点
            for (int i=0; i<items.at(0)->childCount(); ++i){
                QTreeWidgetItem *itemChild = items.at(0)->child(i);

                for (auto iter=mMapTabWidget.begin(); iter!=mMapTabWidget.end(); ++iter){
                    if (iter.value() == itemChild){
                        if (itemChild->data(0, TreeNodeRole::roleType).toUInt() == TreeNodeType::nodeClient){
                            qintptr storedPtr = itemChild->data(0, TreeNodeRole::roleData).value<qintptr>();
                            ClientData* clientData = reinterpret_cast<ClientData*>(storedPtr);
                            clientData->remove();
                            clientData->deleteLater();
                        }
                        else if (itemChild->data(0, TreeNodeRole::roleType).toUInt() == TreeNodeType::nodeServer){
                            qintptr storedPtr = itemChild->data(0, TreeNodeRole::roleData).value<qintptr>();
                            ServerData* serverData = reinterpret_cast<ServerData*>(storedPtr);
                            serverData->remove();
                            serverData->deleteLater();
                        }

                        ui->rightTabWidget->removeTab(ui->rightTabWidget->indexOf(iter.key()));
                        iter.key()->deleteLater();
                        mMapTabWidget.erase(iter);
                        break;
                    }
                }

                QString guidChild = itemChild->data(0, TreeNodeRole::roleGuid).toString();
                QString guidParent = items.at(0)->data(0, TreeNodeRole::roleGuid).toString();
                GlobalSettings settings("Simulator.ini");
                settings.remove(QString("%1/%2").arg(guidParent, guidChild));
                settings.remove(guidChild);
            }

            //再删除自己
            QTreeWidgetItem* itemParent = ui->treeWidget->topLevelItem(0);
            QTreeWidgetItem* itemChild = itemParent->takeChild(itemParent->indexOfChild(items.at(0)));

            QString guidParent = itemParent->data(0, TreeNodeRole::roleGuid).toString();
            QString guidChild = itemChild->data(0, TreeNodeRole::roleGuid).toString();
            GlobalSettings settings("Simulator.ini");
            settings.remove(QString("%1/%2").arg(guidParent, guidChild));

            delete itemChild;
        }
    }
}


void CentralWindow::on_action_copy_triggered()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if (items.size() <= 0)
        return;

    qintptr storedPtr = items.at(0)->data(0, TreeNodeRole::roleData).value<qintptr>();
    TreeNodeType nodeType = (TreeNodeType)items.at(0)->data(0, TreeNodeRole::roleType).toUInt();
    if (nodeType == TreeNodeType::nodeClient){
        ClientData* clientData = reinterpret_cast<ClientData*>(storedPtr);
        if (mCloneClientData == nullptr)
            mCloneClientData = new ClientData("", "");
        clientData->copy(mCloneClientData);
    }
    else if (nodeType == TreeNodeType::nodeServer){
        ServerData* serverData = reinterpret_cast<ServerData*>(storedPtr);
        if (mCloneServerData == nullptr)
            mCloneServerData = new ServerData("", "");
        serverData->copy(mCloneServerData);
    }
}


void CentralWindow::on_action_paste_triggered()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if (items.size() <= 0)
        return;

    qintptr storedPtr = items.at(0)->data(0, TreeNodeRole::roleData).value<qintptr>();
    TreeNodeType nodeType = (TreeNodeType)items.at(0)->data(0, TreeNodeRole::roleType).toUInt();
    if (nodeType == TreeNodeType::nodeClient){
        ClientData* clientData = reinterpret_cast<ClientData*>(storedPtr);
        mCloneClientData->paste(clientData);
        mCloneClientData->deleteLater();
        mCloneClientData = nullptr;

        for (auto iter=mMapTabWidget.begin(); iter!=mMapTabWidget.end(); ++iter){
            if (iter.value() == items.at(0)){
                static_cast<ClientWindow*>(iter.key())->load();
                return;
            }
        }
    }
    else if (nodeType == TreeNodeType::nodeServer){
        ServerData* serverData = reinterpret_cast<ServerData*>(storedPtr);
        mCloneServerData->paste(serverData);
        mCloneServerData->deleteLater();
        mCloneServerData = nullptr;

        for (auto iter=mMapTabWidget.begin(); iter!=mMapTabWidget.end(); ++iter){
            if (iter.value() == items.at(0)){
                static_cast<ServerWindow*>(iter.key())->load();
                return;
            }
        }
    }
}


void CentralWindow::on_action_open_triggered()
{
    QList<QTreeWidgetItem*> items = ui->treeWidget->selectedItems();
    if (items.size() <= 0)
        return;

    openSession(items.at(0));
}

QPixmap CentralWindow::maskPixmap(QPixmap pixmap, QColor clrMask)
{
    QPixmap result = pixmap;
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(result.rect(), clrMask);
    return result;
}

QPixmap CentralWindow::maskPixmap(QString filename, QColor clrMask)
{
    QPixmap result = QPixmap(filename);
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(result.rect(), clrMask);
    return result;
}
