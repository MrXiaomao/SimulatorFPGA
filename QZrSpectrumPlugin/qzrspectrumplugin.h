#ifndef QZRSPECTRUMPLUGIN_H
#define QZRSPECTRUMPLUGIN_H

#include <QObject>
#include "iplugin.h"
#include "qlitethread.h"

#include <QtEndian> //qFromBigEndian需要
#include <cstring>  // 用于内存初始化（如memset）

class QZrSpectrumPlugin : public IPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "QZrSinWaveform.IPlugin")
    Q_INTERFACES(IPlugin)

public:
    QZrSpectrumPlugin();
    ~QZrSpectrumPlugin();

    // IPlugin 接口实现
    QString name() const override{
        return "锆活化工程-活化能谱(新)";
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
    // 预处理
    QVariant preProcess();

    // 更新文件信息显示
    void updateFileInfo();

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

#endif // QZRSPECTRUMPLUGIN_H
