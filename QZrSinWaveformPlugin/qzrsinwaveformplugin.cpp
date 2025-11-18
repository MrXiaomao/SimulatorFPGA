#include "qzrsinwaveformplugin.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QElapsedTimer>

QZrSinWaveformPlugin::QZrSinWaveformPlugin()
    : mInitialized(false)
{
}

QZrSinWaveformPlugin::~QZrSinWaveformPlugin() {
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->exit(0);
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }
}

IPlugin* QZrSinWaveformPlugin::clone() {
    IPlugin* plugin = new QZrSinWaveformPlugin();
    return plugin;
}

#include <QtEndian>
bool QZrSinWaveformPlugin::initialize() {
    if (mInitialized)
        return true; // 避免重复初始化

    mTimerThread = new QLiteThread();
    mTimerThread->setWorkThreadProc([=](){
        qRegisterMetaType<QVariantMap>("QVariantMap&");
        qRegisterMetaType<QByteArray>("QByteArray&");
        quint64 base = 0;
        QElapsedTimer elapsedTimer;

        //波形数据（波形长度*16bit）
        quint16 waveformLength = 512;
        quint16 period = 128;  //周期
        quint16 amplitude = 10000; // 振幅
        quint16 grain = 1;    //粒度
        quint64 numberOfPackets = 1;

        while (!mTimerThread->isInterruptionRequested())
        {
            elapsedTimer.restart();
            QByteArray waveformBytes;
            //包头0xFFFFAAB1 + 数据类型（0x00D1）+ 波形数据（波形长度*16bit） + 保留位（32bit）+ 包尾0xFFFFCCD1
            waveformBytes.resize(4 + 2 + 512*2 + 4 + 4);

            // //包头0xFFFFAAB1
            // waveformBytes.at(QByteArray::fromHex("FFFFAAB1"));
            // //设备编号（16bit）
            // waveformBytes.push_back(QByteArray::fromHex("0001"));
            // //数据类型（0x00D1）
            // waveformBytes.push_back(QByteArray::fromHex("00D1"));

            // //保留位（32bit）
            // waveformBytes.push_back(QByteArray::fromHex("00000000"));
            // //包尾0xFFFFCCD1
            // waveformBytes.push_back(QByteArray::fromHex("FFFFCCD1"));

            quint16* offset = (quint16*)waveformBytes.data();
            offset[0] = 0xFFFF;//注意大小端互换
            offset[1] = 0xB1AA;
            offset[2] = 0xD100;
            offset += 3;

            for(quint64 x=base; x < base + waveformLength; x += grain)
            {
                double angle = (float) x / period * 2 * 3.1415926;
                quint16 data = (double)amplitude * sin(angle) + amplitude;//+amplitude保证数据都>=0
                offset[0] = qbswap(data);
                offset++;
                // waveformBytes.push_back(quint8((data >> 8) & 0xFF));
                // waveformBytes.push_back(quint8(data & 0xFF));
            }

            offset[0] = 0x0000;
            offset[1] = 0x0000;
            offset[2] = 0xFFFF;
            offset[3] = 0xD1CC;
            // //保留位（32bit）
            // waveformBytes.push_back(QByteArray::fromHex("00000000"));
            // //包尾0xFFFFCCD1
            // waveformBytes.push_back(QByteArray::fromHex("FFFFCCD1"));

            {
                QVariantMap data;
                data["timestamp"] = QDateTime::currentDateTime().toString();
                data["data"] = waveformBytes;

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

            base++;
            if (!mCycleTransfer)
                break;

            qint64 sleepTime = mSampleFrequency - elapsedTimer.elapsed();
            QThread::msleep(sleepTime < 0 ? 0 : sleepTime);
        }

        mTimerThread = nullptr;
    });
    mTimerThread->start();

    mInitialized = true;
    qDebug() << "QZrSinWaveformPlugin initialized";
    return true;
}

void QZrSinWaveformPlugin::shutdown(){
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }

    mInitialized = false;
    qDebug() << "QZrSinWaveformPlugin shutdown";
}

QStringList QZrSinWaveformPlugin::supportedMethods() const{
    return {"connect", "disconnect", "readParameters", "writeParameters"}; // 声明支持的方法
}

QVariant QZrSinWaveformPlugin::invoke(const QString& method, const QVariantMap& params){
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

QVariant QZrSinWaveformPlugin::connectDevice(const QVariantMap& params){
    Q_UNUSED(params); // 示例中无需参数
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device connected";
    return result;
}

QVariant QZrSinWaveformPlugin::disconnectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device disconnected";
    return result;
}

QVariant QZrSinWaveformPlugin::readParameters(const QVariantMap& params){
    Q_UNUSED(params);

    QVariantMap result;
    result["[1]是否循环发送"] = mCycleTransfer;//前面带个序号是为了禁止自动排序，因为QMap是排序的
    result["[2]发送周期/ms"] = mSampleFrequency;
    return result;
}

QVariant QZrSinWaveformPlugin::writeParameters(const QVariantMap& params){
    mCycleTransfer = params.value("[1]是否循环发送").toBool();
    mSampleFrequency = params.value("[2]发送周期/ms").toUInt();
    qDebug() << "QZrSinWaveformPlugin writing parameters:" << params;

    QVariantMap result;
    result["success"] = true;
    result["message"] = "QZrSinWaveformPlugin parameters written successfully";
    return result;
}
