#ifndef PLUGIN_ZR_H
#define PLUGIN_ZR_H

#include <QObject>
#include "iplugin.h"
#include "qlitethread.h"

class Plugin_Zr : public IPlugin{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "zr.IPlugin")
    Q_INTERFACES(IPlugin)

public:
    Plugin_Zr();
    ~Plugin_Zr();

    // IPlugin 接口实现
    QString name() const override{
        return "锆活化工程-正弦波形";
    }

    // 插件名称
    // 版本号
    QString version() const override {
        return "1.0.0";
    }

    // 描述
    QString description() const override{
        return "生成正弦波波形数据并发送";
    }

    // 分类
    QString category() const override{
        return "锆活化工程";
    }

    bool initialize() override;
    void shutdown() override;
    QStringList supportedMethods() const override;
    QVariant invoke(const QString& method, const QVariantMap& params) override;

private:
    // 具体功能方法 ------------------------------
    QVariant connectDevice(const QVariantMap& params);
    QVariant disconnectDevice(const QVariantMap& params);
    QVariant readParameters(const QVariantMap& params);
    QVariant writeParameters(const QVariantMap& params);

    bool mInitialized = false;
    quint32 mSampleFrequency = 1000;//发送周期（毫秒）
    QLiteThread* mTimerThread = nullptr;
};

#endif // PLUGIN_ZR_H
