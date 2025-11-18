#ifndef IPLUGIN_H
#define IPLUGIN_H
#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>

// 抽象产品接口 - 所有插件的基类（Abstract Factory Pattern）
class IPlugin : public QObject {
    Q_OBJECT // 启用Qt元对象系统（信号槽、反射）

public:
    virtual ~IPlugin() = default; // 虚析构函数保证多态销毁

    // ------------------------------ 元数据接口 ------------------------------
    // 插件名称（唯一标识，如 "锆活化工程插件"）
    virtual QString name() const = 0;

    // 版本号（如 "1.0.0"）
    virtual QString version() const = 0;

    // 功能描述（如 "正弦波数据插件"）
    virtual QString description() const = 0;

    // 分类（如 "锆活化"，用于分组管理）
    virtual QString category() const = 0;

    // ------------------------------ 生命周期接口 ------------------------------
    // 原计划是通过插件工厂通过调用createPlugin来生产多个插件，这里简化一下，直接自己生自己吧 (^-^)
    virtual IPlugin* clone() = 0;
    // 初始化插件（加载资源、连接信号等）
    virtual bool initialize() = 0;
    // 反初始化插件（释放资源、断开连接等）
    virtual void shutdown() = 0;

    // ------------------------------ 功能调用接口 ------------------------------
    // 动态调用插件方法（方法名+参数）
    virtual QVariant invoke(const QString& method, const QVariantMap& params = QVariantMap()) = 0;
    // 声明支持的方法列表（防止非法调用）
    virtual QStringList supportedMethods() const = 0;

signals:
    // 观察者模式：插件主动通知事件（如数据更新、状态变更）
    void notifyEvent(const QString& event, const QVariantMap& data);
};

// 声明接口唯一标识符（用于Qt元对象系统反射）
QT_BEGIN_NAMESPACE
#define IPlugin_iid "com.snowWolf.IPlugin"
Q_DECLARE_INTERFACE(IPlugin, IPlugin_iid) // 注册接口到Qt元对象系统
QT_END_NAMESPACE

#endif // IPLUGIN_H
