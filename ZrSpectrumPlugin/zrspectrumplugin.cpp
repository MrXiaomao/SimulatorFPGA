#include "zrspectrumplugin.h"
#include <QDebug>
#include <QDateTime>
#include <QRandomGenerator>
#include <QElapsedTimer>
#include <QFile>
<<<<<<< HEAD
=======

>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
#include <QtConcurrent>

QByteArray quint32ToBytes(quint32 value, bool littleEndian = false)
{
    QByteArray ba(4, Qt::Uninitialized);

    if (littleEndian) {
        // 小端序：低位在前
        ba[0] = static_cast<char>(value & 0xFF);
        ba[1] = static_cast<char>((value >> 8) & 0xFF);
        ba[2] = static_cast<char>((value >> 16) & 0xFF);
        ba[3] = static_cast<char>((value >> 24) & 0xFF);
    } else {
        // 大端序：高位在前（网络序）
        ba[0] = static_cast<char>((value >> 24) & 0xFF);
        ba[1] = static_cast<char>((value >> 16) & 0xFF);
        ba[2] = static_cast<char>((value >> 8) & 0xFF);
        ba[3] = static_cast<char>(value & 0xFF);
    }

    return ba;
}

ZrSpectrumPlugin::ZrSpectrumPlugin()
    : mInitialized(false)
{
}

ZrSpectrumPlugin::~ZrSpectrumPlugin() {
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->exit(0);
        mTimerThread->wait();
        mTimerThread = nullptr;
    }
}

IPlugin* ZrSpectrumPlugin::clone() {
    IPlugin* plugin = new ZrSpectrumPlugin();
    return plugin;
}

#include <QtEndian>
bool ZrSpectrumPlugin::initialize() {
    if (mInitialized)
        return true; // 避免重复初始化

    mTimerThread = new QLiteThread();
    mTimerThread->setWorkThreadProc([=](){
        qRegisterMetaType<QVariantMap>("QVariantMap&");
        qRegisterMetaType<QByteArray>("QByteArray&");

        updateFileInfo(); // 更新文件信息

        QString event = "fileInfo";
        QVariantMap data;
        data["fileSize"] = m_fileSize;
        data["totalPackets"] = totalPackets;
        QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));

        QFile binaryFile(binaryFilePath);
        if (!binaryFile.open(QIODevice::ReadOnly)) {
            qCritical() << QString("无法打开二进制文件: %1").arg(binaryFilePath);
<<<<<<< HEAD
            mTimerThread->deleteLater();
            mTimerThread = nullptr;
            return ;
=======
            return;
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
        }

        qDebug() << QString("开始读取二进制文件: %1").arg(binaryFilePath);

        // 使用内存映射提高大文件读取性能
        uchar* fileData = binaryFile.map(0, binaryFile.size());
        if (!fileData) {
            qWarning() << "内存映射失败，使用传统文件读取";
            binaryFile.close();
<<<<<<< HEAD
            mTimerThread->deleteLater();
            mTimerThread = nullptr;
            return ;
=======
            return;
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
        }

        qint64 fileSize = binaryFile.size();
        qint64 currentPos = 0;
        sendPackets = 0;

        QElapsedTimer elapsedTimer;

        while (!mTimerThread->isInterruptionRequested() && currentPos < fileSize - 1)
        {
            elapsedTimer.restart();
            QByteArray specttrumBytes;
            specttrumBytes.reserve(32*1060);//32个数据包，每个数据包1060字节

            // 查找下一个0x55包头
            uchar* startPos = fileData + currentPos;
            uchar* endPos = fileData + fileSize;

            // 如果当前位置不是0x55，则查找下一个0x55
            if (*startPos != 0x55) {
                uchar* nextHeader = std::find(startPos, endPos, 0x55);
                if (nextHeader == endPos) {
                    qInfo() << tr("未找到更多数据包");
                    break;
                }
                currentPos = nextHeader - fileData;
                startPos = nextHeader;
            }

            // 查找下一个0x55作为数据包结束（下一个包的开始）
            uchar* nextHeader = std::find(startPos + 1, endPos, 0x55);
            if (nextHeader == endPos) {
                // 文件末尾，发送剩余数据
                nextHeader = endPos;
            }

            // 计算数据包长度
            qint64 packetSize = nextHeader - startPos;

            // 过滤长度异常的数据包
            if (packetSize < 8000 || packetSize > 8500) {
                qDebug() << QString("跳过长度异常的数据包: %1 字节").arg(packetSize);
                currentPos += 1; // 跳过当前0x55，继续查找下一个
                continue;
            }

            // 创建数据包
            QByteArray combinedData(reinterpret_cast<char*>(startPos), packetSize);

            //将数据包从旧有数据结构转化为新的数据结构。
            QVector<QByteArray> packets = OldData2NewData(combinedData);

            if (mTimerThread->isInterruptionRequested())
                break;

            // 填充数据
            for (const QByteArray &packet : packets) {
                specttrumBytes.append(packet);
            }

            sendPackets++;

            // 更新位置
            currentPos += packetSize;

            {
                QVariantMap data;
                data["timestamp"] = QDateTime::currentDateTime().toString();
                data["data"] = specttrumBytes;

                QString event = "spectrum";
                QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));
            }

            {
                QVariantMap data;
                data["timestamp"] = QDateTime::currentDateTime().toString();
                data["data"] = sendPackets;

                QString event = "numberOfPackets";
                QMetaObject::invokeMethod(this, "notifyEvent", Qt::QueuedConnection, Q_ARG(QString, event), Q_ARG(QVariantMap, data));
            }

            if (!mCycleTransfer)
                break;

            qint64 sleepTime = mSampleFrequency - elapsedTimer.elapsed();
            QThread::msleep(sleepTime < 0 ? 0 : sleepTime);
        }
<<<<<<< HEAD

		// 清理资源
        binaryFile.unmap(fileData);
        binaryFile.close();
        mTimerThread->deleteLater();
        mTimerThread = nullptr;
=======
        // 清理资源
        binaryFile.unmap(fileData);
        binaryFile.close();
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
    });
    mTimerThread->start();

    mInitialized = true;
    qDebug() << "ZrSpectrumPlugin initialized";
    return true;
}

//将能谱数据包从旧的数据结构转化为新的数据结构
QVector<QByteArray> ZrSpectrumPlugin::OldData2NewData(const QByteArray& data)
{
    QVector<QByteArray> packets(32);  // 存储32个独立包
    // 接收方转码
    QByteArray uncodedMsg = encode(data, m_receiverFrom1, m_receiverTo1, m_receiverFrom2, m_receiverTo2);

    // 检查报文是否完整 是否以0x55开头 是否以0x00 0x23 结尾
    if (data.isEmpty() || static_cast<quint8>(data.at(8)) != 0xD2) {
        QByteArray codeType;
        codeType.push_back(data.at(8));
        qDebug() << Q_FUNC_INFO << u8"is not a spectrum package, comman code is :" << codeType.toHex(' ').toUpper();
        return packets;
    }

    // 有效数据部分（去除包头19字节，长度为2048*4）
    QByteArray validData = uncodedMsg.mid(19, 2048*4);

    // 构造公共部分
    QByteArray commonData = QByteArray::fromHex("FFFFAAB100D2");
    commonData.append(validData.left(4)); // 能谱序号

    TCPPacket tempSpecdata;
    getDataFromQByte(validData, tempSpecdata);

    // 测量时间：1000ms
    QByteArray tmpData = QByteArray::fromHex("000003E8");
    commonData.append(tmpData);

    // 死时间，单位ns
    commonData.append(quint32ToBytes(tempSpecdata.deathTime));

    QByteArray zeroData = quint32ToBytes(0);
    QByteArray tail = QByteArray::fromHex("FFFFCCD1");

    int ch = 0; //对应2045的道址数
    int onePack = 8192/32; //每一份的道数
    for(int i = 0; i < 32; i++)
    {
        int numCh = 0; //256道道址
        QByteArray packet;  // 当前包独立缓冲
        //先放入公有部分的数据包前面部分
        packet.append(commonData);

        //能谱子编号
        packet.append(quint32ToBytes(i+1).right(2));

        //能谱部分
        if(i==0)
        {
            //原有数据前3道均为零，因此对应8192道，则是前12道为零
            for(int i = 0; i < 12; i++)
            {
                packet.append(zeroData);
                numCh++;
            }
        }

        while(numCh < onePack)
        {
            //将2048道，每一道均分到4道，形成8192道
            quint32 value = tempSpecdata.spectrum[ch];
            for(int i=0; i <4; i++)
            {
                QByteArray tmpData = quint32ToBytes(value);
                packet.append(tmpData);
                numCh++;
            }
            ch++;
        }

        //同步时钟,暂时用0代替
        packet.append(zeroData);

        //两个保留位
        packet.append(zeroData);
        packet.append(zeroData);

        //包尾
        packet.append(tail);

        // 将当前包存入 packets
        packets[i] = packet;
    }

    return packets;
}

// 转码
// #include <emmintrin.h> // _MM_HINT_T0
QByteArray ZrSpectrumPlugin::encode(const QByteArray &data, const QByteArray &from1,
            const QByteArray &to1, const QByteArray &from2, const QByteArray &to2) {

    // 数据为空或者转码规则为空 直接返回
    if (data.isEmpty() || from1.isEmpty() || from2.isEmpty()) {
        return QByteArray();
    }

    // 记录报文头
    QByteArray result;
    // 预计算转码后的数据大小，避免多次扩容
    const int specPackLen = 8237; //2050*4 + 9 +13 + 1 + 3*4+2 %完整数据包的长度（解码后长度），1是包头，9是时间信息以及空白，13是大包的帧内容
    result.reserve(specPackLen);
    result.append(data.at(0));

    // 从第二位开始进行转码
    int i = 1;
    const char* d = data.constData();
    const int from1Size = from1.size();
    const int from2Size = from2.size();
    const int dataSize = data.size();
    const char* f1 = from1.constData();
    const char* f2 = from2.constData();

    // memcmp比较可以极大地提高速度，提速33倍
    while (i < dataSize) {
        bool matched = false;
        if (i + from1Size <= dataSize && memcmp(d + i, f1, from1Size) == 0) {
            result.append(to1);
            i += from1Size;
            matched = true;
        } else if (i + from2Size <= dataSize && memcmp(d + i, f2, from2Size) == 0) {
            result.append(to2);
            i += from2Size;
            matched = true;
        }

        if (!matched) {
            result.append(d[i]);
            i++;
        }
    }

    // 返回转码后的报文
    return result;
}

// 使用reinterpret_cast直接转换
bool ZrSpectrumPlugin::getDataFromQByte(const QByteArray &byteArray, TCPPacket &DataPacket) {
    if (byteArray.size() != sizeof(TCPPacket)) {
        qWarning() <<Q_FUNC_INFO<< "数据大小不匹配";
        return false;
    }

    memcpy(&DataPacket, byteArray.constData(), sizeof(DataPacket));

    // 字节序问题：由于x86 是小端序，网络数据QByteArray通常是大端序，这里必须转化一次才正常
    DataPacket.convertNetworkToHost();
    return true;
}

// 更新文件信息显示
void ZrSpectrumPlugin::updateFileInfo()
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

// 快速统计0x55的个数（数据包总数）
qint64 ZrSpectrumPlugin::countPacketsInFile(const QString& filePath)
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
        return countPacketsInFileTraditional(filePath);
    }

    qint64 packetCount = 0;
    const uchar targetByte = 0x55;

// 并行统计（如果文件很大）
#ifdef QT_CONCURRENT_LIB
    if (fileSize > 1024 * 1024 * 1024) { // 大于1024MB使用并行统计
        packetCount = countPacketsParallel(fileData, fileSize, targetByte);
    } else
#endif
    {
        // 单线程统计
        qint64 lastPos = 0;
        for (qint64 i = 0; i < fileSize; ++i) {
            if (fileData[i] == targetByte) {
                // 计算两个0x55之间的距离（数据包长度）
                qint64 packetLength = i - lastPos;
                //筛选合适的数据包
                if(packetLength > 8000 && packetLength < 8500) //未转码的时候标准包长度为8237
                {
                    packetCount++;
                }
                lastPos = i;
            }
        }

        // 检查最后一个数据包（如果文件以0x55结尾）
        if (lastPos >0 && lastPos < fileSize - 1) {
            qint64 lastPacketLength = fileSize - lastPos;
            if (lastPacketLength >= 8000 && lastPacketLength <= 8500) {
                packetCount++;
            }
        }
    }

    file.unmap(fileData);
    file.close();

    qInfo()<< QString("文件大小: %1 字节，有效数据包总数: %2").arg(fileSize).arg(packetCount);
    return packetCount;
}

// 传统方式统计0x55个数
qint64 ZrSpectrumPlugin::countPacketsInFileTraditional(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }

    qint64 packetCount = 0;
    const qint64 BUFFER_SIZE = 1024 * 1024; // 1MB缓冲区
    QByteArray buffer;
    char targetChar = 0x55;

    while (!file.atEnd()) {
        QByteArray chunk = file.read(BUFFER_SIZE);
        packetCount += chunk.count(targetChar);
    }

    file.close();
    return packetCount;
}


// 并行统计 - 修正为检查数据包长度
// 测试发现该方法对边界处理有问题，会导致数据包个数漏记数（在与MATLAB读取文件代码对比发现到的问题）
// 但是该方法计算速度非常快，后续发送数据包并不受此影响。所以更建议用该方法。
qint64 ZrSpectrumPlugin::countPacketsParallel(uchar* data, qint64 size, uchar target)
{
    const int threadCount = QThread::idealThreadCount();
    const qint64 chunkSize = size / threadCount;

    QVector<QFuture<qint64>> futures;

    for (int i = 0; i < threadCount; ++i) {
        qint64 start = i * chunkSize;
        qint64 end = (i == threadCount - 1) ? size : (i + 1) * chunkSize;

        futures.append(QtConcurrent::run([=]() {
            qint64 localCount = 0;
            qint64 lastPos = -1;

            // 在当前chunk中查找0x55并检查长度
            for (qint64 j = start; j < end; ++j) {
                if (data[j] == target) {
                    if (lastPos != -1) {
                        qint64 packetLength = j - lastPos;
                        if (packetLength >= 8000 && packetLength <= 8500) {//未转码的时候标准包长度为8237
                            localCount++;
                        }
                    }
                    lastPos = j;
                }
            }

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

void ZrSpectrumPlugin::shutdown(){
    if (mTimerThread){
        mTimerThread->requestInterruption();
        mTimerThread->wait();
        mTimerThread = nullptr;
    }

    mInitialized = false;
    qDebug() << "ZrSpectrumPlugin shutdown";
}

QStringList ZrSpectrumPlugin::supportedMethods() const{
    return {"connect", "disconnect", "readParameters", "writeParameters"}; // 声明支持的方法
}

QVariant ZrSpectrumPlugin::invoke(const QString& method, const QVariantMap& params){
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

QVariant ZrSpectrumPlugin::connectDevice(const QVariantMap& params){
    Q_UNUSED(params); // 示例中无需参数
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device connected";
    return result;
}

QVariant ZrSpectrumPlugin::disconnectDevice(const QVariantMap& params){
    Q_UNUSED(params);
    QVariantMap result;
    result["success"] = true;
    result["message"] = "QGenericFilePlugin device disconnected";
    return result;
}

QVariant ZrSpectrumPlugin::readParameters(const QVariantMap& params){
    Q_UNUSED(params);

    QVariantMap result;
    result["[1]是否循环发送"] = mCycleTransfer;//前面带个序号是为了禁止自动排序，因为QMap是排序的
    result["[2]发送周期/ms"] = mSampleFrequency;
    result["[3]锆活化测试数据路径"] = binaryFilePath;
    result["[4]总能谱个数"] = totalPackets;
    result["[5]文件大小/MB"] = m_fileSize;
    return result;
}

QVariant ZrSpectrumPlugin::writeParameters(const QVariantMap& params){
    mCycleTransfer = params.value("[1]是否循环发送").toBool();
    mSampleFrequency = params.value("[2]发送周期/ms").toUInt();
    binaryFilePath = params.value("[3]锆活化测试数据路径").toString();
    // totalPackets = params.value("[4]总能谱个数").toULongLong();
    // m_fileSize = params.value("[5]文件大小/MB").toUInt();

    qDebug() << "ZrSpectrumPlugin writing parameters:" << params;

    QVariantMap result;
    result["success"] = true;
    result["message"] = "ZrSpectrumPlugin parameters written successfully";
    return result;
}
