#include "qgenericfile.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QFile>

QGenericFilePlugin::QGenericFilePlugin()
    : mInitialized(false)
{
}

QGenericFilePlugin::~QGenericFilePlugin() {
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->exit(0);
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }
}

IPlugin* QGenericFilePlugin::clone() {
    QGenericFilePlugin* plugin = new QGenericFilePlugin();
    return plugin;
}

bool QGenericFilePlugin::initialize() {
    if (mInitialized)
        return true; // 避免重复初始化

    mTimerThread = new QLiteThread();
    mTimerThread->setWorkThreadProc([=](){
        qRegisterMetaType<QVariantMap>("QVariantMap&");
        qRegisterMetaType<QByteArray>("QByteArray&");
        QElapsedTimer elapsedTimer;
        QFile file(mFileName);
        if (!file.open(QIODevice::ReadOnly)) return ;

        quint64 numberOfPackets = 1;
        while (!mTimerThread->isInterruptionRequested())
        {
            elapsedTimer.restart();
            QByteArray packetBytes;
            {                
                packetBytes = file.read(mBlockSize);
                if (!packetBytes.isEmpty()){
                    {
                        QVariantMap data;
                        data["timestamp"] = QDateTime::currentDateTime().toString();
                        data["data"] = packetBytes;

                        //emit notifyEvent("waveform", data);
                        QString event = "waveform";
                        QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));
                    }

                    {
                        QVariantMap data;
                        data["timestamp"] = QDateTime::currentDateTime().toString();
                        data["data"] = numberOfPackets++;

                        QString event = "numberOfPackets";
                        QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));
                    }
                }

                if (file.atEnd()){
                    if (mCycleTransfer){
                        file.seek(0);
                    }
                    else{
                        break;
                    }
                }
            }

            QThread::msleep(mSampleFrequency - elapsedTimer.elapsed());
        }
    });
    mTimerThread->start();

    mInitialized = true;
    qDebug() << "QGenericFilePlugin initialized";
    return true;
}

void QGenericFilePlugin::shutdown(){
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }

    mInitialized = false;
    qDebug() << "QGenericFilePlugin shutdown";
}

QStringList QGenericFilePlugin::supportedMethods() const{
    return {"connect", "disconnect", "readParameters", "writeParameters"}; // 声明支持的方法
}

QVariant QGenericFilePlugin::invoke(const QString& method, const QVariantMap& params){
    if (method == "connect") {
        return connectDevice(params);
    }
    else if (method == "disconnect") {
        return disconnectDevice(params);
    } else if (method == "readParameters") {
        return readParameters(params);
    } else if (method == "writeParameters") {
        return writeParameters(params);
    }

    return QVariant();
}

QVariant QGenericFilePlugin::connectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device connected";
    return result;
}

QVariant QGenericFilePlugin::disconnectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device disconnected";
    return result;
}

QVariant QGenericFilePlugin::readParameters(const QVariantMap& params){
    Q_UNUSED(params);

    QVariantMap result;
    result["[1]文件路径"] = mFileName;
    result["[2]是否循环发送"] = mCycleTransfer;
    result["[3]发送周期/ms"] = mSampleFrequency;
    result["[4]数据包大小/Byte"] = mBlockSize;
    return result;
}

QVariant QGenericFilePlugin::writeParameters(const QVariantMap& params){
    mFileName = params.value("[1]文件路径").toString();
    mCycleTransfer = params.value("[2]是否循环发送").toBool();
    mSampleFrequency = params.value("[3]发送周期/ms").toUInt();
    mBlockSize = params.value("[4]数据包大小/Byte").toUInt();
    qDebug() << "QGenericFilePlugin writing parameters:" << params;

    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin parameters written successfully";
    return result;
}
