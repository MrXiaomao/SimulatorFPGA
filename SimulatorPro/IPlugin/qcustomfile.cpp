#include "qcustomfile.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QFile>

QCustomFilePlugin::QCustomFilePlugin()
    : mInitialized(false)
{
}

QCustomFilePlugin::~QCustomFilePlugin() {
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->exit(0);
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }
}

IPlugin* QCustomFilePlugin::clone() {
    QCustomFilePlugin* plugin = new QCustomFilePlugin();
    return plugin;
}

bool QCustomFilePlugin::initialize() {
    if (mInitialized)
        return true; // 避免重复初始化

    mTimerThread = new QLiteThread();
    mTimerThread->setWorkThreadProc([=](){
        qRegisterMetaType<QVariantMap>("QVariantMap&");
        qRegisterMetaType<QByteArray>("QByteArray&");
        QElapsedTimer elapsedTimer;
        QFile file(mFileName);
        if (!file.open(QIODevice::ReadOnly)) return ;

        //组包
        QByteArray packetBytes;
        {
            //包头
            packetBytes.append(QByteArray::fromHex(mPacketHead.toLocal8Bit()));

            //数据内容
            if (mFileName.endsWith(".dat")){
                packetBytes.append(file.readAll());
            }
            else{
                while (!file.atEnd()){
                    QByteArray lines = file.readLine();
                    lines = lines.replace("\r\n", "");
                    QList<QByteArray> listLine = lines.split(',');
                    for( auto line : listLine){
                        if (mDataBits==8)
                            packetBytes.push_back(static_cast<quint8>(line.at(0)));
                        else if (mDataBits==16)
                            packetBytes.push_back(static_cast<quint16>(line.toUShort()));
                        else if (mDataBits==32)
                            packetBytes.push_back(static_cast<quint32>(line.toUInt()));
                        else if (mDataBits==64)
                            packetBytes.push_back(static_cast<quint64>(line.toULongLong()));
                        else
                            packetBytes.push_back(line);
                    }
                }
                file.close();
            }

            //包尾
            packetBytes.append(QByteArray::fromHex(mPacketTail.toLocal8Bit()));
        }

        quint64 numberOfPackets = 1;
        while (!mTimerThread->isInterruptionRequested())
        {
            elapsedTimer.restart();


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

                if (!mCycleTransfer)
                    break;
            }

            QThread::msleep(mSampleFrequency - elapsedTimer.elapsed());
        }
    });
    mTimerThread->start();

    mInitialized = true;
    qDebug() << "QCustomFilePlugin initialized";
    return true;
}

void QCustomFilePlugin::shutdown(){
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->wait(500);
        mTimerThread = nullptr;
    }

    mInitialized = false;
    qDebug() << "QCustomFilePlugin shutdown";
}

QStringList QCustomFilePlugin::supportedMethods() const{
    return {"connect", "disconnect", "readParameters", "writeParameters"}; // 声明支持的方法
}

QVariant QCustomFilePlugin::invoke(const QString& method, const QVariantMap& params){
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

QVariant QCustomFilePlugin::connectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QCustomFilePlugin device connected";
    return result;
}

QVariant QCustomFilePlugin::disconnectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QCustomFilePlugin device disconnected";
    return result;
}

QVariant QCustomFilePlugin::readParameters(const QVariantMap& params){
    Q_UNUSED(params);

    QVariantMap result;
    result["[1]文件路径"] = mFileName;
    result["[2]是否循环发送"] = mCycleTransfer;
    result["[3]发送周期/ms"] = mSampleFrequency;
    result["[4]数据位数"] = mDataBits;
    return result;
}

QVariant QCustomFilePlugin::writeParameters(const QVariantMap& params){
    mFileName = params.value("[1]文件路径").toString();
    mCycleTransfer = params.value("[2]是否循环发送").toBool();
    mSampleFrequency = params.value("[3]发送周期/ms").toUInt();
    mDataBits = params.value("[4]数据位数").toUInt();
    qDebug() << "QCustomFilePlugin writing parameters:" << params;

    QVariantMap result;
    result["success"] = true;
    result["message"] = "QCustomFilePlugin parameters written successfully";
    return result;
}
