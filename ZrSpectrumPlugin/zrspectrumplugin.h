#ifndef ZRSPECTRUMPLUGIN_H
#define ZRSPECTRUMPLUGIN_H

#include <QObject>
#include "iplugin.h"
#include "qlitethread.h"

#include <QtEndian> //qFromBigEndian需要
#include <cstring>  // 用于内存初始化（如memset）

class ZrSpectrumPlugin : public IPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "ZrSinWaveform.IPlugin")
    Q_INTERFACES(IPlugin)

public:
    // 数据包结构定义,网口传来的原始数据结构
    #pragma pack(push, 1) // 确保1字节对齐
    struct TCPPacket {
        quint32 specID;     // 能谱序号
        quint32 specAccumTime;   //能谱测量对应累积时长，单位ms
        quint32 deathTime;  //能谱测量死时间，单位10ns
        quint32 spectrum[2045];  // 能谱，起点x坐标为3，需要注意的是，能谱本来2048，但是前三道被时间信息占用，因此2045指后面部分。

        // 构造函数初始化
        TCPPacket()
            : specID(0),
            specAccumTime(0),
            deathTime(0) {
            // 初始化能谱数组
            std::memset(spectrum, 0, sizeof(spectrum));
        }

        // 添加字节序转换成员函数
        // 字节序问题：x86 是小端序，网络数据通常是大端序
        void convertNetworkToHost() {
            specID = qFromBigEndian(specID);
            specAccumTime = qFromBigEndian(specAccumTime);
            deathTime = qFromBigEndian(deathTime);

            // 转换能谱数据数组
            for (int i = 0; i < 2045; ++i) {
                spectrum[i] = qFromBigEndian(spectrum[i]);
            }
        }
    };
    #pragma pack(pop) // 恢复默认对齐

    ZrSpectrumPlugin();
    ~ZrSpectrumPlugin();

    // IPlugin 接口实现
    QString name() const override{
        return "锆活化工程-活化能谱";
    }

    // 插件名称
    // 版本号
    QString version() const override {
        return "1.0.0";
    }

    // 描述
    QString description() const override{
        return "生成活化能谱并发送";
    }

    // 分类
    QString category() const override{
        return "锆活化工程";
    }

    IPlugin* clone() override;
    bool initialize() override;
    void shutdown() override;
    QStringList supportedMethods() const override;
    QVariant invoke(const QString& method, const QVariantMap& params = QVariantMap()) override;

private:
    QVector<QByteArray> OldData2NewData(const QByteArray& data);
    // 转码
    QByteArray encode(const QByteArray &data, const QByteArray &from1, const QByteArray &to1, const QByteArray &from2, const QByteArray &to2);

    //从旧数据中提取数据包
    bool getDataFromQByte(const QByteArray &byteArray, TCPPacket &DataPacket);

    // 更新文件信息显示
    void updateFileInfo();

    //采用常规打开文件方式共计0x55字节的个数
    qint64 countPacketsInFileTraditional(const QString& filePath);

    // 快速统计0x55的个数（数据包总数）,使用内存映射快速统计
    qint64 countPacketsInFile(const QString& filePath);

    // 并行统计0x55个数
    qint64 countPacketsParallel(uchar* data, qint64 size, uchar target);

    // 具体功能方法 ------------------------------
    QVariant connectDevice(const QVariantMap& params);
    QVariant disconnectDevice(const QVariantMap& params);
    QVariant readParameters(const QVariantMap& params);
    QVariant writeParameters(const QVariantMap& params);

    // 发送方转码
    const QByteArray m_senderFrom = QByteArray::fromHex("55");                      // 发送方转码前
    const QByteArray m_senderFrom2 = QByteArray::fromHex("FF");                     // 发送方转码前
    const QByteArray m_senderTo1 = QByteArray::fromHex("FF 00");                    // 发送方转码后
    const QByteArray m_senderTo2 = QByteArray::fromHex("FF FF");                    // 发送方转码后

    // 接收方转码
    const QByteArray m_receiverFrom1 = m_senderTo1;                                 // 接收方转码前
    const QByteArray m_receiverFrom2 = m_senderTo2;                                 // 接收方转码前
    const QByteArray m_receiverTo1 = m_senderFrom;                                  // 接收方转码后
    const QByteArray m_receiverTo2 = m_senderFrom2;                                 // 接收方转码后

    bool mInitialized = false;
    bool mCycleTransfer = true; //是否循环发送
    QString binaryFilePath;     // 二进制文件路径,锆活化实验数据
    qint32 m_fileSize = 0;        //文件大小，单位MB
    qint64 totalPackets = 0;    //文件种总数据包个数，不包含指令
    qint64 sendPackets = 0;     //已发送数据包个数

    quint32 mSampleFrequency = 1000;//发送周期（毫秒）
    QLiteThread* mTimerThread = nullptr;
};

#endif // ZRSPECTRUMPLUGIN_H
