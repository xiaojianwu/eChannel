#include "transferinstance.h"

#include <QUuid>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QDir>

#include <Logger.h>

#include "sessionmgr.h"


#include "utpmanager.h"

TransferInstance::TransferInstance()
{
}


TransferInstance::~TransferInstance()
{
}

void TransferInstance::init(QString turnserver, QString logFile)
{
	//qRegisterMetaType<TransportState>("TransportState");
    m_turnServer = turnserver;

	// Session������
	SessionMgr::instance()->init(this);

    UTPManager::instance()->init();

    FocusIce::PJICESocket::setIceLogfile(logFile);
}


QString TransferInstance::getTurnServer()
{
    return m_turnServer;
}

void TransferInstance::updateCurrentUser(QString currentUser)
{
	m_currentUser = currentUser;
}


// ����һ��Session�� ����sessionID
// Session�ɷ��ͷ����ɣ����ܷ���Ҫ���뷢�ͷ����ɵ�SessionId
QString TransferInstance::initSession(bool isControl, QString sessionId)
{
	QString uuid = sessionId;
	if (sessionId.isEmpty())
	{
		uuid = QUuid::createUuid().toString();
		uuid = uuid.remove("{");
		uuid = uuid.remove("}");
	}


	Session *session = SessionMgr::instance()->createSession(uuid, isControl);

	return uuid;	
}


// ��ȡ���õĴ���ͨ��

Channel* TransferInstance::getChannel(QString sessionId)
{
    Session* session = SessionMgr::instance()->getSession(sessionId);
    return session->getChannel();
}

#if QT_VERSION < 0x050000
 Q_EXPORT_PLUGIN2(p2pinstance, TransferInstance);
#endif