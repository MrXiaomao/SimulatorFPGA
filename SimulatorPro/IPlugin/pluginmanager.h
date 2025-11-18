#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H
#include "IPlugin.h"
#include <QObject>
#include <QMap>
#include <QPluginLoader>
#include <QDir>
#include <QStringList>

// 单例模式：全局唯一插件管理器（Singleton Pattern）
class PluginManager : public QObject {
    Q_OBJECT

public:
    static PluginManager* instance();

    // ------------------------------ 插件加载/卸载 ------------------------------
    bool loadPlugins(const QString& pluginPath);  // 加载指定目录下的插件
    bool unloadPlugins();  // 卸载所有插件

    // ------------------------------ 插件获取/调用 ------------------------------
    IPlugin* getPlugin(const QString& pluginName) const; // 按名称获取插件实例
    QStringList getPluginNames() const;  // 获取所有插件名称

    // 动态调用插件方法（封装底层接口）
    QVariant invokePlugin(const QString& pluginName,
                          const QString& method,
                          const QVariantMap& params = QVariantMap());

    // ------------------------------ 类型转换 ------------------------------
    // 模板方法：将插件实例转换为指定类型（如具体插件类）
    template<typename T>
    T*getPluginAs(const QString& pluginName) const{
        IPlugin* plugin = getPlugin(pluginName);
        return qobject_cast<T*>(plugin);  // 利用Qt元对象系统进行安全转换
    }

private:
    explicit PluginManager(QObject* parent = nullptr);
    ~PluginManager();

    static PluginManager* s_instance;  // 单例实例指针
    QMap<QString, QPluginLoader*> m_loaders;  // 插件名称 → 加载器的映射（用于卸载）
};

#endif // PLUGINMANAGER_H
