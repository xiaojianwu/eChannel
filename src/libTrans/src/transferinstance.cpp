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

	// Session管理器
	SessionMgr::instance()->init(this);

    UTPManager::instance()->init();

    FocusIce::PJICESocket::setIceLogfile(logFile);

    FocusIce::PJICESocket::setIceTurnAuth(uName, passwd, realm);
}


QString TransferInstance::getTurnServer()
{
    return m_turnServer;
}

// 创建一个Session， 返回sessionID
// Session由邀请方方生成，接受方需要传入发送方生成的SessionId
QString TransferInstance::initSession(QString sessionId)
{
	QString uuid = sessionId;

    // 用来控制由哪端发起utp connect
    bool isControl = false;

	if (sessionId.isEmpty())
	{
		uuid = QUuid::createUuid().toString();
		uuid = uuid.remove("{");
		uuid = uuid.remove("}");
	}
    else
    {
        // 接收方作为控制端，由于接收方此时的session还未初始化
        isControl = true;
    }


	Session *session = SessionMgr::instance()->createSession(uuid, isControl);

	return uuid;	
}


// 获取可用的传输通道

Channel* TransferInstance::getChannel(QString sessionId)
{
    Session* session = SessionMgr::instance()->getSession(sessionId);
    return session->getChannel();
}

#if QT_VERSION < 0x050000
 Q_EXPORT_PLUGIN2(p2pinstance, TransferInstance);
#endif