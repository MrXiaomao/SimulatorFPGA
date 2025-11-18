#ifndef QGENERICFILE_H
#define QGENERICFILE_H

#include <QObject>
#include "iplugin.h"
#include "qlitethread.h"

class QGenericFilePlugin : public IPlugin{
    Q_OBJECT
    //Q_PLUGIN_METADATA(IID "file.IPlugin")
    //Q_INTERFACES(IPlugin)

public:
    QGenericFilePlugin();
    ~QGenericFilePlugin();

    // IPlugin 接口实现
    QString name() const override{
        return "本地文件";
    }

    // 插件名称
    // 版本号
    QString version() const override {
        return "1.0.0";
    }

    // 描述
    QString description() const override{
        return "从文件中读取完整段波形数据";
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
    quint32 mBlockSize = 4*1024*1024;//每次读取文件块大小（字节）
    bool mCycleTransfer = true;//是否循环发送
    quint32 mSampleFrequency = 1000;//发送周期（毫秒）
    QLiteThread* mTimerThread = nullptr;// 定时器（模拟数据更新）
};

#endif // QGENERICFILE_H
