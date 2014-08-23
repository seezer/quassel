#ifndef TASK_H
#define TASK_H

#include "command.h"

#include <QDateTime>
#include <QScopedPointer>
#include <QObject>
#include <QRunnable>

class Task : public QObject, public QRunnable
{
    Q_OBJECT
public:
    Task(Command *command, QObject *parent = 0);

    QDateTime runAt() const;
    void setRunAt(const QDateTime &runAt);

    virtual void run();

private:
    QDateTime _runAt;

    QScopedPointer<Command> _command;
};

#endif // TASK_H
