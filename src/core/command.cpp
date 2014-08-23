#include "command.h"
#include "corenetwork.h"

Command::Command(UserId userId, CoreNetwork *network, QObject *parent)
    : QObject(parent),
      _userId(userId),
      _network(network)
{
}

Command::~Command()
{
}

void Command::setupOutput()
{
    connect(this, SIGNAL(msg(Message::Type,BufferInfo::Type,QString,QString)),
            _network, SLOT(displayMsg(Message::Type,BufferInfo::Type,QString,QString)), Qt::QueuedConnection);
}

void Command::sendMessage(Message::Type msgType, BufferInfo::Type bufferType, const QString &target, const QString &text)
{
    emit msg(msgType, bufferType, target, text);
}

UserId Command::userId()
{
    return _userId;
}
