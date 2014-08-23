#include "task.h"
#include "command.h"
Task::Task(Command *command, QObject *parent)
    : QObject(parent),
      _command(command)
{
}

QDateTime Task::runAt() const
{
    return _runAt;
}

void Task::setRunAt(const QDateTime &runAt)
{
    _runAt = runAt;
}

void Task::run()
{
    _command->setupOutput();
    _command->exec();
}

