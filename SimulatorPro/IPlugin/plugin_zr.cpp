#include "plugin_zr.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QElapsedTimer>

Plugin_Zr::Plugin_Zr()
    : mInitialized(false)
{
}

Plugin_Zr::~Plugin_Zr() {
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->exit(0);
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }
}

bool Plugin_Zr::initialize() {
    if (mInitialized)
        return true; // 避免重复初始化

    mTimerThread = new QLiteThread();
    mTimerThread->setWorkThreadProc([=](){
        qRegisterMetaType<QVariantMap>("QVariantMap&");
        qRegisterMetaType<QByteArray>("QByteArray&");
        quint64 base = 0;
        QElapsedTimer elapsedTimer;
        while (!mTimerThread->isInterruptionRequested())
        {
            elapsedTimer.restart();
            QByteArray waveformBytes;
            //包头0xFFFFAAB1 + 设备编号（16bit） + 数据类型（0x00D1）+ 波形数据（波形长度*16bit） + 保留位（32bit）+ 包尾0xFFFFCCD1

            //包头0xFFFFAAB1
            waveformBytes.push_back(QByteArray::fromHex("FFFFAAB1"));
            //设备编号（16bit）
            waveformBytes.push_back(QByteArray::fromHex("0001"));
            //数据类型（0x00D1）
            waveformBytes.push_back(QByteArray::fromHex("00D1"));

            //波形数据（波形长度*16bit）
            quint16 waveformLength = 512;
            quint16 period = 128;  //周期
            quint16 amplitude = 10000; // 振幅
            quint16 grain = 1;    //粒度
            for(quint64 x=base; x < base + waveformLength; x += grain)
            {
                double angle = (float) x / period * 2 * 3.1415926;
                quint16 data = (double)amplitude * sin(angle) + amplitude;//+amplitude保证数据都>=0
                waveformBytes.push_back(quint8((data >> 8) & 0xFF));
                waveformBytes.push_back(quint8(data & 0xFF));
            }

            //保留位（32bit）
            waveformBytes.push_back(QByteArray::fromHex("00000000"));
            //包尾0xFFFFCCD1
            waveformBytes.push_back(QByteArray::fromHex("FFFFCCD1"));

            QVariantMap data;
            data["timestamp"] = QDateTime::currentDateTime().toString();
            data["data"] = waveformBytes;

            base++;
            //emit notifyEvent("waveform", data);
            QString event = "waveform";
            QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));

            QThread::msleep(mSampleFrequency - elapsedTimer.elapsed());
        }        
    });
    mTimerThread->start();

    mInitialized = true;
    qDebug() << "Plugin_Zr initialized";
    return true;
}

void Plugin_Zr::shutdown(){
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }

    mInitialized = false;
    qDebug() << "Plugin_Zr shutdown";
}

QStringList Plugin_Zr::supportedMethods() const{
    return {"connect", "disconnect", "readParameters", "writeParameters"}; // 声明支持的方法
}

QVariant Plugin_Zr::invoke(const QString& method, const QVariantMap& params){
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

    return QVariant(); // 未知方法返回空
}

QVariant Plugin_Zr::connectDevice(const QVariantMap& params){
    Q_UNUSED(params); // 示例中无需参数
    QVariantMap result;
    result["success"] = true;
    result["message"] = "Device connected";
    return result;
}

QVariant Plugin_Zr::disconnectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "Device disconnected";
    return result;
}

QVariant Plugin_Zr::readParameters(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["sampleFrequency"] = mSampleFrequency;
    return result;
}

QVariant Plugin_Zr::writeParameters(const QVariantMap& params){
    mSampleFrequency = params.value("sampleFrequency").toUInt();
    qDebug() << "Writing parameters:" << params;

    QVariantMap result;
    result["success"] = true;
    result["message"] = "Parameters written successfully";
    return result;
}
