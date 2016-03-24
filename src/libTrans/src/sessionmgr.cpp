#include "sessionmgr.h"

#include <QUuid>
#include <QDebug>

#include <QTimer>
#include <QStringList>

#include <Logger.h>

#include <assert.h>

// 点对点或 中转 通道管理
SessionMgr::SessionMgr()
{

}

SessionMgr::~SessionMgr()
{

}


SessionMgr *SessionMgr::inst = 0;
SessionMgr *SessionMgr::instance()
{
	if (!inst)
	{
		inst = new SessionMgr;
	}
	return inst;
}


void SessionMgr::init(TransferInterface* inteface)
{
	m_interface = inteface;
}

// 创建会话
Session* SessionMgr::createSession(QString uuid, bool isControl)
{
	Session* session = NULL;
	if (m_hashSession.contains(uuid))
	{
		session = m_hashSession[uuid];
		if (session != NULL)
		{
			return session;
		}
	}
	else
	{
		session = new Session(this);
		session->init(uuid, m_interface, isControl);

        connect(session, SIGNAL(sigClose()), this, SLOT(onClose()));

		m_hashSession[uuid] = session;
	}
	return session;
}

void SessionMgr::onClose()
{
    Session* sess = qobject_cast<Session*>(sender());

    QString sid = m_hashSession.key(sess);
    m_hashSession.remove(sid);

    delete sess;
}

// 获得Session实例
Session* SessionMgr::getSession(QString uuid)
{
	Session* session = NULL;
	if (m_hashSession.contains(uuid))
	{
		session = m_hashSession[uuid];
	}
    else
    {
        Q_ASSERT_X(false, "getSession", "session not found.");
        //session = createSession(uuid, false);
    }
	return session;
}