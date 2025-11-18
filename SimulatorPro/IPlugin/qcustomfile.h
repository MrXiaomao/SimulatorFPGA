#ifndef QCUSTOMFILE_H
#define QCUSTOMFILE_H

#include <QObject>
#include "iplugin.h"
#include "qlitethread.h"

class QCustomFilePlugin : public IPlugin{
    Q_OBJECT
    //Q_PLUGIN_METADATA(IID "customfile.IPlugin")
    //Q_INTERFACES(IPlugin)

public:
    QCustomFilePlugin();
    ~QCustomFilePlugin();

    // IPlugin 接口实现
    QString name() const override{
        return "自定义文件数据包";
    }

    // 插件名称
    // 版本号
    QString version() const override {
        return "1.0.0";
    }

    // 描述
    QString description() const override{
        return "自定义数据头、数据尾，数据内容从文件中读取";
    }

    // 分类
    QString category() const override{
        return "通用分类";
    }

    IPlugin* clone() override;
    bool initialize() override;
    void shutdown() override;
    QStringList supportedMethods() const override;
    QVariant invoke(const QString& method, const QVariantMap& params = QVariantMap()) override;

private:
    QVariant connectDevice(const QVariantMap& params);
    QVariant disconnectDevice(const QVariantMap& params);
    QVariant readParameters(const QVariantMap& params);
    QVariant writeParameters(const QVariantMap& params);

    bool mInitialized = false; // 初始化状态标志
    QString mFileName;// 文件名
    QString mPacketHead = "FF FF AA 1B 00 1D";//包头
    QString mPacketTail = "00 00 00 00 FF FF CC 1D";//包尾
    quint8 mDataBits = 16;//数据字节大小（单字节、双字节、4字节、8字节等待）
    bool mCycleTransfer = true;//是否循环发送
    quint32 mSampleFrequency = 1000;//发送周期（毫秒）
    QLiteThread* mTimerThread = nullptr;// 定时器（模拟数据更新）
};

#endif // QCUSTOMFILE_H
