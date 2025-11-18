#include "pluginregistry.h"
#include "qgenericfile.h"
#include "qcustomfile.h"
//#include "qzrsinwaveformplugin.h"
PluginRegistry::PluginRegistry(QObject *parent)
    : QObject{parent}
{
    //本地文件
    {
        QGenericFilePlugin *plugin = new QGenericFilePlugin();
        registerPlugin(plugin);
    }

    //自定义文件
    {
        QCustomFilePlugin *plugin = new QCustomFilePlugin();
        registerPlugin(plugin);
    }

    // {
    //     QZrSinWaveformPlugin *plugin = new QZrSinWaveformPlugin();
    //     registerPlugin(plugin);
    // }
}

PluginRegistry::~PluginRegistry()
{
    //暂时改为手动注册
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter){
        IPlugin* plugin = qobject_cast<IPlugin*>(iter.value());
        plugin->deleteLater();
    }
}

PluginRegistry* PluginRegistry::s_instance = nullptr;
PluginRegistry* PluginRegistry::instance()
{
    if (PluginRegistry::s_instance == nullptr)
        PluginRegistry::s_instance = new PluginRegistry();
    return PluginRegistry::s_instance;
}

// ------------------------------ 注册/注销 ------------------------------
void PluginRegistry::registerPlugin(IPlugin* plugin)
{
    QMutexLocker locker(&m_mutex);
    m_plugins[plugin->name()] = plugin;

    m_categoryMap[plugin->category()].append(plugin->name());
}

void PluginRegistry::unregisterPlugin(const QString& pluginName)
{
    QMutexLocker locker(&m_mutex);
    m_plugins.remove(pluginName);

    for (auto iter = m_categoryMap.begin(); iter != m_categoryMap.end(); ++iter){
        if (iter.value().contains(pluginName)){
            iter.value().removeOne(pluginName);
            return;
        }
    }
}

// ------------------------------ 查询 ------------------------------
IPlugin* PluginRegistry::getPlugin(const QString& pluginName) const
{
    QMutexLocker locker(&m_mutex);
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter){
        if (iter.key() == pluginName){
            IPlugin* plugin = /*qobject_cast<IPlugin*>*/(iter.value());
            return plugin;
        }
    }

    return nullptr;
}

QList<IPlugin*> PluginRegistry::getAllPlugins() const
{
    QMutexLocker locker(&m_mutex);
    QList<IPlugin*> result;
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter){
        result << iter.value();
    }
    return result;
}

QStringList PluginRegistry::getPluginNames() const
{
    QMutexLocker locker(&m_mutex);
    QStringList result;
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter){
        result << iter.key();
    }
    return result;
}

QList<IPlugin*> PluginRegistry::getPluginsByCategory(const QString& category) const
{
    QList<IPlugin*> result;
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter){
        if (iter.value()->category() == category)
            result << iter.value();
    }
    return result;
}

// ------------------------------ 状态检查 ------------------------------
bool PluginRegistry::isPluginRegistered(const QString& pluginName) const
{
    for (auto iter = m_plugins.begin(); iter != m_plugins.end(); ++iter){
        if (iter.key() == pluginName){
            return true;
        }
    }

    return false;
}
