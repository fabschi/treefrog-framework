#ifndef TSCHEDULER_H
#define TSCHEDULER_H

#include <QTimer>
#include <QThread>
#include <TGlobal>
#include <TDatabaseContext>


class T_CORE_EXPORT TScheduler : public QThread, public TDatabaseContext
{
    Q_OBJECT
public:
    TScheduler(QObject *parent = 0);
    virtual ~TScheduler();

    void start(int msec);
    void restart();
    void stop();
    int	interval() const;
    bool isSingleShot() const;
    void setSingleShot(bool singleShot);

protected:
    virtual void job() = 0;
    void rollbackTransaction();
    void publish(const QString &topic, const QString &text);
    void publish(const QString &topic, const QByteArray &binary);

private slots:
    void start(Priority priority = InheritPriority);

signals:
    void startTimer(int msec);
    void startTimer();
    void stopTimer();

private:
    void run();

    QTimer *timer {nullptr};
    bool rollback {false};

    T_DISABLE_COPY(TScheduler)
    T_DISABLE_MOVE(TScheduler)
};

#endif // TSCHEDULER_H
