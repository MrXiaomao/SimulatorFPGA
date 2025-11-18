<<<<<<< HEAD
#ifndef LITETHREAD_H
#define LITETHREAD_H

=======
/*
 * @Author: MrPan
 * @Date: 2025-11-13 11:35:59
 * @LastEditors: Maoxiaoqing
 * @LastEditTime: 2025-11-18 17:05:39
 * @Description: 请填写简介
 */
#ifndef LITETHREAD_H
#define LITETHREAD_H
 
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
#include <QThread>
typedef std::function<void()> LPThreadWorkProc;
class QLiteThread :public QThread {
    Q_OBJECT
<<<<<<< HEAD

=======
 
>>>>>>> 3491862ae1401aa40408842f803a57ce5ac45010
private:
    LPThreadWorkProc m_pfThreadWorkProc = 0;

public:
    explicit QLiteThread(QObject* parent = Q_NULLPTR, LPThreadWorkProc pfThreadWorkProc = Q_NULLPTR)
        : QThread(parent)
        , m_pfThreadWorkProc(pfThreadWorkProc)
    {
        //qRegisterMetaType<QVariant>("QVariant");
        connect(this, &QThread::finished, this, &QThread::deleteLater);
    }

    //析构函数
    ~QLiteThread()
    {
    }

    void setWorkThreadProc(LPThreadWorkProc pfThreadRun) {
        this->m_pfThreadWorkProc = pfThreadRun;
    }

protected:
    void run() override {
        m_pfThreadWorkProc();
    }

signals:
    //这里信号函数的参数个数可以根据自己需要随意增加
//    void invokeSignal();
//    void invokeSignal(QVariant);

public slots:

};

#endif // LITETHREAD_H
