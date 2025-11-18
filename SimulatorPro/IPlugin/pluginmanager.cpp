#include <QApplication>
#include <QDebug>
#include <QPluginLoader>
#include "pluginmanager.h"
#include "pluginregistry.h"

PluginManager* PluginManager::s_instance = nullptr;
PluginManager* PluginManager::instance()
{
    if (PluginManager::s_instance == nullptr)
        PluginManager::s_instance = new PluginManager();
    return PluginManager::s_instance;
}

#include <QDir>
#include <QStringList>
PluginManager::PluginManager(QObject* parent)
    : QObject(parent)
{
    //加载插件
    {
        QString pluginPath = QApplication::applicationDirPath() + "/QtPlugins/Simulator";
        QDir dir(pluginPath);
        if (dir.exists()) {
            dir.setFilter(QDir::Files);
            dir.setNameFilters(QStringList() << "*.dll");
            QStringList dllFiles = dir.entryList();
            foreach (const QString &filePath, dllFiles) {
                if (loadPlugins(pluginPath + QDir::separator() + filePath)) {

                }
            }

            QStringList pluginNames = getPluginNames();
            qDebug() << "Loaded plugins:" << pluginNames;
        }
    }
}

PluginManager::~PluginManager(){
    unloadPlugins();
}

bool PluginManager::loadPlugins(const QString& pluginPath)
{
    PluginRegistry* pluginRegistry = PluginRegistry::instance();
    QPluginLoader *pluginLoader = new QPluginLoader(pluginPath);
    if (pluginLoader->load()){
        IPlugin* plugin = qobject_cast<IPlugin*>(pluginLoader->instance());
        //plugin->initialize();
        pluginRegistry->registerPlugin(plugin);

        m_loaders[plugin->name()] = pluginLoader;
        return true;
    }

    pluginLoader->deleteLater();
    return false;
}

bool PluginManager::unloadPlugins()
{
    PluginRegistry* pluginRegistry = PluginRegistry::instance();
    for (auto iter = m_loaders.begin(); iter != m_loaders.end(); ++iter){
        IPlugin* plugin = qobject_cast<IPlugin*>(iter.value()->instance());
        plugin->shutdown();
        pluginRegistry->unregisterPlugin(iter.key());
    }
    m_loaders.clear();
    return true;
}

IPlugin* PluginManager::getPlugin(const QString& pluginName) const
{
    PluginRegistry* pluginRegistry = PluginRegistry::instance();
<<<<<<< HEAD
    return pluginRegistry->getPlugin(pluginName)->clone();
=======
    return pluginRegistry->getPlugin(pluginName);
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
}

QStringList PluginManager::getPluginNames() const
{
    PluginRegistry* pluginRegistry = PluginRegistry::instance();
    return pluginRegistry->getPluginNames();
}

// 动态调用插件方法（封装底层接口）
QVariant PluginManager::invokePlugin(const QString& pluginName,
                      const QString& method,
                      const QVariantMap& params)
{
    IPlugin *plugin = getPlugin(pluginName);
    return plugin->invoke(method, params);
}
