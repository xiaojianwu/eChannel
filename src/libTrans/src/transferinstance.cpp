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

void TransferInstance::init(QString turnserver, QString logFile, const QString &uName/* = "test"*/,const QString &passwd/* = "test"*/,const QString &realm/* = "www.vemic.com"*/)
{
    m_turnServer = turnserver;

	// Session������
	SessionMgr::instance()->init(this);

    UTPManager::instance()->init();

    FocusIce::PJICESocket::setIceLogfile(logFile);

    FocusIce::PJICESocket::setIceTurnAuth(uName, passwd, realm);
}


QString TransferInstance::getTurnServer()
{
    return m_turnServer;
}

// ����һ��Session�� ����sessionID
// Session�����뷽�����ɣ����ܷ���Ҫ���뷢�ͷ����ɵ�SessionId
QString TransferInstance::initSession(QString sessionId)
{
	QString uuid = sessionId;

    // �����������Ķ˷���utp connect
    bool isControl = false;

	if (sessionId.isEmpty())
	{
		uuid = QUuid::createUuid().toString();
		uuid = uuid.remove("{");
		uuid = uuid.remove("}");
	}
    else
    {
        // ���շ���Ϊ���ƶˣ����ڽ��շ���ʱ��session��δ��ʼ��
        isControl = true;
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