#ifndef TRANSFER_INSTANCE_H
#define TRANSFER_INSTANCE_H

#include <QObject>
#include <QQueue>

#include <QtPlugin>
#include "libTrans.h"

class TransferInstance : public TransferInterface
{
	Q_OBJECT
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	Q_PLUGIN_METADATA(IID "com.focuschina.TransInterface" FILE "libTrans.json")
#endif
		
		Q_INTERFACES(TransferInterface)

public:
	TransferInstance();
	~TransferInstance();

public:
	virtual void init(QString turnserver, QString logFile);

    virtual QString getTurnServer();

	virtual void updateCurrentUser(QString currentUser);


	// ����һ��Session�� ����sessionID
	// Session�ɷ��ͷ����ɣ����ܷ���Ҫ���뷢�ͷ����ɵ�SessionId
	virtual QString initSession(bool isControl, QString sessionId = "");

    /**
	* @brief  ��ȡ���õĴ���ͨ��
	* @param sessionId  Session��ΨһID
	* @return Channel* ����ͨ��
	*/
    virtual Channel* getChannel(QString sessionId);


	virtual QString getCurrentUser() { return m_currentUser; }

private:

    QString         m_turnServer;
	QString			m_currentUser;
};

#endif // TRANSFER_INSTANCE_H
