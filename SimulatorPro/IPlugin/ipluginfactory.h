#ifndef IPLUGINFACTORY_H
#define IPLUGINFACTORY_H
#include "IPlugin.h"
#include <QObject>

// 工厂方法模式：定义插件创建接口（Factory Method Pattern）
class IPluginFactory : public QObject {
Q_OBJECT

public:
    virtual ~IPluginFactory() = default;

    // 创建插件实例（具体工厂实现）
    virtual IPlugin* createPlugin(const QString& pluginName) = 0; // 返回插件
};

// 声明接口唯一标识符
#define IPluginFactory_iid "com.snowWolf.IPluginFactory"
Q_DECLARE_INTERFACE(IPluginFactory, IPluginFactory_iid)

#endif // IPLUGINFACTORY_H
