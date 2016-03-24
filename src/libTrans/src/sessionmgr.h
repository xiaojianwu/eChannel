#ifndef SESSIONMGR_H
#define SESSIONMGR_H

#include <QObject>
#include <QHash>

#include <string>
#include <vector>

#include <QQueue>
#include <QMutex>
#include <QList>

#include "session.h"

#include "libTrans.h"

class SessionMgr : public QObject
{
	Q_OBJECT
public:
	static SessionMgr* instance();


	void init(TransferInterface* inteface);

private:
	static SessionMgr* inst;

protected:
	SessionMgr();
	~SessionMgr();


public:
	// 创建会话
	Session* createSession(QString uuid, bool isControl);


	// 获得Session实例
	Session* getSession(QString uuid);

private slots:
    void onClose();

private:
	// key: sessionid
	QHash<QString, Session*> m_hashSession;

	TransferInterface*				m_interface;
};

#endif // SESSIONMGR_H
