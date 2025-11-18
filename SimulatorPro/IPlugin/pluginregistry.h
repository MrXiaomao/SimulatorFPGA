// PluginRegistry.h
#ifndef PLUGINREGISTRY_H
#define PLUGINREGISTRY_H

#include "IPlugin.h"
#include <QObject>
#include <QMap>
#include <QList>
#include <QMutex>

// 注册表模式：集中管理插件元数据和实例（Registry Pattern）
class PluginRegistry : public QObject {
    Q_OBJECT

public:
    // 单例模式：全局唯一实例（Singleton Pattern）
    static PluginRegistry* instance();

    // ------------------------------ 注册/注销 ------------------------------
    void registerPlugin(IPlugin* plugin); // 注册插件
    void unregisterPlugin(const QString& pluginName); // 注销插件

    // ------------------------------ 查询 ------------------------------
    IPlugin* getPlugin(const QString& pluginName) const; // 按名称查询
    QList<IPlugin*> getAllPlugins() const; // 获取所有插件
    QStringList getPluginNames() const;	// 获取所有插件名称
    QList<IPlugin*> getPluginsByCategory(const QString& category) const;  // 按分类查询

    // ------------------------------ 状态检查 ------------------------------
    bool isPluginRegistered(const QString& pluginName) const;  // 检查是否已注册

private:
    explicit PluginRegistry(QObject* parent = nullptr);
    ~PluginRegistry();

    static PluginRegistry* s_instance;  // 单例实例指针
    QMap<QString, IPlugin*> m_plugins;  // 插件名称 → 实例的映射
    QMap<QString, QStringList> m_categoryMap;  // 分类 → 插件名称列表的映射
    mutable QMutex m_mutex;  // 互斥锁保证线程安全
};

#endif // PLUGINREGISTRY_H
