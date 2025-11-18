#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic pop
#include "mainwindow.h"
#include "globalsettings.h"
#include "lightstyle.h"
#include "darkstyle.h"
#include "customcolorstyle.h"

#include <QApplication>
#include <QTranslator>
#include <log4qt/log4qt.h>
#include <log4qt/logger.h>
#include <log4qt/layout.h>
#include <log4qt/patternlayout.h>
#include <log4qt/consoleappender.h>
#include <log4qt/dailyfileappender.h>
#include <log4qt/logmanager.h>
#include <log4qt/propertyconfigurator.h>
#include <log4qt/loggerrepository.h>
#include <log4qt/fileappender.h>

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_DisableHighDpiScaling); // 禁用高DPI缩放支持
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); // 使用高DPI位图
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    QApplication::setApplicationName("Fpga仿真模拟器");
    QApplication::setOrganizationName("Copyright (c) 2025");
    QApplication::setOrganizationDomain("");
    QApplication::setApplicationVersion("2.0.1.0");
    QApplication::setStyle(QStyleFactory::create("fusion"));//WindowsVista fusion windows

    GlobalSettings settings;
    if(settings.value("Global/Options/enableNativeUI",false).toBool()) {
        QApplication::setAttribute(Qt::AA_DontUseNativeDialogs,false);
        QApplication::setAttribute(Qt::AA_DontUseNativeMenuBar,false);
        QApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings,false);
    }

    QString fontFamily = settings.value("Global/Options/fontFamily", "微软雅黑").toString();
    quint32 fontPointSize = settings.value("Global/Options/fontPointSize", 12).toInt();
    QFont font = qApp->font();
    font.setStyleStrategy(QFont::PreferAntialias);
    font.setHintingPreference(QFont::PreferFullHinting);
    font.setFamily(fontFamily);
    font.setPointSize(fontPointSize);
    qApp->setFont(font);
    qApp->setStyle(new DarkStyle());
    qApp->style()->setObjectName("fusion");

    settings.beginGroup("Version");
    settings.setValue("Version",GIT_VERSION);
    settings.endGroup();

    // 启用新的日子记录类
    QString filename = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
    QString sConfFilename = QString("./config/%1.log4qt.conf").arg(filename);
    if (QFileInfo::exists(sConfFilename)){
        Log4Qt::PropertyConfigurator::configure(sConfFilename);
    } else {
        Log4Qt::LogManager::setHandleQtMessages(true);
        Log4Qt::Logger *logger = Log4Qt::Logger::rootLogger();
        logger->setLevel(Log4Qt::Level::DEBUG_INT); //设置日志输出级别

        /****************PatternLayout配置日志的输出格式****************************/
        Log4Qt::PatternLayout *layout = new Log4Qt::PatternLayout();
        layout->setConversionPattern("%d{yyyy-MM-dd HH:mm:ss.zzz} [%p]: %m %n");
        layout->activateOptions();

        /***************************配置日志的输出位置***********/
        //输出到控制台
        Log4Qt::ConsoleAppender *consoleAppender = new Log4Qt::ConsoleAppender(layout, Log4Qt::ConsoleAppender::STDOUT_TARGET);
        consoleAppender->activateOptions();
        consoleAppender->setEncoding(QTextCodec::codecForName("UTF-8"));
        logger->addAppender(consoleAppender);

        //输出到文件(如果需要把离线处理单独保存日志文件，可以改这里)
        Log4Qt::DailyFileAppender *dailiAppender = new Log4Qt::DailyFileAppender(layout, "logs/.log", QString("%1_yyyy-MM-dd").arg(filename));
        dailiAppender->setAppendFile(true);
        dailiAppender->activateOptions();
        dailiAppender->setEncoding(QTextCodec::codecForName("UTF-8"));
        logger->addAppender(dailiAppender);
    }

    // 确保logs目录存在
    QDir dir(QDir::currentPath() + "/logs");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QTranslator qtTranslator;
    QTranslator qtbaseTranslator;
    //QTranslator appTranslator;
    QString qlibpath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if(qtTranslator.load("qt_zh_CN.qm",qlibpath))
        qApp->installTranslator(&qtTranslator);
    if(qtbaseTranslator.load("qtbase_zh_CN.qm",qlibpath))
        qApp->installTranslator(&qtbaseTranslator);

    QString darkTheme = "true";
    settings.beginGroup("Global/Startup");
    if(settings.contains("darkTheme"))
        darkTheme = settings.value("darkTheme").toString();
    settings.endGroup();

    bool isDarkTheme = true;
    if(darkTheme == "true") {
        isDarkTheme = true;
        QGoodWindow::setAppDarkTheme();
    } else {
        isDarkTheme = false;
        QGoodWindow::setAppLightTheme();
    }

    MainWindow w(isDarkTheme);
    QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    int x = (screenRect.width() - w.width()) / 2;
    int y = (screenRect.height() - w.height()) / 2;
    w.move(x, y);
    w.setWindowState(w.windowState() | Qt::WindowMaximized);
    w.show();

    int ret = a.exec();

    //运行运行到这里，此时主窗体析构函数还没触发，所以shutdownRootLogger需要在主窗体销毁以后再做处理
    QObject::connect(&w, &QObject::destroyed, []{
        auto logger = Log4Qt::Logger::rootLogger();
        logger->removeAllAppenders();
        logger->loggerRepository()->shutdown();
    });

    return ret;
}
