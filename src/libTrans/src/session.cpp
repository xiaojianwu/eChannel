#include "session.h"


#include <QtGlobal>
#include <Logger.h>

#include <QStringList>


#include "sessionmgr.h"

Session::Session(QObject *parent):QObject(parent)
{
    m_channel = NULL;
    m_transInterface = NULL;

}

Session::~Session()
{

}


void Session::init(QString sessionId, TransferInterface* tInterface, bool isControl)
{
    m_transInterface = tInterface;
    m_sessionId = sessionId;

    m_isControl = isControl;

    m_channel = new TransportChannel(this);
    connect(m_channel, SIGNAL(sigClose()), this, SLOT(onClose()));

    m_threadChannel.setObjectName("TransportChannelThread");

    //m_channel->moveToThread(&m_threadChannel);

    //m_threadChannel.start();

    m_channel->init(m_sessionId, isControl, tInterface);
}


TransportChannel* Session::getChannel()
{
	return m_channel;
}

void Session::onClose()
{
    delete m_channel;
    m_channel = NULL;
    emit sigClose();
}