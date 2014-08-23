#include "scheduler.h"

#include "task.h"
#include "command.h"

#include <QThreadPool>

Scheduler::Scheduler(int maxThreads, QObject *parent) :
    QObject(parent)
{
    // even if maxThreads is < 1, one thread is created
    QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
}

Scheduler::~Scheduler()
{
    QThreadPool::globalInstance()->waitForDone();
}


Task *Scheduler::createTask(Command *command)
{
    return new Task(command);
}

void Scheduler::runTask(Task *task)
{
    // TODO use an own pool if core ever has other usecases for the global one
    QThreadPool *pool = QThreadPool::globalInstance();
    pool->start(task);
}
