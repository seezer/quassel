#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QObject>

class Command;
class Task;

class Scheduler : public QObject
{
    Q_OBJECT
public:
    explicit Scheduler(int maxThreads, QObject *parent = 0);
    ~Scheduler();

    Task *createTask(Command *command);
    /**
     * @brief Schedule a task for immediate execution on next free worker
     * @param task
     */
    void runTask(Task *task);

    // TODO provide runAt
signals:
    void start();

public slots:
private:
};

#endif // SCHEDULER_H
