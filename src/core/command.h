#ifndef COMMAND_H
#define COMMAND_H

#include "types.h"
#include "message.h"

class CoreNetwork;

class Command : public QObject
{
    Q_OBJECT

public:
    Command(UserId userId, CoreNetwork *network, QObject *parent = 0);
    virtual ~Command();
    virtual void exec() = 0;
    virtual void setupOutput();
    virtual void sendMessage(Message::Type msgType, BufferInfo::Type bufferType, const QString &target, const QString &text);

signals:
    void msg(Message::Type, BufferInfo::Type ,const QString &target, const QString &msg);

protected:
    virtual UserId userId();

private:
    UserId _userId;
    CoreNetwork *_network;
};

#endif // COMMAND_H
