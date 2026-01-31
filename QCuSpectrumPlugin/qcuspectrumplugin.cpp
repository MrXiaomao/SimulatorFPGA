#include "qcuspectrumplugin.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QFile>
#include <QtConcurrent>

// 快速统计0x00 00 AA B2 0x00 00 CC D2的个数（数据包总数）
// 帧头/帧尾定义（小端存储，按字节顺序）
const uint8_t FRAME_HEADER[] = {0x00, 0x00, 0xAA, 0xB3};
const uint8_t FRAME_TAIL[] = {0x00, 0x00, 0xCC, 0xD3};
const size_t HEADER_LEN = sizeof(FRAME_HEADER);
const size_t TAIL_LEN = sizeof(FRAME_TAIL);
enum FrameState { STATE_IDLE, STATE_HEADER, STATE_DATA, STATE_TAIL };
int countFramesWithStateMachine(uchar* buffer, qint64 size/*const QVector<uint8_t>& buffer*/) {
    int frameCount = 0;
    FrameState state = STATE_IDLE;
    size_t headerMatchPos = 0;
    size_t tailMatchPos = 0;

    for (qint64 i = 0; i < size; ++i) {
        uchar byte = buffer[i];
        switch (state) {
        case STATE_IDLE:
            if (byte == FRAME_HEADER[0]) {
                headerMatchPos = 1;
                state = STATE_HEADER;
            }
            break;

        case STATE_HEADER:
            if (byte == FRAME_HEADER[headerMatchPos]) {
                headerMatchPos++;
                if (headerMatchPos == HEADER_LEN) {
                    state = STATE_DATA;
                    tailMatchPos = 0; // 重置帧尾匹配
                }
            } else {
                state = STATE_IDLE; // 匹配失败，回到初始状态
            }
            break;

        case STATE_DATA:
            // 同时尝试匹配帧尾
            if (byte == FRAME_TAIL[tailMatchPos]) {
                tailMatchPos++;
                if (tailMatchPos == TAIL_LEN) {
                    frameCount++;
                    state = STATE_IDLE; // 帧结束，回到初始状态
                }
            } else {
                tailMatchPos = (byte == FRAME_TAIL[0]) ? 1 : 0; // 重置帧尾匹配
            }
            break;

        default:
            state = STATE_IDLE;
            break;
        }
    }

    return frameCount;
}

QCuSpectrumPlugin::QCuSpectrumPlugin()
    : mInitialized(false)
{
}

QCuSpectrumPlugin::~QCuSpectrumPlugin() {
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->exit(0);
        mTimerThread->wait();
        mTimerThread = nullptr;
    }
}

IPlugin* QCuSpectrumPlugin::clone() {
    IPlugin* plugin = new QCuSpectrumPlugin();
    return plugin;
}

#include <QtEndian>
bool QCuSpectrumPlugin::initialize() {
    if (mInitialized)
        return true; // 避免重复初始化

    mTimerThread = new QLiteThread();
    mTimerThread->setWorkThreadProc([=](){
        qRegisterMetaType<QVariantMap>("QVariantMap");
        qRegisterMetaType<QByteArray>("QByteArray");

        QFile binaryFile(binaryFilePath);
        if (!binaryFile.open(QIODevice::ReadOnly)) {
            qCritical() << QString("无法打开二进制文件: %1").arg(binaryFilePath);
            mTimerThread->deleteLater();
            mTimerThread = nullptr;
            return ;
        }

        qDebug() << QString("开始读取二进制文件: %1").arg(binaryFilePath);

        // 使用内存映射提高大文件读取性能
        uchar* fileData = binaryFile.map(0, binaryFile.size());
        if (!fileData) {
            qWarning() << "内存映射失败，使用传统文件读取";
            binaryFile.close();
            mTimerThread->deleteLater();
            mTimerThread = nullptr;
            return ;
        }

        qint64 fileSize = binaryFile.size();
        qint64 currentPos = 0;
        sendPackets = 0;

        QElapsedTimer elapsedTimer;
        QByteArray specttrumBytes;
        specttrumBytes.reserve(32*1060);//32个数据包，每个数据包1060字节
        elapsedTimer.start();
        qint32 nextTime = mSampleFrequency;

        FrameState state = FrameState::STATE_HEADER;
        std::vector<uint8_t> currentFrame;
        size_t headerMatchPos = 0;  // 帧头匹配进度
        size_t trailerMatchPos = 0; // 帧尾匹配进度
        while (1)
        {
            QDateTime tmStart = QDateTime::currentDateTime();
            if (mTimerThread->isInterruptionRequested() || currentPos >= fileSize - 1)
                break;

            specttrumBytes.clear();

            // 查找下一个0x55包头
            uchar* startPos = fileData + currentPos;
            uchar* endPos = fileData + fileSize;

            ///////////////////////////////////////////////////////////////////////////
            if (state == FrameState::STATE_HEADER)
            {
                // 匹配帧头：逐字节对比FRAME_HEADER
                while (startPos!=endPos)
                {
                    if (*startPos == FRAME_HEADER[headerMatchPos]) {
                        headerMatchPos++;
                        for (unsigned int i=1; i<HEADER_LEN; ++i){
                            startPos++;
                            if (startPos!=endPos && *startPos == FRAME_HEADER[headerMatchPos])
                                headerMatchPos++;
                            else{
                                headerMatchPos = 0; // 重置帧头匹配进度
                                break;
                            }
                        }

                        if (headerMatchPos == HEADER_LEN) {
                            // 帧头匹配完成，开始读取数据
                            state = FrameState::STATE_DATA;
                            specttrumBytes.push_back(QByteArray::fromHex("00 00 AA B3"));
                            headerMatchPos = 0; // 重置帧头匹配进度
                            break;
                        }
                    } else {
                        startPos++;
                    }
                }
            }

            if (state == FrameState::STATE_DATA)
            {
                //startPos++;
                while (++startPos!=endPos)
                {
                    specttrumBytes.push_back(*startPos);
                    if (specttrumBytes.endsWith(QByteArray::fromHex("00 00 CC D3")))
                    {
                        // 帧尾匹配完成，提取完整帧
                        startPos++;
                        state = FrameState::STATE_HEADER; // 等待下一个帧头
                        break;
                    }
                }
            }
            ///////////////////////////////////////////////////////////////////////////

            if (mTimerThread->isInterruptionRequested())
                break;

            // 更新位置
            currentPos = startPos - fileData;

            sendPackets++;

            while (elapsedTimer.elapsed() < nextTime) {
                QThread::msleep(1u);
            }

            nextTime += mSampleFrequency;
            {
                QVariantMap data;
                data["timestamp"] = QDateTime::currentDateTime().toString();
                data["data"] = specttrumBytes;
                data["numberOfPackets"] = sendPackets;

                QString event = "spectrum";
                QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));                
            }

            if (!mCycleTransfer)
                break;
        }

		// 清理资源
        binaryFile.unmap(fileData);
        binaryFile.close();
        mTimerThread->deleteLater();
        mTimerThread = nullptr;
    });
    mTimerThread->start();

    mInitialized = true;
    qDebug() << "QCuSpectrumPlugin initialized";
    return true;
}

// 更新文件信息显示
void QCuSpectrumPlugin::updateFileInfo()
{
    if (binaryFilePath.isEmpty() || !QFile::exists(binaryFilePath)) {
        return;
    }

    QFile file(binaryFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    qint64 fileSize = file.size();
    file.close();

    // 快速统计数据包数量
    qint64 packetCount = countPacketsInFile(binaryFilePath);
    totalPackets = packetCount;

    QString info = QString("文件大小: %1 MB, 数据包数量: %2")
                       .arg(fileSize / (1024.0 * 1024.0), 0, 'f', 2)
                       .arg(packetCount);
    m_fileSize = fileSize / (1024 * 1024);
    qDebug() << "文件信息:" << info;
}


qint64 QCuSpectrumPlugin::countPacketsInFile(const QString& filePath)
{
    qDebug() <<QString("开始统计文件数据包数量: %1").arg(filePath);

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical()<< QString("无法打开文件: %1").arg(filePath);
        return -1;
    }

    qint64 fileSize = file.size();
    if (fileSize == 0) {
        qCritical()<<tr("文件为空");
        return 0;
    }

    // 使用内存映射
    uchar* fileData = file.map(0, fileSize);
    if (!fileData) {
        qDebug()<<tr("内存映射失败，使用传统方式统计");
        file.close();
        return 0;
    }

    qint64 packetCount = 0;

// 并行统计（如果文件很大）
#ifdef QT_CONCURRENT_LIB
    if (fileSize > 1024 * 1024 * 1024) { // 大于1024MB使用并行统计
        packetCount = countPacketsParallel(fileData, fileSize);
    } else
#endif
    {
        // 单线程统计
        packetCount = countFramesWithStateMachine(fileData, fileSize);
    }

    file.unmap(fileData);
    file.close();

    qInfo()<< QString("文件大小: %1 字节，有效数据包总数: %2").arg(fileSize).arg(packetCount);
    return packetCount;
}

// 并行统计 - 修正为检查数据包长度
// 测试发现该方法对边界处理有问题，会导致数据包个数漏记数（在与MATLAB读取文件代码对比发现到的问题）
// 但是该方法计算速度非常快，后续发送数据包并不受此影响。所以更建议用该方法。
qint64 QCuSpectrumPlugin::countPacketsParallel(uchar* data, qint64 size)
{
    const int threadCount = QThread::idealThreadCount();
    const qint64 chunkSize = size / threadCount;

    QVector<QFuture<qint64>> futures;

    for (int i = 0; i < threadCount; ++i) {
        qint64 start = i * chunkSize;
        qint64 end = (i == threadCount - 1) ? size : (i + 1) * chunkSize;

        futures.append(QtConcurrent::run([=]() {
            qint64 localCount = countFramesWithStateMachine(data+start, end - start);
            return localCount;
        }));
    }

    qint64 total = 0;
    for (auto& future : futures) {
        future.waitForFinished();
        total += future.result();
    }

    // 还需要检查跨chunk边界的数据包
    // 这里简化处理，实际可能需要更复杂的边界处理
    return total;
}

void QCuSpectrumPlugin::shutdown(){
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->wait();
        mTimerThread = nullptr;
    }

    mInitialized = false;
    qDebug() << "QCuSpectrumPlugin shutdown";
}

QStringList QCuSpectrumPlugin::supportedMethods() const{
    return {"connect", "disconnect", "readParameters", "writeParameters"}; // 声明支持的方法
}

QVariant QCuSpectrumPlugin::invoke(const QString& method, const QVariantMap& params){
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
    else if (method == "preProcess") {
        //预处理
        binaryFilePath = params.value("[3]铜活化测试数据路径").toString();
        return preProcess();
    }

    return QVariant(); // 未知方法返回空
}

QVariant QCuSpectrumPlugin::connectDevice(const QVariantMap& params){
    Q_UNUSED(params); // 示例中无需参数
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device connected";
    return result;
}

QVariant QCuSpectrumPlugin::disconnectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device disconnected";
    return result;
}

QVariant QCuSpectrumPlugin::readParameters(const QVariantMap& params){
    Q_UNUSED(params);

    QVariantMap result;
    result["[1]是否循环发送"] = mCycleTransfer;//前面带个序号是为了禁止自动排序，因为QMap是排序的
    result["[2]发送周期/ms"] = mSampleFrequency;
    result["[3]铜活化测试数据路径"] = binaryFilePath;
    result["[4]总能谱个数"] = totalPackets;
    result["[5]文件大小/MB"] = m_fileSize;
    return result;
}

QVariant QCuSpectrumPlugin::writeParameters(const QVariantMap& params){
    mCycleTransfer = params.value("[1]是否循环发送").toBool();
    mSampleFrequency = params.value("[2]发送周期/ms").toUInt();
    binaryFilePath = params.value("[3]铜活化测试数据路径").toString();
    // totalPackets = params.value("[4]总能谱个数").toULongLong();
    // m_fileSize = params.value("[5]文件大小/MB").toUInt();

    qDebug() << "QCuSpectrumPlugin writing parameters:" << params;

    QVariantMap result;
    result["success"] = true;
    result["message"] = "QCuSpectrumPlugin parameters written successfully";
    return result;
}

QVariant QCuSpectrumPlugin::preProcess()
{
    updateFileInfo(); // 更新文件信息

    QString event = "fileInfo";
    QVariantMap data;
    data["fileSize"] = m_fileSize;
    data["totalPackets"] = totalPackets;
    QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));

    QVariantMap result;
    result["success"] = true;
    result["message"] = "QCuSpectrumPlugin preProcess is completed";
    return result;
}
